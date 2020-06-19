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

#include "ui/frames/inner/new_table_warning_frame.h"
#include "base/file_util.h"
#include "ui/delegates/partition_util.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"
#include "ui/utils/widget_util.h"

#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

namespace {
    const int kButtonWidth = 200;
    const int kButtonHeight = 36;

    const int kCommentLabelWidth = 398;
}

namespace installer {

NewTableWarningFrame::NewTableWarningFrame(QWidget* parent)
    : QFrame(parent),
      device_path_() {
  this->setObjectName("new_table_warning_frame");

  this->initUI();
  this->initConnections();
}

QString NewTableWarningFrame::devicePath() const {
  return device_path_;
}

void NewTableWarningFrame::setDevicePath(const QString& device_path,
                                         const QString& device_info) {
  device_path_ = device_path;
  disk_name_label_->setText(device_info);
}

void NewTableWarningFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(::QObject::tr("Format Warning"));
    comment_label_->setText(
        ::QObject::tr("Continuing installation will format the whole disk, "
           "please make a backup of all your data. If you do not know what you are doing, please do not continue"));
    cancel_button_->setText(::QObject::tr("Cancel"));
    confirm_button_->setText(::QObject::tr("Continue"));
  } else {
    QFrame::changeEvent(event);
  }
}

void NewTableWarningFrame::initConnections() {
  connect(cancel_button_, &QPushButton::clicked,
          this, &NewTableWarningFrame::canceled);
  connect(confirm_button_, &QPushButton::clicked,
          this, &NewTableWarningFrame::onConfirmButtonClicked);
}

void NewTableWarningFrame::initUI() {
  title_label_ = new TitleLabel(::QObject::tr("Format Warning"));
  title_label_->setObjectName("title_label");

  QLabel* disk_label = new QLabel();
  disk_label->setPixmap(QPixmap(GetOsTypeLargeIcon(OsType::Empty)));
  disk_name_label_ = new QLabel();
  disk_name_label_->setObjectName("disk_name_label");

  comment_label_ = new CommentLabel(
    ::QObject::tr("Continuing installation will format the whole disk, "
       "please make a backup of all your data. If you do not know what you are doing, please do not continue"));
  comment_label_->setFixedWidth(kCommentLabelWidth);
  comment_label_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  comment_label_->setWordWrap(true);
  comment_label_->setAlignment(Qt::AlignCenter);

  cancel_button_ = new QPushButton(::QObject::tr("Cancel"));
  cancel_button_->setFixedSize(kButtonWidth, kButtonHeight);
  confirm_button_ = new QPushButton(::QObject::tr("Continue"));
  confirm_button_->setFixedSize(kButtonWidth, kButtonHeight);

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(0);
  buttonLayout->addWidget(cancel_button_, 0, Qt::AlignHCenter | Qt::AlignLeft);
  buttonLayout->addSpacing(20);
  buttonLayout->addWidget(confirm_button_, 0, Qt::AlignHCenter | Qt::AlignRight);
  QWidget *buttonWrapWidget = new QWidget;
  buttonWrapWidget->setLayout(buttonLayout);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(kMainLayoutSpacing);
  layout->addSpacing(90);
  layout->addWidget(title_label_, 0, Qt::AlignHCenter);
  layout->addWidget(disk_label, 0, Qt::AlignHCenter);
  layout->addWidget(disk_name_label_, 0, Qt::AlignHCenter);
  layout->addWidget(comment_label_, 0, Qt::AlignHCenter);
  layout->addStretch();
  layout->addWidget(buttonWrapWidget, 0, Qt::AlignHCenter);

  this->setLayout(layout);
  this->setContentsMargins(0, 0, 0, 0);
}

void NewTableWarningFrame::onConfirmButtonClicked() {
  emit this->confirmed(device_path_);
}

}  // namespace installer
