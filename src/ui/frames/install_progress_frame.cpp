
/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/frames/install_progress_frame.h"
#include "ui/interfaces/frameinterfaceprivate.h"

#include "base/file_util.h"
#include "base/thread_util.h"
#include "service/hooks_manager.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/frames/consts.h"
#include "ui/frames/inner/install_progress_slide_frame.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/rounded_progress_bar.h"
#include "ui/widgets/tooltip_pin.h"
#include "ui/widgets/title_label.h"

#include <math.h>
#include <QDebug>
#include <QEvent>
#include <QPropertyAnimation>
#include <QStyle>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

namespace installer {

namespace {

const int kProgressBarWidth = 500;
const int kProgressBarHeight = 20;

const int kTooltipWidth = 60;
const int kTooltipHeight = 31;
const int kTooltipLabelMargin = 2;
const int kTooltipFrameWidth = kProgressBarWidth + kTooltipWidth;

const int kRetainingInterval = 3000;
const int kSimulationTimerInterval = 3000;
const int kProgressAnimationDuration = 500;

}  // namespace

class InstallProgressFramePrivate : public FrameInterfacePrivate{
    Q_OBJECT

public:
    explicit InstallProgressFramePrivate(FrameInterface* parent);

    ~InstallProgressFramePrivate();

    void initConnections();
    void initUI();

    // Update value of progress bar to |progress| and update tooltip position.
    void updateProgressBar(int progress);

    // Handles error state
    void onHooksErrorOccurred();
    // Handles successful installation.
    void onHooksFinished();
    void onProgressUpdate(int progress);
    void onRetainingTimerTimeout();
    void onSimulationTimerTimeout();

    Q_DECLARE_PUBLIC(InstallProgressFrame)
    InstallProgressFrame* q_ptr = nullptr;

    bool failed_;

    HooksManager* hooks_manager_ = nullptr;
    QThread* hooks_manager_thread_ = nullptr;

    TitleLabel* title_label_ = nullptr;
    CommentLabel* comment_label_ = nullptr;
    InstallProgressSlideFrame* slide_frame_ = nullptr;
    QLabel* tooltip_label_ = nullptr;
    QProgressBar* progress_bar_ = nullptr;

    QPropertyAnimation* progress_animation_ = nullptr;

    QTimer* simulation_timer_ = nullptr;
};

InstallProgressFrame::InstallProgressFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , progress_(0)
    , d_private(new InstallProgressFramePrivate(this))
{
    setObjectName("install_progress_frame");
}

InstallProgressFrame::~InstallProgressFrame()
{}

void InstallProgressFrame::setProgress(int progress) {
    progress_ = progress;

    Q_D(InstallProgressFrame);
    d->updateProgressBar(progress);
}

bool InstallProgressFrame::failed() const
{
    Q_D(const InstallProgressFrame);

    return d->failed_;
}

void InstallProgressFrame::startSlide() {
    const bool disable_slide =
            GetSettingsBool(kInstallProgressPageDisableSlide);
    const bool disable_animation =
            GetSettingsBool(kInstallProgressPageDisableSlideAnimation);
    const int duration = GetSettingsInt(kInstallProgressPageAnimationDuration);

    Q_D(InstallProgressFrame);
    d->slide_frame_->startSlide(disable_slide, disable_animation, duration);
}

void InstallProgressFrame::init()
{
    startSlide();
}

void InstallProgressFrame::finished()
{

}

bool InstallProgressFrame::shouldDisplay() const
{
    return true;
}

QString InstallProgressFrame::returnFrameName() const
{
    return "Installing";
}

bool InstallProgressFrame::allowPrevious() const
{
    return false;
}

void InstallProgressFrame::simulate() {
    Q_D(InstallProgressFrame);

    if (!d->simulation_timer_->isActive()) {
        startSlide();

        // Reset progress value.
        d->onProgressUpdate(d->progress_bar_->minimum());
        d->simulation_timer_->start();
    }
}

void InstallProgressFrame::runHooks(bool ok) {
    qDebug() << "runHooks()" << ok;

    Q_D(InstallProgressFrame);

    if (ok) {
        // Partition operations take 5% progress.
        d->onProgressUpdate(kBeforeChrootStartVal);

        qDebug() << "emit runHooks() signal";
        // Notify HooksManager to run hooks/ in background thread.
        // Do not run hooks in debug mode.
#ifdef NDEBUG
        emit d->hooks_manager_->runHooks();
#endif
    } else {
        d->onHooksErrorOccurred();
    }
}

void InstallProgressFrame::changeEvent(QEvent* event) {
    Q_D(InstallProgressFrame);

    if (event->type() == QEvent::LanguageChange) {
        d->title_label_->setText(tr("Installing"));
        d->comment_label_->setText(
                    tr("Here are a few cool things to look out for..."));
    } else {
        FrameInterface::changeEvent(event);
    }
}

InstallProgressFramePrivate::InstallProgressFramePrivate(FrameInterface* parent)
    : FrameInterfacePrivate(parent)
    , q_ptr(qobject_cast<InstallProgressFrame* >(parent))
    , failed_(true)
    , hooks_manager_(new HooksManager())
    , hooks_manager_thread_(new QThread(this))
    , simulation_timer_(new QTimer(this))
{
    hooks_manager_->moveToThread(hooks_manager_thread_);

    initUI();
    initConnections();

    hooks_manager_thread_->start();

    simulation_timer_->setSingleShot(false);
    simulation_timer_->setInterval(kSimulationTimerInterval);
}

InstallProgressFramePrivate::~InstallProgressFramePrivate()
{
    QuitThread(hooks_manager_thread_);
}

void InstallProgressFramePrivate::initConnections() {
    connect(hooks_manager_, &HooksManager::errorOccurred,
            this, &InstallProgressFramePrivate::onHooksErrorOccurred);
    connect(hooks_manager_, &HooksManager::finished,
                this, &InstallProgressFramePrivate::onHooksFinished);
    connect(hooks_manager_, &HooksManager::processUpdate,
            this, &InstallProgressFramePrivate::onProgressUpdate);
    connect(hooks_manager_thread_, &QThread::finished,
            hooks_manager_, &HooksManager::deleteLater);
    connect(simulation_timer_, &QTimer::timeout,
            this, &InstallProgressFramePrivate::onSimulationTimerTimeout);
}

void InstallProgressFramePrivate::initUI() {
    title_label_ = new TitleLabel(tr("Installing"));
    comment_label_ = new CommentLabel(
                tr("Here are a few cool things to look out for..."));

    slide_frame_ = new InstallProgressSlideFrame();

    QFrame* tooltip_frame = new QFrame();
    tooltip_frame->setObjectName("tooltip_frame");
    tooltip_frame->setContentsMargins(0, 0, 0, 0);
    tooltip_frame->setFixedSize(kTooltipFrameWidth, kTooltipHeight);
    tooltip_label_ = new TooltipPin(tooltip_frame);
    tooltip_label_->setFixedSize(kTooltipWidth, kTooltipHeight);
    tooltip_label_->setAlignment(Qt::AlignHCenter);
    tooltip_label_->setText("0%");
    // Add left margin.
    tooltip_label_->move(kTooltipLabelMargin, tooltip_label_->y());

    // NOTE(xushaohua): QProgressBar::paintEvent() has performance issue on
    // loongson platform, when chunk style is set. So we override paintEvent()
    // and draw progress bar chunk by hand.
    progress_bar_ = new RoundedProgressBar();
    progress_bar_->setObjectName("progress_bar");
    progress_bar_->setFixedSize(kProgressBarWidth, kProgressBarHeight);
    progress_bar_->setTextVisible(false);
    // Set progress range to [0, 999] so that progress bar can be painted
    // more smoothly.
    progress_bar_->setRange(0, 999);
    progress_bar_->setOrientation(Qt::Horizontal);
    progress_bar_->setValue(0);

    //add main layout
    centerLayout->addWidget(title_label_, 0, Qt::AlignHCenter);
    centerLayout->addSpacing(kMainLayoutSpacing);
    centerLayout->addWidget(comment_label_, 0, Qt::AlignHCenter | Qt::AlignTop);
    centerLayout->addWidget(slide_frame_);
    centerLayout->addWidget(tooltip_frame, 0, Qt::AlignHCenter | Qt::AlignBottom);
    centerLayout->addSpacing(5);
    centerLayout->addWidget(progress_bar_, 0, Qt::AlignHCenter);
    nextButton->hide();

    Q_Q(InstallProgressFrame);
    q->setStyleSheet(ReadFile(":/styles/install_progress_frame.css"));

    progress_animation_ = new QPropertyAnimation(q, "progress", this);
    progress_animation_->setDuration(kProgressAnimationDuration);
    progress_animation_->setEasingCurve(QEasingCurve::InOutCubic);
}

void InstallProgressFramePrivate::updateProgressBar(int progress) {
    Q_ASSERT(progress_bar_->maximum() != 0);
    progress_bar_->setValue(progress);
    const double percentage = progress * 1.0 / progress_bar_->maximum();

    // Calculate percentage of progress.
    const int real_progress = int(floor(percentage * 100.0));
    tooltip_label_->setText(QString("%1%").arg(real_progress));
    // Add right margin.
    int x = int(kProgressBarWidth * percentage - kTooltipLabelMargin);
    if (x < kTooltipLabelMargin) {
        // Add left margin.
        x = kTooltipLabelMargin;
    }
    const int y = tooltip_label_->y();
    tooltip_label_->move(x, y);

    // Force QProgressBar to repaint.

    Q_Q(InstallProgressFrame);
    q->style()->unpolish(progress_bar_);
    q->style()->polish(progress_bar_);
    progress_bar_->repaint();
}

void InstallProgressFramePrivate::onHooksErrorOccurred() {
    failed_ = true;
    slide_frame_->stopSlide();
    WriteInstallSuccessed(false);
    Q_Q(InstallProgressFrame);
    q->m_proxy->nextFrame();
}

void InstallProgressFramePrivate::onHooksFinished() {
    failed_ = false;

    // Set progress value to 100 explicitly.
    onProgressUpdate(100);
    WriteInstallSuccessed(true);
    QTimer::singleShot(kRetainingInterval,
                       this, &InstallProgressFramePrivate::onRetainingTimerTimeout);
}

void InstallProgressFramePrivate::onProgressUpdate(int progress) {
    // Multiple progress value by 10 to fit progress_bar_ range.
    const int virtual_progress = progress * 10;
    progress_animation_->setEndValue(virtual_progress);
    progress_animation_->start();
}

void InstallProgressFramePrivate::onRetainingTimerTimeout() {
    slide_frame_->stopSlide();

    Q_Q(InstallProgressFrame);
    q->m_proxy->nextFrame();
}

void InstallProgressFramePrivate::onSimulationTimerTimeout() {
    // Increase 5% each time.
    const int progress = (progress_bar_->value() + 10) * 100
            / progress_bar_->maximum();
    if (progress > progress_bar_->maximum()) {
        simulation_timer_->stop();
    } else {
        onProgressUpdate(progress);
    }
}

}  // namespace installer

#include "install_progress_frame.moc"
