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

#include "ui/frames/inner/select_bootloader_frame.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

#include "ui/frames/consts.h"
#include "ui/models/bootloader_list_model.h"
#include "ui/views/frameless_list_view.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"

namespace installer {

SelectBootloaderFrame::SelectBootloaderFrame(FrameProxyInterface *frameProxyInterface, QWidget* parent)
    : ChildFrameInterface(frameProxyInterface, parent)
{
  this->setObjectName("select_bootloader_frame");

  this->initUI();
  this->initConnections();
}

bool SelectBootloaderFrame::focusSwitch()
{
    if (list_view_->hasFocus()) {
        back_button_->setFocus();
    } else {
        list_view_->setFocus();
    }
    return true;
}

bool SelectBootloaderFrame::doSpace()
{
    return true;
}

bool SelectBootloaderFrame::doSelect()
{
    if (back_button_->hasFocus()) {
        emit back_button_->clicked();
    }
    return true;
}

bool SelectBootloaderFrame::directionKey(int keyvalue)
{
    switch (keyvalue) {
    case Qt::Key_Up: {
            if(list_view_->hasFocus()){
                QModelIndex testindex = list_view_->currentIndex();
                if ((testindex.row() - 1) >= 0) {
                    list_view_->setCurrentIndex(testindex.sibling(testindex.row() - 1, 0));
                }
            }
        }
        break;
    case Qt::Key_Down: {
            if(list_view_->hasFocus()){
                QModelIndex testindex = list_view_->currentIndex();
                if ((testindex.row() + 1) < list_view_->count()) {
                    list_view_->setCurrentIndex(testindex.sibling(testindex.row() + 1, 0));
                }
            }
        }
        break;
    case Qt::Key_Left:
        break;
    case Qt::Key_Right:
        break;
    }
    return true;
}

void SelectBootloaderFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(::QObject::tr("Select location for boot loader"));
    comment_label_->setText(
        ::QObject::tr("If you do not understand the settings, please select the recommended one"));
    back_button_->setText(::QObject::tr("Back"));
  } else {
    QWidget::changeEvent(event);
  }
}

void SelectBootloaderFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 25, 25);
    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);

    return QWidget::paintEvent(event);
}

void SelectBootloaderFrame::initConnections() {
  connect(back_button_, &QPushButton::clicked,
          this, &SelectBootloaderFrame::finished);
  connect(list_view_->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &SelectBootloaderFrame::onPartitionListViewSelected);
  connect(list_model_, &BootloaderListModel::rowChanged,
          this, &SelectBootloaderFrame::onModelChanged);

  connect(this, &SelectBootloaderFrame::deviceRefreshed,
          list_model_, &BootloaderListModel::onDeviceRefreshed);
}

void SelectBootloaderFrame::initUI() {
  title_label_ = new TitleLabel(::QObject::tr("Select location for boot loader"));
  comment_label_ = new CommentLabel(
      ::QObject::tr("If you do not understand the settings, please select the recommended one"));
  QHBoxLayout* comment_layout = new QHBoxLayout();
  comment_layout->setContentsMargins(0, 0, 0, 0);
  comment_layout->setSpacing(0);
  comment_layout->addWidget(comment_label_);

  list_view_ = new DListView;
  list_view_->setFixedWidth(560);
  list_model_ = new BootloaderListModel(this);
  list_view_->setModel(list_model_);

  back_button_ = new QPushButton(::QObject::tr("Back"));
  back_button_->setFixedSize(310, 36);

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(0);
  buttonLayout->addWidget(back_button_, 0, Qt::AlignHCenter | Qt::AlignLeft);
  QWidget *buttonWrapWidget = new QWidget;
  buttonWrapWidget->setContentsMargins(0, 0, 0, 0);
  buttonWrapWidget->setLayout(buttonLayout);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(kMainLayoutSpacing);
  layout->addStretch();
  layout->addWidget(title_label_, 0, Qt::AlignCenter);
  layout->addLayout(comment_layout);
  layout->addStretch();
  layout->addSpacing(40);
  layout->addWidget(list_view_, 0, Qt::AlignHCenter);
  layout->addStretch();
  layout->addSpacing(50);
  layout->addWidget(buttonWrapWidget, 0, Qt::AlignCenter);
  layout->addSpacing(10);

  this->setLayout(layout);
  this->setContentsMargins(0, 0, 0, 0);
}

void SelectBootloaderFrame::onModelChanged() {
  // Select recommended bootloader.
  const QModelIndex index = list_model_->getRecommendedIndex();
  if (index.isValid()) {
    list_view_->selectionModel()->select(index, QItemSelectionModel::Select);
    // Update selection model explicitly.
    emit list_view_->selectionModel()->currentChanged(index, QModelIndex());
  }
}

void SelectBootloaderFrame::onPartitionListViewSelected(
    const QModelIndex& current, const QModelIndex& previous) {
  Q_UNUSED(previous);
  if (current.isValid()) {
    const QString path = list_model_->getPath(current);
    if (!path.isEmpty()) {
      // Both AdvancedPartitionFrame and PartitionDelegate will
      // update advanced bootloader path
      emit this->bootloaderUpdated(path, false);
    }
  }
}

}  // namespace installer
