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

#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QPlainTextEdit>
#include <QLabel>

#include "base/file_util.h"
#include "ui/delegates/main_window_util.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/qr_widget.h"
#include "ui/widgets/title_label.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/title_label.h"


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
    NavButton *reboot_button_ ;
    NavButton *save_log_button_;
    QRWidget *qr_widget_;
    QPlainTextEdit *m_plainTextEdit ;
    QPushButton *control_button_ ;
    QScrollArea *m_scrollArea ;

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
        qr_widget_->setText("Error: failed to read log");
    }

    void onControlButtonClicked();

    InstallFailedFrame *m_ptr;
};

InstallFailedFrame::InstallFailedFrame(QWidget *parent) : QFrame(parent)
    , d_private(new InstallFailedFramePrivate(this))
{
    this->setObjectName("install_failed_frame");

    d_private->initUI();
    d_private->initConnections();

    // Show QR widget by default.
    d_private->m_scrollArea->hide();
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

    if (encoded_msg.isEmpty()) {
        // If encoded_msg if empty, qr_widget will generate a rectangle filled with
        // red color, which is not what we expect.
        encoded_msg = EncodeErrorMsg("Error: failed to read log");
    }
    d->updatetx();
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
    connect(save_log_button_, &NavButton::clicked,
            m_ptr, &InstallFailedFrame::showSaveLogFrame);
}

void InstallFailedFramePrivate::initUI()
{    
    QLabel *status_label = new QLabel();
    status_label->setPixmap(installer::renderPixmap(":/images/fail.svg"));
    title_label_ = new TitleLabel("");
    comment_label_ = new CommentLabel;

    updatetx();

    QHBoxLayout *comment_layout = new QHBoxLayout();
    comment_layout->setContentsMargins(0, 0, 0, 0);
    comment_layout->setSpacing(0);
    comment_layout->addWidget(comment_label_);

    m_plainTextEdit = new QPlainTextEdit;
    m_plainTextEdit->setObjectName("plainTextEdit");

    QVBoxLayout *labelLayout = new QVBoxLayout;
    labelLayout->setMargin(0);
    labelLayout->addWidget(m_plainTextEdit);

    QWidget *labelWidget = new QWidget;
    labelWidget->setLayout(labelLayout);

    QFrame *content_frame = new QFrame();
    content_frame->setObjectName("content_frame");
    content_frame->setFixedSize(kContentWindowWidth, kContentWindowHeight);

    m_scrollArea = new QScrollArea(content_frame);
    m_scrollArea->setWidget(labelWidget);
    m_scrollArea->setObjectName("scrollarea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFocusPolicy(Qt::NoFocus);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_scrollArea->setContentsMargins(0, 0, 0, 0);
    m_scrollArea->setFixedWidth(kContentWindowWidth - kControlButtonSize - 2);
    m_scrollArea->setFixedHeight(kContentWindowHeight);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollArea->setStyleSheet("background: transparent;");
    QScroller::grabGesture(m_scrollArea, QScroller::TouchGesture);

    qr_widget_ = new QRWidget(content_frame);
    qr_widget_->setMargin(kQrMargin);
    qr_widget_->setFixedSize(kQrWindowSize, kQrWindowSize);
    qr_widget_->move((kContentWindowWidth - kQrWindowSize) / 2,
                     (kContentWindowHeight - kQrWindowSize) / 2);

    control_button_ = new PointerButton(content_frame);
    control_button_->setObjectName("control_button");
    control_button_->setFlat(true);
    control_button_->setFixedSize(kControlButtonSize, kControlButtonSize);
    // Move control_button_ to top-right corner of content area.
    control_button_->move(kContentWindowWidth - kControlButtonSize, 0);

    reboot_button_ = new NavButton;
    save_log_button_ = new NavButton;

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
    if (m_scrollArea->isVisible()) {
        m_scrollArea->setVisible(false);
        qr_widget_->setVisible(true);
    } else {
        m_scrollArea->setVisible(true);
        qr_widget_->setVisible(false);
    }
}

}// namespace installer

#include "install_failed_frame.moc"
