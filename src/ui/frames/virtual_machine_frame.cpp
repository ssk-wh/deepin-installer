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

#include "ui/interfaces/frameinterfaceprivate.h"

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
class VirtualMachineFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT

    friend VirtualMachineFrame;

public:
    explicit VirtualMachineFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate (parent)
        , q_ptr(qobject_cast<VirtualMachineFrame* >(parent))
    {}

    void initUI();

    VirtualMachineFrame* q_ptr = nullptr;

    TitleLabel* title_label_ = nullptr;
    CommentLabel* comment_label_ = nullptr;
};

VirtualMachineFrame::VirtualMachineFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new VirtualMachineFramePrivate(this))
{
  setObjectName("virtual_machine_frame");

  m_private->initUI();
}

void VirtualMachineFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    m_private->title_label_->setText(tr("Friendly Reminder"));
    m_private->comment_label_->setText(
        tr("You are using a virtual machine which will affect system performance and user experience. "
           "To get a smoother experience, please install %1 in a real environment").arg(DSysInfo::productType() == DSysInfo::Deepin ? tr("Deepin") : tr("UOS")));
    next_button_->setText(tr("Continue"));
  } else {
    FrameInterface::changeEvent(event);
  }
}

VirtualMachineFrame::~VirtualMachineFrame()
{

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

void VirtualMachineFramePrivate::initUI() {
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

  nextButton->setText(tr("Continue"));

  centerLayout->setContentsMargins(0, 0, 0, 0);
  centerLayout->setSpacing(kMainLayoutSpacing);
  centerLayout->addStretch();
  centerLayout->addWidget(title_label_, 0, Qt::AlignCenter);
  centerLayout->addLayout(comment_layout);
  centerLayout->addStretch();

  q_ptr->setContentsMargins(0, 0, 0, 0);
}

}  // namespace installer
#include "virtual_machine_frame.moc"
