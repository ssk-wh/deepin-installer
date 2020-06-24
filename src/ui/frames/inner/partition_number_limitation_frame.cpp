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

#include "ui/frames/inner/partition_number_limitation_frame.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"
#include "ui/utils/widget_util.h"

#include <QEvent>
#include <QVBoxLayout>

namespace installer {

namespace {
    const int kHintLabelWidth = 390;
    const int kTitleFontSize = 24; // 24pt
    const int kWarningLabelSize = 30;
}

PartitionNumberLimitationFrame::PartitionNumberLimitationFrame(
    QWidget* parent) : QFrame(parent) {
  this->setObjectName("partition_number_limitation_frame");

  this->initUI();
  this->initConnections();
}


void PartitionNumberLimitationFrame::showEvent(QShowEvent* event) {
    title_label_->setText(::QObject::tr("Failed to Create New Partition"));
    comment1_label_->setText(
        ::QObject::tr("You should delete a primary partition before creating a new one, as there can only be four primary partitions on an MBR disk"));
    comment2_label_->setText(
        ::QObject::tr("You should delete a primary partition before creating a logical one, or move the existing logical partition to the end of the disk"));
    back_button_->setText(::QObject::tr("Back"));

  return QFrame::showEvent(event);
}

void PartitionNumberLimitationFrame::initConnections() {
  connect(back_button_, &QPushButton::clicked,
          this, &PartitionNumberLimitationFrame::finished);
}

void PartitionNumberLimitationFrame::initUI() {
    QLabel* warning_label = new QLabel();
    warning_label->setFixedSize(QSize(kWarningLabelSize, kWarningLabelSize));
    QPixmap pixmap = installer::renderPixmap(":/images/warning.svg");
    pixmap = pixmap.scaled(warning_label->size(), Qt::KeepAspectRatio
                           , Qt::SmoothTransformation);
    warning_label->setPixmap(pixmap);

  title_label_ = new QLabel(::QObject::tr("Failed to Create New Partition"));
  QFont font;
  font.setPointSize(kTitleFontSize);
  title_label_->setFont(font);
  title_label_->setContentsMargins(0, 0, 0, 0);
  title_label_->adjustSize();

  QHBoxLayout* title_layout = new QHBoxLayout();
  title_layout->setContentsMargins(0, 0, 0, 0);
  title_layout->setSpacing(0);
  title_layout->addStretch();
  title_layout->addWidget(warning_label, 0, Qt::AlignVCenter);
  title_layout->addSpacing(8);
  title_layout->addWidget(title_label_, 0, Qt::AlignVCenter);
  title_layout->addStretch();

  comment1_label_ = new CommentLabel(this);
  comment1_label_->setFixedWidth(kHintLabelWidth);
  comment1_label_->setWordWrap(true);

  comment2_label_ = new CommentLabel(this);
  comment2_label_->setFixedWidth(kHintLabelWidth);
  comment2_label_->setWordWrap(true);

  back_button_ = new QPushButton(::QObject::tr("Back"));
  back_button_->setFixedSize(310, 36);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(0);
  layout->setSpacing(0);
  layout->addStretch();
  layout->addLayout(title_layout);
  layout->addSpacing(40);
  layout->addWidget(comment1_label_, 0, Qt::AlignHCenter);
  layout->addSpacing(20);
  layout->addWidget(comment2_label_, 0, Qt::AlignHCenter);
  layout->addStretch();
  layout->addWidget(back_button_, 0, Qt::AlignHCenter);

  QHBoxLayout* mainLayout = new QHBoxLayout;
  mainLayout->setMargin(0);
  mainLayout->addStretch();
  mainLayout->addLayout(layout);
  mainLayout->addStretch();

  setLayout(mainLayout);
}

}  // namespace installer
