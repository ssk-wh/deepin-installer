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

#include "ui/frames/swap_warnning_frame.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/select_button.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <DDialog>

DWIDGET_USE_NAMESPACE

namespace {
    const int kWarnningDialogWidth = 400;
    const int kWarnningDialogHeight = 220;
}

namespace installer {

SwapWarnningFrame::SwapWarnningFrame(QWidget* parent)
    : DDialog(parent)
{
    setObjectName("swap_warnning_frame");

    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    initUI();
    initConnections();
}

void SwapWarnningFrame::display()
{
    exec();
}

void SwapWarnningFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    setTitle(::QObject::tr("Friendly Note"));
    comment_label_->setText(
        ::QObject::tr("No swap partition created, which may affect system performance"));
    continue_button_->setText(::QObject::tr("OK"));

    if (m_close_button) {
        const int marginSize = this->layout()->margin();
        m_close_button->move(width() - m_close_button->width() - marginSize, marginSize);
        m_close_button->raise();
        m_close_button->show();
    }
  } else {
    QWidget::changeEvent(event);
  }
}

void SwapWarnningFrame::initConnections() {
  connect(continue_button_, &QPushButton::clicked,
          this, &SwapWarnningFrame::quitCancelled);
  connect(m_close_button, &DImageButton::clicked,
          this, &SwapWarnningFrame::quitCancelled);
}

void SwapWarnningFrame::initUI() {
  comment_label_ = new CommentLabel(
              ::QObject::tr("No swap partition created, which may affect system performance"));
  comment_label_->setFixedWidth(380);
  comment_label_->setWordWrap(true);
  comment_label_->setAlignment(Qt::AlignCenter);

  continue_button_ = new SelectButton();
  continue_button_->setText(::QObject::tr("OK"));
  continue_button_->setFixedSize(170, 36);
  continue_button_->setFocusPolicy(Qt::TabFocus);

  QPixmap pixmap = installer::renderPixmap(":/images/interaction_warning.svg");
  const auto ratio = devicePixelRatioF();
  pixmap = pixmap.scaled(48 * ratio, 48 * ratio, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  setIconPixmap(pixmap);

  setTitle(::QObject::tr("Friendly Note"));

  addContent(comment_label_, Qt::AlignTop | Qt::AlignHCenter);

  int index = 1;
  insertButton(index, continue_button_, true);

  setContentsMargins(0, 0, 0, 0);
  setFixedSize(kWarnningDialogWidth, kWarnningDialogHeight);

  setupCloseButton();
}

void SwapWarnningFrame::setupCloseButton()
{
    // TODO: use titleBar implement.
    m_close_button = new DImageButton(this);
    m_close_button->setFocusPolicy(Qt::TabFocus);
    m_close_button->setFixedSize(40, 40);
    m_close_button->setNormalPic(":/images/close_normal.svg");
    m_close_button->setHoverPic(":/images/close_normal.svg");
    m_close_button->setPressPic(":/images/close_normal.svg");
}

}  // namespace installer
