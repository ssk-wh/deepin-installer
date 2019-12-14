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

#include "install_failed_frame.h"

#include "ui/frames/install_failed_frame.h"
#include "base/file_util.h"
#include "ui/delegates/main_window_util.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/qr_widget.h"
#include "ui/widgets/title_label.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/pointer_button.h"

#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>

namespace installer {

namespace {

const int kContentWindowWidth = 580;
const int kContentWindowHeight = 320;

const int kQrMargin = 8;
const int kQrWindowSize = 280;

const int kControlButtonSize = 32;

}  // namespace

class InstallFailedFramePrivate : public QObject
{
    Q_OBJECT
public:
    InstallFailedFramePrivate(InstallFailedFrame *qq) : m_ptr(qq) {}

    QLabel *title_label_;
    CommentLabel *comment_label_ ;
    QPushButton *reboot_button_ ;
    QPushButton *save_log_button_;
    QRWidget *qr_widget_;
    QWidget* qrParentWidget;
    QPlainTextEdit *m_plainTextEdit ;
    QPushButton *control_button_ ;
    QStackedLayout* stacked_layout;

    void initConnections();
    void initUI();
    void updatetx()
    {
        title_label_->setText(tr("Installation Failed"));
        comment_label_->setText(
            tr("Sorry for the trouble. Please photo or scan the QR code to send us the error log, "
               "or save the log to an external disk. We will help solve the issue."));
        reboot_button_->setText(tr("Exit"));
        save_log_button_->setText(tr("Save Log"));
    }

    void onControlButtonClicked();

    InstallFailedFrame *m_ptr;
};

InstallFailedFrame::InstallFailedFrame(QWidget *parent) : QFrame(parent)
    , d_private(new InstallFailedFramePrivate(this))
{
    this->setObjectName("install_failed_frame");

    d_private->initUI();
    d_private->updatetx();
    d_private->initConnections();
}

InstallFailedFrame::~InstallFailedFrame()
{

}

void InstallFailedFrame::updateMessage()
{
    Q_D(InstallFailedFrame);

    QString msg, encoded_msg;
    if (!ReadErrorMsg(msg, encoded_msg)) {
        msg = "Error: failed to read log file!";
        encoded_msg = EncodeErrorMsg(msg);
    }

    d->m_plainTextEdit->setPlainText(msg);
    d->m_plainTextEdit->moveCursor(QTextCursor::End);
    d->m_plainTextEdit->ensureCursorVisible();

    if (encoded_msg.isEmpty()) {
        // If encoded_msg if empty, qr_widget will generate a rectangle filled with
        // red color, which is not what we expect.
        encoded_msg = EncodeErrorMsg("Error: failed to read log");
    }
    d->qr_widget_->setText(encoded_msg);
}

void InstallFailedFrame::changeEvent(QEvent *event)
{
    Q_D(InstallFailedFrame);

    if (event->type() == QEvent::LanguageChange) {
        d->updatetx();
    } else {
        QFrame::changeEvent(event);
    }
}

void InstallFailedFramePrivate::initConnections()
{
    connect(control_button_, &QPushButton::clicked,
            this, &InstallFailedFramePrivate::onControlButtonClicked);
    connect(reboot_button_, &QPushButton::clicked,
            m_ptr, &InstallFailedFrame::finished);
    connect(save_log_button_, &QPushButton::clicked,
            m_ptr, &InstallFailedFrame::showSaveLogFrame);
}

void InstallFailedFramePrivate::initUI()
{
    QLabel *status_label = new QLabel();
    status_label->setPixmap(installer::renderPixmap(":/images/fail.svg"));
    title_label_ = new TitleLabel("");
    comment_label_ = new CommentLabel;

    QHBoxLayout *comment_layout = new QHBoxLayout();
    comment_layout->setContentsMargins(0, 0, 0, 0);
    comment_layout->setSpacing(0);
    comment_layout->addWidget(comment_label_);

    m_plainTextEdit = new QPlainTextEdit;
    m_plainTextEdit->setObjectName("plainTextEdit");
    m_plainTextEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_plainTextEdit->setReadOnly(true);
    m_plainTextEdit->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFrame *content_frame = new QFrame();
    content_frame->setObjectName("content_frame");
    content_frame->setFixedSize(kContentWindowWidth, kContentWindowHeight);

    qr_widget_ = new QRWidget(content_frame);
    qr_widget_->setStyleSheet("background: transparent");
    qr_widget_->setMargin(kQrMargin);
    qr_widget_->setFixedSize(kQrWindowSize, kQrWindowSize);

    QVBoxLayout* qrLayout = new QVBoxLayout;
    qrLayout->setMargin(0);
    qrLayout->setSpacing(0);
    qrLayout->addWidget(qr_widget_, 0, Qt::AlignCenter);

    qrParentWidget = new QWidget;
    qrParentWidget->setLayout(qrLayout);

    stacked_layout = new QStackedLayout;
    stacked_layout->addWidget(qrParentWidget);
    stacked_layout->addWidget(m_plainTextEdit);

    stacked_layout->setAlignment(qr_widget_, Qt::AlignCenter);

    content_frame->setLayout(stacked_layout);

    control_button_ = new PointerButton(content_frame);
    control_button_->setObjectName("control_button");
    control_button_->setFlat(true);
    control_button_->setFixedSize(kControlButtonSize, kControlButtonSize);
    // Move control_button_ to top-right corner of content area.
    control_button_->move(kContentWindowWidth - kControlButtonSize, 0);
    control_button_->raise();
    control_button_->show();

    reboot_button_ = new QPushButton;
    save_log_button_ = new QPushButton;

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kMainLayoutSpacing);
    layout->addStretch();
    layout->addWidget(status_label, 0, Qt::AlignCenter);
    layout->addWidget(title_label_, 0, Qt::AlignCenter);
    layout->addLayout(comment_layout);
    layout->addStretch();
    layout->addWidget(content_frame, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(save_log_button_, 0, Qt::AlignCenter);
    layout->addWidget(reboot_button_, 0, Qt::AlignCenter);

    m_ptr->setLayout(layout);
    m_ptr->setContentsMargins(0, 0, 0, 0);
    m_ptr->setStyleSheet(ReadFile(":/styles/install_failed_frame.css"));
}

void InstallFailedFramePrivate::onControlButtonClicked()
{
    // Toggle visibility of m_scrollArea and qr_widget_.
    if (stacked_layout->currentWidget() == m_plainTextEdit) {
        stacked_layout->setCurrentWidget(qrParentWidget);
    }
    else {
        stacked_layout->setCurrentWidget(m_plainTextEdit);
    }

    control_button_->raise();
}

}// namespace installer

#include "install_failed_frame.moc"
