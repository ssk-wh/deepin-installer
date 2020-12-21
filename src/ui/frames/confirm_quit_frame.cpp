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

    initUI();
    initConnections();
}

void ConfirmQuitFrame::display()
{
    exec();
}

void ConfirmQuitFrame::updateTsForSuccessPage()
{
    setTitle(::QObject::tr("Shut Down"));
    comment_label_->setText(
        ::QObject::tr("You can experience it after configuring user information in next system startup."));
    continue_button_->setText(::QObject::tr("Cancel"));
    abort_button_->setText(::QObject::tr("Shut down"));
}

void ConfirmQuitFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    setTitle(::QObject::tr("Abort Installation"));
    comment_label_->setText(
        ::QObject::tr("Relevant operations you made in the installation process will not take effect, abort or continue installation?"));
    continue_button_->setText(::QObject::tr("Continue"));
    abort_button_->setText(::QObject::tr("Abort"));

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

void ConfirmQuitFrame::initConnections() {
  connect(continue_button_, &QPushButton::clicked,
          this, [=] {
      emit quitCancelled();
  });
  connect(abort_button_, &QPushButton::clicked,
          this, [=] {
      emit quitConfirmed();
  });

  connect(m_close_button, &DImageButton::clicked,
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

  setupCloseButton();
}

void ConfirmQuitFrame::setupCloseButton()
{
    // TODO: use titleBar implement.
    m_close_button = new DImageButton(this);
    //m_close_button->setFocusPolicy(Qt::TabFocus);
    m_close_button->setFixedSize(40, 40);
    m_close_button->setNormalPic(":/images/close_normal.svg");
    m_close_button->setHoverPic(":/images/close_normal.svg");
    m_close_button->setPressPic(":/images/close_normal.svg");
}

bool ConfirmQuitFrame::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::MouseButton::RightButton) {
            return true;
        }
    }

    return DDialog::eventFilter(watched, event);
}

void ConfirmQuitFrame::showEvent(QShowEvent *event)
{
    qApp->installEventFilter(this);
    return DDialog::showEvent(event);
}

void ConfirmQuitFrame::hideEvent(QHideEvent *event)
{
    qApp->removeEventFilter(this);
    return DDialog::hideEvent(event);
}

void ConfirmQuitFrame::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton)
    {
        /*移动中的鼠标位置相对于初始位置的相对位置.*/
        QPoint relativePos = this->pos();
        /*然后移动窗体即可.*/
        this->move(relativePos);
    }
}

}  // namespace installer
