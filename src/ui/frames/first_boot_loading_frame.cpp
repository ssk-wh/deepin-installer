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

#include "ui/frames/first_boot_loading_frame.h"

#include <QEvent>
#include <QVBoxLayout>

#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/spinner_label.h"
#include "ui/widgets/title_label.h"

namespace installer {

FirstBootLoadingFrame::FirstBootLoadingFrame(FrameProxyInterface *frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent) {
  this->setObjectName("first_boot_loading_frame");

    this->initUI();
}

void FirstBootLoadingFrame::init()
{
    emit startRunHooks();
}

void FirstBootLoadingFrame::finished()
{

}

bool FirstBootLoadingFrame::shouldDisplay() const
{
    return true;
}

QString FirstBootLoadingFrame::returnFrameName() const
{
    return ::QObject::tr("Tuning system");
}

bool FirstBootLoadingFrame::allowPrevious() const
{
    return false;
}

void FirstBootLoadingFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(::QObject::tr("Tuning system"));
    comment_label_->setText(
        ::QObject::tr("Applying changes to your system, please wait..."));
  } else {
    QWidget::changeEvent(event);
  }
}

void FirstBootLoadingFrame::hideEvent(QHideEvent* event) {
  spinner_label_->stop();
  QWidget::hideEvent(event);
}

void FirstBootLoadingFrame::showEvent(QShowEvent* event) {
  spinner_label_->start();
  Q_EMIT closeButtionChange(false);
  QWidget::showEvent(event);
}

void FirstBootLoadingFrame::initConnections() {

}

void FirstBootLoadingFrame::initUI() {
  title_label_ = new TitleLabel(::QObject::tr("Tuning system"));
  comment_label_ = new CommentLabel(
      ::QObject::tr("Applying changes to your system, please wait..."));
  spinner_label_ = new SpinnerLabel();

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(kMainLayoutSpacing);
  layout->addStretch();
  layout->addWidget(title_label_, 0, Qt::AlignCenter);
  layout->addWidget(comment_label_, 0, Qt::AlignCenter);
  layout->addSpacing(15);
  layout->addWidget(spinner_label_, 0, Qt::AlignCenter);
  layout->addStretch();

  this->setLayout(layout);
  this->setContentsMargins(0, 0, 0, 0);
}

}  // namespace installer
