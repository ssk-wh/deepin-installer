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

#include "ui/first_boot_setup_window.h"

#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QResizeEvent>
#include <QStackedLayout>
#include <QThread>
#include <QMap>
#include <QShortcut>
#include <DBackgroundGroup>
#include <DTitlebar>
#include <DImageButton>
#include <linux/vt.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/kd.h>

#include "base/thread_util.h"
#include "service/first_boot_hook_worker.h"
#include "service/power_manager.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "third_party/global_shortcut/global_shortcut.h"
#include "ui/frames/first_boot_loading_frame.h"
#include "ui/frames/system_info_frame.h"
#include "ui/frames/timezone_frame.h"
#include "ui/utils/widget_util.h"
#include "ui/xrandr/multi_head_manager.h"
#include "base/command.h"
#include "base/file_util.h"

#include "ui/frames/language_frame.h"
#include "ui/frames/control_platform_frame.h"
#include "ui/frames/inner/system_info_keyboard_frame.h"
#include "ui/frames/control_panel_frame.h"
#include "ui/frames/confirm_quit_frame.h"

DWIDGET_USE_NAMESPACE

namespace {
    const int kLeftViewWidth = 200;
    const int kLeftViewItemWidth = 180;
    const int kLeftViewItemHeight = 36;
    const int kLeftViewItemSpacing = 10;
}

namespace installer {

FirstBootSetupWindow::FirstBootSetupWindow(QWidget *parent)
    : DMainWindow(parent),
      hook_worker_thread_(new QThread(this)),
      hook_worker_(new FirstBootHookWorker()),
      m_showPastFrame(false)
{
  this->setObjectName("first_boot_setup_window");
    if (GetSettingsBool("lightdm_enable_auto_login")) {
        installer::CustomSetSettingsBool(kSystemInfoSetupAfterReboot, false);
    }

  hook_worker_thread_->start();
  hook_worker_->moveToThread(hook_worker_thread_);

  initUI();
  initPages();
  registerShortcut();
  initConnections();

  Qt::WindowFlags flags = windowFlags();
  flags &= ~Qt::WindowMinMaxButtonsHint;
  flags |= Qt::WindowCloseButtonHint;
  setWindowFlags(flags);

  titlebar()->setMenuVisible(false);

  Q_ASSERT(m_frames.count() > 0);
  m_frames.first()->init();

  updateFrameLabelState(m_frames.first(), FrameLabelState::Show);
  stacked_layout_->setCurrentWidget(m_frames.first());

  if ( !GetSettingsBool(kSkipSelectLanguagePage) ||
        GetSettingsBool(kSkipSelectLanguagePageOnFirstBoot)) {
      Q_ASSERT(m_frames.first() == language_frame_);
      nextFrame();
  }
}

FirstBootSetupWindow::~FirstBootSetupWindow() {
  QuitThread(hook_worker_thread_);
}

void FirstBootSetupWindow::fullscreen() {
    ShowFullscreen(this);
}

void FirstBootSetupWindow::nextFrame()
{
    FrameInterface* frame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
    Q_ASSERT(frame != nullptr);

    frame->finished();
    updateFrameLabelState(frame, FrameLabelState::FinishedConfig);

    if (!m_showPastFrame){
        m_frames.removeFirst();
    }

    for (auto it = m_frames.begin(); it != m_frames.end();) {
        if ((*it)->shouldDisplay()){
            // If the current display page is the fallback page clicked by the user
            // , then, the next page is the one that has displayed earlier.
            // If not, then, the next page is the one that displayed for the first time
            // , so, must be initial.
            if (!m_showPastFrame){
                (*it)->init();
            }

            m_hasShowFrames << frame;
            updateFrameLabelState(*it, FrameLabelState::Show);
            stacked_layout_->setCurrentWidget(*it);
            // Can only appear back or not back, to traverse the updates
            if ((*it)->allowPrevious() != m_currentAllowPreviousState || !(*it)->allowPrevious()) {
                updateFrameLabelPreviousState((*it)->allowPrevious());
                m_currentAllowPreviousState = (*it)->allowPrevious();
            }
            m_showPastFrame = false;
            break;
        }
        else {
            (*it)->init();
            (*it)->finished();
            it = m_frames.erase(it);
            m_showPastFrame = false;
        }
    }

    // TODO: reboot or shutdown
}

void FirstBootSetupWindow::exitInstall(bool reboot)
{

}

void FirstBootSetupWindow::hideChildFrame() const
{

}

void FirstBootSetupWindow::setWindowIcon(const QString &path)
{
    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        qDebug() << "set window icon is null. path: " << path;
    } else {
        const int marginSize = this->layout()->margin();
        const int iconSize = this->menuWidget()->height() - 2 * marginSize;

        QLabel *windowLabel = new QLabel;
        windowLabel->setFixedSize(iconSize, iconSize);
        windowLabel->setPixmap(pixmap);
        windowLabel->setParent(this);
        windowLabel->raise();
        windowLabel->setVisible(true);
        windowLabel->move(QPoint(x() + marginSize, y() + marginSize));

        // 设置任务栏窗口图标
        return QMainWindow::setWindowIcon(QIcon(path));
    }
}

void FirstBootSetupWindow::onCloseEvent()
{
    confirm_quit_frame_->display();
}

void FirstBootSetupWindow::initConnections() {
    connect(close_button_, &DImageButton::clicked, this, &FirstBootSetupWindow::onCloseEvent);

    connect(confirm_quit_frame_, &ConfirmQuitFrame::quitCancelled, this, [=](){
               confirm_quit_frame_->close();
               qApp->setActiveWindow(this);
            });
    connect(confirm_quit_frame_, &ConfirmQuitFrame::quitConfirmed,
            this, &FirstBootSetupWindow::shutdownSystem);

    connect(loading_frame_, &FirstBootLoadingFrame::closeButtionChange, this
            , &FirstBootSetupWindow::setCloseButtonVisible);

    connect(hook_worker_, &FirstBootHookWorker::hookFinished,
            this, &FirstBootSetupWindow::onHookFinished);

    connect(control_platform_frame_, &ControlPlatformFrame::requestFinished, this,
            &FirstBootSetupWindow::onControlPlatformFinished);

    connect(monitor_mode_shortcut_, &GlobalShortcut::activated,
            multi_head_manager_, &MultiHeadManager::switchXRandRMode);
    connect(multi_head_manager_, &MultiHeadManager::primaryScreenChanged,
            this, &FirstBootSetupWindow::onPrimaryScreenChanged);
    connect(back_button_, &DImageButton::clicked, this, &FirstBootSetupWindow::backPage);
    connect(stacked_layout_, &QStackedLayout::currentChanged, back_button_, &DImageButton::raise);

    connect(m_frameLabelsView, &DListView::clicked, this, &FirstBootSetupWindow::onFrameLabelsViewClicked);

    connect(control_panel_shortcut_, &QShortcut::activated,
            control_panel_frame_, &ControlPanelFrame::toggleVisible);

    connect(language_frame_, &LanguageFrame::timezoneUpdated,
            timezone_frame_, &TimezoneFrame::updateTimezoneBasedOnLanguage);

    connect(language_frame_, &LanguageFrame::coverMainWindowFrameLabelsView
            , this, &FirstBootSetupWindow::updateFrameLabelPreviousState);
}

void FirstBootSetupWindow::initUI() {
    back_button_ = new DImageButton(this);
    back_button_->setFixedSize(48, 38);
    back_button_->move(20, 20);
    back_button_->setNormalPic(":/images/back_normal.svg");
    back_button_->setHoverPic(":/images/back_hover.svg");
    back_button_->setPressPic(":/images/back_pressed.svg");
    back_button_->setDisabledPic(":/images/back_disabled.svg");
    back_button_->hide();

    close_button_ = new DImageButton(this);
    close_button_->setObjectName("close_button");
    close_button_->setFocusPolicy(Qt::TabFocus);
    close_button_->setFixedSize(40, 40);
    close_button_->setNormalPic(":/images/close_normal.svg");
    close_button_->setHoverPic(":/images/close_normal.svg");
    close_button_->setPressPic(":/images/close_normal.svg");

    stacked_layout_ = new QStackedLayout;
    stacked_layout_->setContentsMargins(0, 0, 0, 0);
    stacked_layout_->setSpacing(0);

    QVBoxLayout* vbox_layout = new QVBoxLayout();
    vbox_layout->setContentsMargins(0, 0, 0, 0);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(stacked_layout_);

    QWidget* contentWidget = new QWidget;
    contentWidget->setContentsMargins(0, 0, 0, 0);
    contentWidget->setLayout(vbox_layout);

    m_frameSelectedLayout = new QVBoxLayout;
    m_frameSelectedLayout->setContentsMargins(kLeftViewItemSpacing, 0, kLeftViewItemSpacing, 0);
    m_frameSelectedLayout->setSpacing(0);

    m_frameSelectedListWidget = new QWidget;
    m_frameSelectedListWidget->setObjectName("frameSelectedListWidget");
    m_frameSelectedListWidget->setLayout(m_frameSelectedLayout);
    m_frameSelectedListWidget->setFixedWidth(kLeftViewWidth);

    m_frameLabelsViewCoverWidget = new QWidget(m_frameSelectedListWidget);
    m_frameLabelsViewCoverWidget->setObjectName("frameLabelsViewCoverWidget");
    m_frameLabelsViewCoverWidget->setStyleSheet("{background-color: rgba(255, 255, 255, 0.5);}");
    m_frameLabelsViewCoverWidget->hide();

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(m_frameSelectedListWidget);
    mainLayout->addSpacing(1);
    mainLayout->addWidget(contentWidget);

    DBackgroundGroup* bgGroup = new DBackgroundGroup;
    bgGroup->setContentsMargins(10, 10, 10, 10);
    bgGroup->setLayout(mainLayout);

    setContentsMargins(0, 0, 0, 0);
    setCentralWidget(bgGroup);

    control_panel_frame_ = new ControlPanelFrame(this);
    control_panel_frame_->hide();

    this->setFocusPolicy(Qt::TabFocus);
}

void FirstBootSetupWindow::initPages()
{
    confirm_quit_frame_ = new ConfirmQuitFrame(this);
    confirm_quit_frame_->hide();
    language_frame_ = new LanguageFrame(this);
    m_keyboardFrame = new SystemInfoKeyboardFrame(this);
    system_info_frame_ = new SystemInfoFrame(this);
    timezone_frame_ = new TimezoneFrame(this);
    loading_frame_ = new FirstBootLoadingFrame(this);
    control_platform_frame_ = new ControlPlatformFrame(this);

    m_originalFrames = {
        language_frame_,
        m_keyboardFrame,
        timezone_frame_,
        system_info_frame_,
        control_platform_frame_,
        loading_frame_,
    };

    for (FrameInterface* frame : m_originalFrames) {
        stacked_layout_->addWidget(frame);
        m_frames << frame;
    }

    m_frameLabelsView = new DListView(this);
    m_frameLabelsView->setResizeMode(QListView::Adjust);
    m_frameLabelsView->setItemSize(QSize(kLeftViewItemWidth, kLeftViewItemHeight + kLeftViewItemSpacing));
    m_frameLabelsModel = new QStandardItemModel();
    m_frameLabelsView->setModel(m_frameLabelsModel);

    m_frameSelectedLayout->addSpacing(60);
    m_frameSelectedLayout->addWidget(m_frameLabelsView, 0, Qt::AlignHCenter);

    constructLabelView();

    language_frame_->acceptLicense(true);
}


FrameInterface *FirstBootSetupWindow::getFrameInterface(QStandardItem *item) const
{
    for (auto it = m_frameModelItemMap.begin(); it != m_frameModelItemMap.end(); ++it) {
        if (it.value() == item) {
            return it.key();
        }
    }

    return nullptr;
}

void FirstBootSetupWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        for (int i = 0; i < m_frameLabelsModel->rowCount(); ++i) {
            QStandardItem* item = m_frameLabelsModel->item(i);
            FrameInterface* frame = getFrameInterface(item);
            if (frame) {
                item->setText(frame->returnFrameName());
            }
        }
    }
    else {
        QWidget::changeEvent(event);
    }
}

void FirstBootSetupWindow::resizeEvent(QResizeEvent *event)
{
    if (close_button_) {
      const int marginSize = this->layout()->margin();
      close_button_->move(width() - close_button_->width() - marginSize, marginSize);
      close_button_->raise();
    }

    DMainWindow::resizeEvent(event);
}

void FirstBootSetupWindow::constructLabelView()
{
    m_frameLabelsModel->clear();

    int i = 1;
    for (FrameInterface* frame : m_originalFrames){
        if (!frame->shouldDisplay() || frame->frameType() != FrameType::Frame){
            continue;
        }

        DStandardItem* item = new DStandardItem;
        QString pixPathTemplate(":/images/NO_inactive%1.svg");
        item->setIcon(QIcon(pixPathTemplate.arg(i)));
        ++i;

        item->setText(tr(frame->returnFrameName().toLatin1().data()));
        QVariant framePointer = QVariant::fromValue(frame);
        item->setData(framePointer, FramePointerRole);
        item->setFlags(Qt::ItemFlag::NoItemFlags);

        DViewItemAction* action = new DViewItemAction(Qt::AlignmentFlag::AlignVCenter);
        action->setIcon(QIcon(":/images/done_inactive.svg"));
        action->setVisible(false);
        item->setActionList(Qt::Edge::RightEdge, {action});

        m_frameLabelsModel->appendRow(item);
        m_frameModelItemMap[frame] = item;
    }
}

void FirstBootSetupWindow::registerShortcut() {
    control_panel_shortcut_ = new QShortcut(QKeySequence("Ctrl+Alt+Z"), this);
    control_panel_shortcut_->setContext(Qt::ApplicationShortcut);

  // Note(xushaohua): Super key is named Meta key in Qt namespace.
  monitor_mode_shortcut_ = new GlobalShortcut(QKeySequence("Meta+P"), this);
  if (!monitor_mode_shortcut_->registerNow()) {
    qWarning() << "Failed to register global shortcut of Meta+P"
               << "Fallback to Ctrl+Alt+P";
    delete monitor_mode_shortcut_;
    monitor_mode_shortcut_ = new GlobalShortcut(QKeySequence("Ctrl+Alt+P"),
                                                this);
    if (!monitor_mode_shortcut_->registerNow()) {
      qWarning() << "Failed to register global shortcut of Ctrl+Alt+P";
    }
  }

  multi_head_manager_ = new MultiHeadManager(this);
}

void FirstBootSetupWindow::onHookFinished(bool ok) {
  if (!ok) {
    qCritical() << "First boot hook failed!";
  }

  if (QFile::exists("/usr/sbin/lightdm")) {
      qDebug() << SpawnCmd("systemctl", QStringList() << "restart" << "lightdm");
  }
  else {
    if (!changeToTTY(2)) {
      if (!RebootSystemWithMagicKey()) {
          RebootSystem();
      }
      qDebug() << SpawnCmd("killall", QStringList() << "lightdm");
    }
  }
}

void FirstBootSetupWindow::onPrimaryScreenChanged(const QRect& geometry) {
  qDebug() << "on primary screen changed" << geometry;
  this->move(geometry.topLeft());
  this->setFixedSize(geometry.size() / devicePixelRatioF());
}

void FirstBootSetupWindow::onLanguageSelected()
{
    if (GetSettingsBool(kSkipTimezonePage)) {
        return onTimezoneFinished();
    }
    else {
      stacked_layout_->setCurrentWidget(timezone_frame_);
      updateBackButtonVisible(stacked_layout_->currentWidget());
    }
}

void FirstBootSetupWindow::onSystemInfoFinished() {
  if (GetSettingsBool(kSkipNetworkPage)) {
    this->onNetworkFinished();
  } else {
      //    stacked_layout_->setCurrentWidget(network_frame_);
    updateBackButtonVisible(stacked_layout_->currentWidget());
  }
}

void FirstBootSetupWindow::onNetworkFinished()
{
    if (GetSettingsBool(kSkipControlPlatformPage)) {
        return onControlPlatformFinished();
    }
    else {
        stacked_layout_->setCurrentWidget(control_platform_frame_);
        updateBackButtonVisible(stacked_layout_->currentWidget());
    }
}

void FirstBootSetupWindow::onControlPlatformFinished()
{
    emit hook_worker_->startHook();
}

void FirstBootSetupWindow::onTimezoneFinished() {
    if (!GetSettingsBool(kSkipSystemInfoPage)) {
        stacked_layout_->setCurrentWidget(system_info_frame_);
    }
    else {
        onSystemInfoFinished();
    }
}

void FirstBootSetupWindow::backPage()
{
    // TODO: this function will be delete later
}

void FirstBootSetupWindow::updateBackButtonVisible(QWidget* page)
{
    // TODO: this function will be delete later
}

bool FirstBootSetupWindow::changeToTTY(int ttyNum) const
{
    auto is_a_console = [=](int fd) {
        char arg { 0 };
        return (isatty(fd) && ioctl(fd, KDGKBTYPE, &arg) == 0 && ((arg == KB_101) || (arg == KB_84)));
    };

    auto open_a_console = [=](const char *fnam) -> int {
        int fd;
        fd = open(fnam, O_RDWR);
        if (fd < 0)
            fd = open(fnam, O_WRONLY);
        if (fd < 0)
            fd = open(fnam, O_RDONLY);
        if (fd < 0)
            return -1;
        return fd;
    };

    auto getfd = [=](const char *fnam) -> int {
        int fd;

        if (fnam) {
            if ((fd = open_a_console(fnam)) >= 0) {
                if (is_a_console(fd))
                    return fd;
            }
            qWarning() << stderr << "Couldn't open %s" << fnam;
        }

        QStringList conspath {
            "/proc/self/fd/0",
            "/dev/tty",
            "/dev/tty0",
            "/dev/vc/0",
            "/dev/systty",
            "/dev/console",
        };

        for (const QString& path : conspath) {
            if ((fd = open_a_console(path.toUtf8().data())) >= 0) {
                if (is_a_console(fd)) {
                    return fd;
                }
            }
        }

        for (fd = 0; fd < 3; fd++)
            if (is_a_console(fd))
                return fd;

        qWarning() << stderr << "Couldn't get a file descriptor referring to the console";

        return fd;
    };

    int fd;
    if ((fd = getfd(nullptr)) < 0) {
        qWarning() << "Couldn't get a file descriptor referring to the console";
    }

    if (ioctl(fd, VT_ACTIVATE, ttyNum)) {
        qWarning() << "ioctl VT_ACTIVATE" << ttyNum;
        return false;
    }

    if (ioctl(fd, VT_WAITACTIVE, ttyNum)) {
        qWarning() << "ioctl VT_WAITACTIVE" << ttyNum;
        return false;
    }

    return true;
}

void FirstBootSetupWindow::previousFrameSelected(FrameInterface *frame)
{
    FrameInterface* currentFrame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
    Q_ASSERT(currentFrame != nullptr);

    if (m_showPastFrame){
        updateFrameLabelState(currentFrame, FrameLabelState::FinishedConfig);
    }
    else {
        updateFrameLabelState(currentFrame, FrameLabelState::Initial);
    }

    updateFrameLabelState(frame, FrameLabelState::Show);
    stacked_layout_->setCurrentWidget(frame);

    m_showPastFrame = true;
}

void FirstBootSetupWindow::onFrameLabelsViewClicked(const QModelIndex &index)
{
    Q_ASSERT(sender() == m_frameLabelsView);

    FrameInterface* framePointer = index.data(FramePointerRole).value<FrameInterface*>();
    Q_ASSERT(framePointer);
    if (!m_frameModelItemMap[framePointer]->flags().testFlag(Qt::ItemFlag::ItemIsEnabled)){
        // TODO: will user another way implement.
        FrameInterface* frame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
        Q_ASSERT(frame);
        m_frameLabelsView->setCurrentIndex(m_frameLabelsModel->indexFromItem(
                                               m_frameModelItemMap[frame]));
        return;
    }

    previousFrameSelected(framePointer);
}

void FirstBootSetupWindow::updateFrameLabelPreviousState(bool allow)
{
    FrameInterface* currentFrame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
    Q_ASSERT(currentFrame != nullptr);
    for (FrameInterface* frame : m_hasShowFrames) {
        if (frame != currentFrame) {
            if (allow) {
                updateFrameLabelState(frame, FrameLabelState::FinishedConfig);
            } else {
                updateFrameLabelState(frame, FrameLabelState::Previous);
            }
        }
    }
}

void FirstBootSetupWindow::shutdownSystem()
{
    if (!ShutdownSystem()) {
        qWarning() << "ShutdownSystem() failed!";
    }

    if (!ShutdownSystemWithMagicKey()) {
        qWarning() << "ShutdownSystemWithMagicKey() failed!";
    }
}

void FirstBootSetupWindow::setCloseButtonVisible(bool visible)
{
    close_button_->setVisible(visible);
}

void FirstBootSetupWindow::updateFrameLabelState(FrameInterface *frame, FrameLabelState state)
{
    if (!m_frameModelItemMap.contains(frame)){
        return;
    }

    DStandardItem* item = m_frameModelItemMap[frame];
    switch (state) {
    case FrameLabelState::Initial:
        item->actionList(Qt::Edge::RightEdge).first()->setVisible(false);
        item->setFlags(Qt::ItemFlag::NoItemFlags);
        break;
    case FrameLabelState::Show:
        item->actionList(Qt::Edge::RightEdge).first()->setVisible(false);
        item->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);

        m_frameLabelsView->setCurrentIndex(m_frameLabelsModel->indexFromItem(m_frameModelItemMap[frame]));
        break;
    case FrameLabelState::FinishedConfig:
        item->actionList(Qt::Edge::RightEdge).first()->setVisible(true);
        item->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        break;
    case FrameLabelState::Previous:
        item->actionList(Qt::Edge::RightEdge).first()->setVisible(true);
        item->setFlags(Qt::ItemFlag::NoItemFlags);
        break;
    default:
        qWarning() << "invalid state value";
        break;
    }
}

}  // namespace installer
