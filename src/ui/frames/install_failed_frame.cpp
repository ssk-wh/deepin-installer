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
#include <DFrame>

DWIDGET_USE_NAMESPACE

namespace installer {

namespace {
const int kContentWindowWidth = 538;
const int kContentWindowHeight = 226;

const int kQrMargin = 8;
const int kQrWindowSize = 142;

const int kControlButtonSize = 32;

const int kButtonWidth = 200;
const int kButtonHeight = 36;

const int kCommentLabelWidth = 363;
}  // namespace

class InstallFailedFramePrivate : public QObject
{
    Q_OBJECT
public:
    InstallFailedFramePrivate(InstallFailedFrame *qq) : m_ptr(qq) {}

    QLabel *title_label_;
    CommentLabel *comment_label_ ;
    QPushButton *reboot_button_ ;
    QPushButton *saveLogButton ;
    QRWidget *qr_widget_;
    DFrame* qrParentWidget;
    QPlainTextEdit *m_plainTextEdit ;
    QPushButton *control_button_ ;
    QStackedLayout* stacked_layout;

    void initConnections();
    void initUI();
    void updatetx()
    {
        title_label_->setText(::QObject::tr("Installation Failed"));
        comment_label_->setText(
            ::QObject::tr("Sorry for the trouble. Please photo or scan the QR code to send us the error log, "
               "or save the log to an external disk. We will help solve the issue."));
        reboot_button_->setText(::QObject::tr("Exit"));
        saveLogButton->setText(::QObject::tr("Save Log"));
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

    QPalette palette;
    palette.setColor(QPalette::Text, QColor(66, 154, 216));
    d->m_plainTextEdit->setPlainText(msg);
    d->m_plainTextEdit->moveCursor(QTextCursor::End);
    d->m_plainTextEdit->ensureCursorVisible();
    d->m_plainTextEdit->setPalette(palette);

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
    connect(saveLogButton, &QPushButton::clicked,
            m_ptr, &InstallFailedFrame::showSaveLogFrame);
}

void InstallFailedFramePrivate::initUI()
{
    QLabel *status_label = new QLabel();
    status_label->setPixmap(installer::renderPixmap(":/images/fail.svg"));
    title_label_ = new TitleLabel("");

    comment_label_ = new CommentLabel;
    comment_label_->setFixedWidth(kCommentLabelWidth);
    QHBoxLayout* comment_layout = new QHBoxLayout();
    comment_layout->setContentsMargins(0, 0, 0, 0);
    comment_layout->setSpacing(0);
    comment_layout->addWidget(comment_label_);

    m_plainTextEdit = new QPlainTextEdit;
    m_plainTextEdit->setFrameShape(QFrame::NoFrame);
    m_plainTextEdit->setObjectName("plainTextEdit");
    m_plainTextEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_plainTextEdit->setReadOnly(true);
    m_plainTextEdit->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    DFrame *content_frame = new DFrame();
    content_frame->setContentsMargins(1, 1, 1, 1);
    content_frame->setObjectName("content_frame");
    content_frame->setFrameRounded(true);
    content_frame->setFixedSize(kContentWindowWidth, kContentWindowHeight);

    qr_widget_ = new QRWidget(content_frame);
    qr_widget_->setMargin(kQrMargin);
    qr_widget_->setFixedSize(kQrWindowSize, kQrWindowSize);

    QVBoxLayout* qrLayout = new QVBoxLayout;
    qrLayout->setMargin(0);
    qrLayout->setSpacing(0);
    qrLayout->addWidget(qr_widget_, 0, Qt::AlignCenter);

    qrParentWidget = new DFrame;
    qrParentWidget->setFrameShape(QFrame::Shape::NoFrame);
    qrParentWidget->setLayout(qrLayout);

    stacked_layout = new QStackedLayout;
    stacked_layout->addWidget(m_plainTextEdit);
    stacked_layout->addWidget(qrParentWidget);

    stacked_layout->setAlignment(qr_widget_, Qt::AlignCenter);

    QHBoxLayout* switchLayout = new QHBoxLayout;
    switchLayout->setContentsMargins(5, 5, 5, 5);
    switchLayout->setSpacing(0);
    switchLayout->addLayout(stacked_layout);

    content_frame->setLayout(switchLayout);

    control_button_ = new QPushButton(content_frame);
    control_button_->setFocusPolicy(Qt::NoFocus);
    control_button_->setIcon(QIcon(installer::renderPixmap(":/images/failed_qr.svg")));
    control_button_->setObjectName("control_button");
    control_button_->setFlat(true);
    control_button_->setFixedSize(kControlButtonSize, kControlButtonSize);
    // Move control_button_ to top-right corner of content area.
    control_button_->move(kContentWindowWidth - kControlButtonSize, 0);
    control_button_->raise();
    control_button_->show();

    reboot_button_ = new QPushButton;
    reboot_button_->setFixedSize(kButtonWidth, kButtonHeight);
    reboot_button_->setFocusPolicy(Qt::NoFocus);
    saveLogButton = new QPushButton;
    saveLogButton->setFixedSize(kButtonWidth, kButtonHeight);
    saveLogButton->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(reboot_button_, 0, Qt::AlignHCenter | Qt::AlignLeft);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(saveLogButton, 0, Qt::AlignHCenter | Qt::AlignRight);
    QWidget *buttonWrapWidget = new QWidget;
    buttonWrapWidget->setLayout(buttonLayout);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->addWidget(status_label, 0, Qt::AlignHCenter);
    layout->addWidget(title_label_, 0, Qt::AlignHCenter);
    layout->addLayout(comment_layout);
    layout->addStretch();
    layout->addWidget(content_frame, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(buttonWrapWidget, 0, Qt::AlignHCenter);

    m_ptr->setLayout(layout);
    m_ptr->setContentsMargins(0, 0, 0, 0);
}

void InstallFailedFramePrivate::onControlButtonClicked()
{
    // Toggle visibility of m_scrollArea and qr_widget_.
    if (stacked_layout->currentWidget() == m_plainTextEdit) {
        stacked_layout->setCurrentWidget(qrParentWidget);
        control_button_->setIcon(QIcon(installer::renderPixmap(":/images/failed_data.svg")));
    }
    else {
        stacked_layout->setCurrentWidget(m_plainTextEdit);
        control_button_->setIcon(QIcon(installer::renderPixmap(":/images/failed_qr.svg")));
    }

    control_button_->raise();
}

}// namespace installer

#include "install_failed_frame.moc"
