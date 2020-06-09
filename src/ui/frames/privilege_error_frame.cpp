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

#include "ui/frames/privilege_error_frame.h"
#include "ui/interfaces/frameinterfaceprivate.h"

#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/title_label.h"
#include "sysinfo/users.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>

namespace installer {

class PrivilegeErrorFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT

    friend PrivilegeErrorFrame;

public:
    PrivilegeErrorFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate (parent)
        , q_ptr(qobject_cast<PrivilegeErrorFrame* >(parent))
    {}

    void initUI();

    PrivilegeErrorFrame* q_ptr = nullptr;
};

void PrivilegeErrorFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 25, 25);
    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);

    return QWidget::paintEvent(event);
}

PrivilegeErrorFrame::PrivilegeErrorFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new PrivilegeErrorFramePrivate(this))
{
  setObjectName("privilege_error_frame");
  setFrameType(FrameType::ExtFrame);

  m_private->initUI();
}

PrivilegeErrorFrame::~PrivilegeErrorFrame()
{

}

void PrivilegeErrorFrame::init()
{

}

void PrivilegeErrorFrame::finished()
{

}

bool PrivilegeErrorFrame::shouldDisplay() const
{
    return !HasRootPrivilege();
}

QString PrivilegeErrorFrame::returnFrameName() const
{
    return "Privilege Error";
}

void PrivilegeErrorFramePrivate::initUI() {
  TitleLabel* title_label = new TitleLabel("Privilege Error");
  CommentLabel* comment_label = new CommentLabel(
      "Please execute with root account");

  centerLayout->setContentsMargins(0, 0, 0, 0);
  centerLayout->setSpacing(kMainLayoutSpacing);
  centerLayout->addStretch();
  centerLayout->addWidget(title_label, 0, Qt::AlignHCenter);
  centerLayout->addWidget(comment_label, 0, Qt::AlignHCenter);
  centerLayout->addStretch();

  centerLayout->setContentsMargins(0, 0, 0, 0);

  disconnect(nextButton, nullptr, nullptr, nullptr);
  connect(nextButton, &QPushButton::clicked, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->m_proxy->nextFrame();
  });
}

}  // namespace installer

#include "privilege_error_frame.moc"
