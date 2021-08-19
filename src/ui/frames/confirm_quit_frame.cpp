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
#include "ui/widgets/select_button.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <DDialog>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>

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

    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
#ifndef QT_DEBUG
    setWindowFlags(windowFlags() & ~Qt::Dialog);    // 配合主窗口中设置Qt::X11BypassWindowManagerHint 时使用，在本地debug模式下不设置
#endif

    initUI();
    initConnections();
}

void ConfirmQuitFrame::display()
{
    setModal(true);
    show();
}

void ConfirmQuitFrame::updateTs()
{
    setTitle(::QObject::tr("Abort Installation"));
    comment_label_->setText(
        ::QObject::tr("Relevant operations you made in the installation process will not take effect, abort or continue installation?"));
    continue_button_->setText(::QObject::tr("Continue"));
    abort_button_->setText(::QObject::tr("Abort"));
}

void ConfirmQuitFrame::updateTsForSuccessPage()
{
    setTitle(::QObject::tr("Shut Down"));
    comment_label_->setText(
        ::QObject::tr("You can experience it after configuring user information in next system startup."));
    continue_button_->setText(::QObject::tr("Cancel", "button"));
    abort_button_->setText(::QObject::tr("Shut down", "button"));
}

void ConfirmQuitFrame::setLogInfo(const QString &log)
{
    // 清理弹窗
    setTitle(::QObject::tr(""));
    clearButtons();
    abort_button_->hide();

    // 重新设置弹窗布局和文案
    insertButton(1, continue_button_, true);
    comment_label_->setText(log);
    continue_button_->setText(tr("Continue"));
}

void ConfirmQuitFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    updateTs();
  } else {
    QWidget::changeEvent(event);
  }
}

void ConfirmQuitFrame::initConnections() {
  connect(continue_button_, &QPushButton::clicked,
          this, [=] {
      emit quitCancelled();
  });
  connect(abort_button_, &QPushButton::clicked,
          this, [=] {
      emit quitConfirmed();
  });

  connect(m_close_button, &DIconButton::clicked,
          this, [=] {
      emit quitCancelled();
  });
}

void ConfirmQuitFrame::initUI() {
  comment_label_ = new CommentLabel(
              ::QObject::tr("Relevant operations you made in the installation process will not take effect, abort or continue installation?"));
  comment_label_->setFixedWidth(380);
  comment_label_->setWordWrap(true);
  comment_label_->setAlignment(Qt::AlignCenter);

  continue_button_ = new SelectButton();
  continue_button_->setFixedSize(170, 36);
  //continue_button_->setFocusPolicy(Qt::TabFocus);

  abort_button_ = new DSuggestButton();
  abort_button_->setFixedSize(170, 36);
  //abort_button_->setFocusPolicy(Qt::TabFocus);

  QPixmap pixmap = installer::renderPixmap(":/images/interaction_warning.svg");
  const auto ratio = devicePixelRatioF();
  pixmap = pixmap.scaled(static_cast<int>(48 * ratio), static_cast<int>(48 * ratio), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  setIcon(QIcon(pixmap));

  setTitle(::QObject::tr("Abort Installation"));

  addContent(comment_label_, Qt::AlignTop | Qt::AlignHCenter);

  insertButton(1, continue_button_, true);
  insertButton(2, abort_button_);

  setContentsMargins(0, 0, 0, 0);
  setFixedSize(kCloseDialogWidth, kCloseDialogHeight);

  setupCloseButton();
}

void ConfirmQuitFrame::setupCloseButton()
{
    m_close_button = new DIconButton(this);
    //m_close_button->setFocusPolicy(Qt::TabFocus);
    m_close_button->setFixedSize(40, 40);
    m_close_button->setIconSize(QSize(40, 40));
    m_close_button->setIcon(QIcon(":/images/close_normal.svg"));
    m_close_button->setFlat(true);

    m_close_button->hide();
}

void ConfirmQuitFrame::resizeEvent(QResizeEvent *event)
{
    if (m_close_button != nullptr) {
        const int marginSize = this->layout()->margin();
        m_close_button->move(width() - m_close_button->width() - marginSize, marginSize);
        m_close_button->raise();
        m_close_button->show();
    }

    DDialog::resizeEvent(event);
}

}  // namespace installer
