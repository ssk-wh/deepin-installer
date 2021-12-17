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

#include "ui/widgets/simple_partition_button.h"

#include <QLabel>
#include <QVBoxLayout>

#include "base/file_util.h"
#include "ui/delegates/partition_util.h"
#include "ui/widgets/rounded_progress_bar.h"
#include "ui/utils/widget_util.h"

namespace installer {

namespace {

const int kButtonWidth = 120 ;
const int kButtonHeight = 120 ;
const int kOsIconWidth = 48;
const int kOsIconHeight = 48;

}  // namespace

SimplePartitionButton::SimplePartitionButton(const Partition::Ptr partition,
                                             QWidget* parent)
    : PointerButton(parent),
      partition_(partition),
      selected_(false) {
  this->setObjectName("simple_partition_button");
  this->setFixedSize(kButtonWidth, kButtonHeight);
  this->setCheckable(true);
  this->initUI();
}

void SimplePartitionButton::setSelected(bool selected) {
  selected_ = selected;
  if (selected) {
    QPixmap pixmap = installer::renderPixmap(GetPartitionIcon128());
    pixmap = pixmap.scaled(os_label_->size(), Qt::KeepAspectRatio
                           , Qt::SmoothTransformation);
    os_label_->setPixmap(pixmap);
  } else {
    QPixmap pixmap(GetPartitionIcon128());
    pixmap = pixmap.scaled(os_label_->size(), Qt::KeepAspectRatio
                           , Qt::SmoothTransformation);
    os_label_->setPixmap(pixmap);
  }
}

void SimplePartitionButton::initUI() {
  os_label_ = new QLabel();
  os_label_->setObjectName("fs_label");
  os_label_->setFixedSize(kOsIconWidth, kOsIconHeight);
  QPixmap pixmap(GetPartitionIcon128());
  pixmap = pixmap.scaled(os_label_->size(), Qt::KeepAspectRatio
                         , Qt::SmoothTransformation);
  os_label_->setPixmap(pixmap);

  QLabel* path_label = new QLabel(GetPartitionLabelAndPath(partition_));
  path_label->setObjectName("path_label");
  path_label->setFixedHeight(20);

  QLabel* usage_label = new QLabel();
  usage_label->setFixedHeight(16);
  usage_label->setText(GetPartitionUsage(partition_));
  usage_label->setObjectName("usage_label");

  QProgressBar* usage_bar = new RoundedProgressBar();
  usage_bar->setFixedSize(80, 6);
  usage_bar->setValue(GetPartitionUsageValue(partition_));

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addStretch();
  layout->addWidget(os_label_, 0, Qt::AlignHCenter);
  layout->addSpacing(6);
  layout->addWidget(path_label, 0, Qt::AlignHCenter);
  layout->addWidget(usage_label, 0, Qt::AlignHCenter);
  layout->addSpacing(6);
  layout->addWidget(usage_bar, 0, Qt::AlignHCenter);
  layout->addStretch();

  this->setLayout(layout);

  this->setCheckable(true);
  this->setFixedSize(kButtonWidth, kButtonHeight);
}

}  // namespace installer
