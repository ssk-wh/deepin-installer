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
#include "ui/delegates/license_delegate.h"

#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <DSysInfo>
#include <QPainterPath>

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
  setFrameType(FrameType::NoLeftLabelExtFrame);

  m_private->initUI();
}

void VirtualMachineFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    m_private->title_label_->setText(::QObject::tr("Friendly Note"));
    m_private->comment_label_->setText(
        ::QObject::tr("You are installing %1 on a virtual machine which may result in sub-optimal performance. For the best experience, please install %1 on a real machine.").arg(DSysInfo::productType() == DSysInfo::Deepin ? ::QObject::tr("Deepin") : LicenseDelegate::product()));
    m_private->nextButton->setText(::QObject::tr("Next"));
  } else {
    FrameInterface::changeEvent(event);
  }
}

void VirtualMachineFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 25, 25);
    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);

    return QWidget::paintEvent(event);
}

bool VirtualMachineFrame::doSelect()
{
    m_private->nextButton->click();
    return true;
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
#ifdef QT_DEBUG
    return true;
#else
    return !GetSettingsBool(kSkipVirtualMachinePage) && IsVirtualMachine() ;
#endif // QT_DBUG
}

QString VirtualMachineFrame::returnFrameName() const
{
    return ::QObject::tr("Friendly Note");
}

void VirtualMachineFramePrivate::initUI() {
  title_label_ = new TitleLabel(::QObject::tr("Friendly Note"));
  //title_label_->setFocusPolicy(Qt::NoFocus);
  comment_label_ = new CommentLabel(
      ::QObject::tr("You are installing %1 on a virtual machine which may result in sub-optimal performance. For the best experience, please install %1 on a real machine.").arg(DSysInfo::productType() == DSysInfo::Deepin ? ::QObject::tr("Deepin") : LicenseDelegate::product()));
  //comment_label_->setFocusPolicy(Qt::NoFocus);
  QHBoxLayout* comment_layout = new QHBoxLayout();
  comment_layout->setContentsMargins(0, 0, 0, 0);
  comment_layout->setSpacing(0);
  comment_layout->addWidget(comment_label_);

  nextButton->setText(::QObject::tr("Next"));

  centerLayout->setContentsMargins(0, 0, 0, 0);
  centerLayout->setSpacing(kMainLayoutSpacing);
  centerLayout->addStretch();
  centerLayout->addWidget(title_label_, 0, Qt::AlignCenter);
  centerLayout->addLayout(comment_layout);
  centerLayout->addStretch();

  q_ptr->setContentsMargins(0, 0, 0, 0);

  disconnect(nextButton, nullptr, nullptr, nullptr);
  connect(nextButton, &QPushButton::clicked, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->m_proxy->nextFrame();
  });
}

}  // namespace installer
#include "virtual_machine_frame.moc"
