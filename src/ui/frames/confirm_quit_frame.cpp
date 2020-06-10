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
#include "ui/utils/widget_util.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <DDialog>

DWIDGET_USE_NAMESPACE

namespace {
    const int kCloseDialogWidth = 400;
    const int kCloseDialogHeight = 220;
}

namespace installer {

ConfirmQuitFrame::ConfirmQuitFrame(QWidget* parent)
    : DDialog(parent)
{
    setObjectName("confirm_quit_frame");

    initUI();
    initConnections();
}

void ConfirmQuitFrame::display()
{
    exec();
}

void ConfirmQuitFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    setTitle(::QObject::tr("Abort Installation"));
    comment_label_->setText(
        ::QObject::tr("Relevant operations you made in the installation process will not take effect, abort or continue installation?"));
    continue_button_->setText(::QObject::tr("Continue"));
    abort_button_->setText(::QObject::tr("Abort"));
  } else {
    QWidget::changeEvent(event);
  }
}

void ConfirmQuitFrame::initConnections() {
  connect(continue_button_, &QPushButton::clicked,
          this, &ConfirmQuitFrame::quitCancelled);
  connect(abort_button_, &QPushButton::clicked,
          this, &ConfirmQuitFrame::quitConfirmed);
}

void ConfirmQuitFrame::initUI() {
  comment_label_ = new CommentLabel(
              ::QObject::tr("Relevant operations you made in the installation process will not take effect, abort or continue installation?"));
  comment_label_->setFixedWidth(380);
  comment_label_->setWordWrap(true);
  comment_label_->setAlignment(Qt::AlignCenter);

  continue_button_ = new QPushButton(::QObject::tr("Continue"));
  continue_button_->setFixedSize(170, 36);
  abort_button_ = new QPushButton(::QObject::tr("Abort"));
  abort_button_->setFixedSize(170, 36);

  QPixmap pixmap = installer::renderPixmap(":/images/interaction_warning.svg");
  const auto ratio = devicePixelRatioF();
  pixmap = pixmap.scaled(48 * ratio, 48 * ratio, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  setIconPixmap(pixmap);

  setTitle(::QObject::tr("Abort Installation"));

  addContent(comment_label_, Qt::AlignTop | Qt::AlignHCenter);

  int index = 1;
  insertButton(index, continue_button_, true);
  ++index;
  insertButton(index, abort_button_);

  setContentsMargins(0, 0, 0, 0);
  setFixedSize(kCloseDialogWidth, kCloseDialogHeight);
}

}  // namespace installer
