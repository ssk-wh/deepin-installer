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

#include "ui/frames/confirm_quit_frame.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>

namespace installer {

ConfirmQuitFrame::ConfirmQuitFrame(FrameProxyInterface *frameProxyInterface, QWidget* parent)
    : BaseFrameInterface (FrameType::ExtFrame, frameProxyInterface, parent)
{
  this->setObjectName("confirm_quit_frame");

  this->initUI();
    this->initConnections();
}

void ConfirmQuitFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(tr("Abort Installation"));
    comment_label_->setText(
        tr("Relevant operations you made in the installation "
           "process will not take effect, abort or continue installation?"));
    continue_button_->setText(tr("Continue"));
    abort_button_->setText(tr("Abort"));
  } else {
    QWidget::changeEvent(event);
  }
}

void ConfirmQuitFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 25, 25);
    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);

    return QWidget::paintEvent(event);
}

void ConfirmQuitFrame::initConnections() {
  connect(continue_button_, &QPushButton::clicked,
          this, &ConfirmQuitFrame::quitCancelled);
  connect(abort_button_, &QPushButton::clicked,
          this, &ConfirmQuitFrame::quitConfirmed);
}

void ConfirmQuitFrame::initUI() {
  title_label_ = new TitleLabel(tr("Abort Installation"));
  comment_label_ = new CommentLabel(
      tr("Relevant operations you made in the installation "
         "process will not take effect, abort or continue installation?"));
  QHBoxLayout* comment_layout = new QHBoxLayout();
  comment_layout->setContentsMargins(0, 0, 0, 0);
  comment_layout->setSpacing(0);
  comment_layout->addWidget(comment_label_);

  continue_button_ = new QPushButton(tr("Continue"));
  continue_button_->setFixedSize(310, 36);
  abort_button_ = new QPushButton(tr("Abort"));
  abort_button_->setFixedSize(310, 36);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(kMainLayoutSpacing);
  layout->addStretch();
  layout->addWidget(title_label_, 0, Qt::AlignCenter);
  layout->addLayout(comment_layout);
  layout->addStretch();
  layout->addWidget(continue_button_, 0, Qt::AlignCenter);
  layout->addSpacing(kNavButtonVerticalSpacing);
  layout->addWidget(abort_button_, 0, Qt::AlignCenter);

  this->setLayout(layout);
  this->setContentsMargins(0, 0, 0, 0);
  setFixedSize(1000, 800);
}

}  // namespace installer
