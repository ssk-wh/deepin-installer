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
#include "ui/frames/virtual_machine_frame.h"

#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/title_label.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "sysinfo/virtual_machine.h"

#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>

DCORE_USE_NAMESPACE

namespace installer {

VirtualMachineFrame::VirtualMachineFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface (FrameType::Frame, frameProxyInterface, parent) {
  setObjectName("virtual_machine_frame");

  initUI();
  initConnections();
}

void VirtualMachineFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(tr("Friendly Reminder"));
    comment_label_->setText(
        tr("You are using a virtual machine which will affect system performance and user experience. "
           "To get a smoother experience, please install %1 in a real environment").arg(DSysInfo::productType() == DSysInfo::Deepin ? tr("Deepin") : tr("UOS")));
    next_button_->setText(tr("Continue"));
  } else {
    FrameInterface::changeEvent(event);
  }
}

void VirtualMachineFrame::init()
{

}

void VirtualMachineFrame::finished()
{

}

bool VirtualMachineFrame::shouldDisplay() const
{
    return !GetSettingsBool(kSkipVirtualMachinePage) && IsVirtualMachine() ;
}
void VirtualMachineFrame::initConnections() {
    connect(next_button_, &QPushButton::clicked,
            this, [=] {
        m_proxy->nextFrame();
    });
}

void VirtualMachineFrame::initUI() {
  title_label_ = new TitleLabel(tr("Friendly Reminder"));
  comment_label_ = new CommentLabel(
      tr("System has detected that you are using a virtual machine, "
         "which will affect the system performance and operation experience, "
         "for a smooth experience, it is recommended to install %1 "
         "in real-machine environment").arg(DSysInfo::productType() == DSysInfo::Deepin ? tr("Deepin") : tr("UOS")));
  QHBoxLayout* comment_layout = new QHBoxLayout();
  comment_layout->setContentsMargins(0, 0, 0, 0);
  comment_layout->setSpacing(0);
  comment_layout->addWidget(comment_label_);

  next_button_ = new NavButton(tr("Continue"));

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(kMainLayoutSpacing);
  layout->addStretch();
  layout->addWidget(title_label_, 0, Qt::AlignCenter);
  layout->addLayout(comment_layout);
  layout->addStretch();
  layout->addWidget(next_button_, 0, Qt::AlignCenter);

  setLayout(layout);
  setContentsMargins(0, 0, 0, 0);
}

}  // namespace installer
