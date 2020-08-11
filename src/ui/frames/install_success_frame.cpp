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

#include "ui/frames/install_success_frame.h"
#include "ui/interfaces/frameinterfaceprivate.h"

#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"
#include "ui/frames/consts.h"
#include "ui/utils/widget_util.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QtCore/QEvent>
#include <QPushButton>

namespace installer {
class InstallSuccessFramePrivate : public QObject
{
    Q_OBJECT
public:
    InstallSuccessFramePrivate(InstallSuccessFrame *FF): q_ptr(FF) {}

    Q_DECLARE_PUBLIC(InstallSuccessFrame)
    InstallSuccessFrame *q_ptr=nullptr;

    TitleLabel *title_label_ = new TitleLabel("");
    CommentLabel *eject_label_ = new CommentLabel;
    CommentLabel *comment_label_ = new CommentLabel;

    QLabel *status_label = new QLabel;

    QPushButton *reboot_button_ = new QPushButton;

    void initConnections();
    void initUI();

    void updateTs();
};

InstallSuccessFrame::InstallSuccessFrame(QWidget *parent)
    : QFrame(parent)
    , d_private(new InstallSuccessFramePrivate(this))
{
    this->setObjectName("install_success_frame");
    d_private->initUI();
    d_private->initConnections();
    d_private->updateTs();
}

InstallSuccessFrame::~InstallSuccessFrame()
{

}

void InstallSuccessFrame::changeEvent(QEvent *event)
{
    Q_D(InstallSuccessFrame);

    if (event->type() == QEvent::LanguageChange) {
        d->updateTs();
    } else {
        QFrame::changeEvent(event);
    }
}

void InstallSuccessFramePrivate::updateTs()
{
    reboot_button_->setText(::QObject::tr("Reboot Now"));
    title_label_->setText(::QObject::tr("Successfully Installed"));
    eject_label_->setText(::QObject::tr("Click the button below and then remove the installation media immediately"));
    comment_label_->setText(::QObject::tr("Reboot to discover and enjoy system features"));
}

void InstallSuccessFramePrivate::initConnections()
{
    QObject::connect(reboot_button_, &QPushButton::clicked,
            q_ptr, &InstallSuccessFrame::finished);
}

void InstallSuccessFramePrivate::initUI()
{
    status_label->setPixmap(installer::renderPixmap(":/images/success.svg"));

    QHBoxLayout *comment_layout = new QHBoxLayout();
    comment_layout->setContentsMargins(0, 0, 0, 0);
    comment_layout->setSpacing(0);
    comment_layout->addWidget(comment_label_);

    reboot_button_->setFixedSize(310, 36);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kMainLayoutSpacing);
    layout->addStretch();
    layout->addWidget(status_label, 0, Qt::AlignCenter);
    layout->addWidget(title_label_, 0, Qt::AlignCenter);
    layout->addLayout(comment_layout);
    layout->addStretch();
    layout->addWidget(eject_label_, 0, Qt::AlignCenter);
    layout->addWidget(reboot_button_, 0, Qt::AlignCenter);

    q_ptr->setLayout(layout);
    q_ptr->setContentsMargins(0, 0, 0, 0);
}

}  // namespace installer

#include "install_success_frame.moc"
