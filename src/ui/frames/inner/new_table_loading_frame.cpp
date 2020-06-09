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

#include "ui/frames/inner/new_table_loading_frame.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"

#include <QEvent>
#include <QVBoxLayout>

namespace installer {

NewTableLoadingFrame::NewTableLoadingFrame(QWidget* parent) : QFrame(parent) {
  this->setObjectName("new_table_loading_frame");

  this->initUI();
}

void NewTableLoadingFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    comment_label_->setText(::QObject::tr("Formatting..."));
  } else {
    QFrame::changeEvent(event);
  }
}

void NewTableLoadingFrame::hideEvent(QHideEvent* event) {
    m_waterProgress->stop();
    QFrame::hideEvent(event);
}

void NewTableLoadingFrame::showEvent(QShowEvent* event) {
    m_waterProgress->start();
    QFrame::showEvent(event);
}

void NewTableLoadingFrame::initUI() {
  comment_label_ = new CommentLabel(::QObject::tr("Formatting..."));
  comment_label_->setAlignment(Qt::AlignHCenter);

  m_waterProgress = new Dtk::Widget::DWaterProgress;
  m_waterProgress->setTextVisible(false);
  m_waterProgress->setFixedSize(100, 100);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(kMainLayoutSpacing);
  layout->addStretch();
  layout->addWidget(m_waterProgress, 0, Qt::AlignHCenter);
  layout->addSpacing(15);
  layout->addWidget(comment_label_, 0, Qt::AlignHCenter);
  layout->addStretch();

  this->setLayout(layout);
  this->setContentsMargins(0, 0, 0, 0);
}

}  // namespace installer
