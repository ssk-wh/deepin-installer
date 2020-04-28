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

#include "partition_table_warning_frame.h"

#include <QDebug>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <DSysInfo>

#include <DFrame>

#include "base/file_util.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/expanded_nav_button.h"
#include "ui/widgets/title_label.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/partition_table_widget.h"

#include <QDebug>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <DFrame>

DWIDGET_USE_NAMESPACE

namespace {
    const int kTitleCommentWidth = 384;
    const int kWarningLabelSize = 30;
}

namespace installer {

PartitionTableWarningFrame::PartitionTableWarningFrame(QWidget* parent)
    : QFrame(parent),
      device_path_() {
  this->setObjectName("partition_table_warning_frame");

  this->initUI();
  this->initConnections();
}

QString PartitionTableWarningFrame::devicePath() const {
  return device_path_;
}

void PartitionTableWarningFrame::setDevicePath(const QString& device_path) {
  device_path_ = device_path;
}

void PartitionTableWarningFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(tr("Warning"));
    comment_label_->setText(
        tr("You have an EFI boot loader but an MBR disk, thus you cannot install %1 directly. "
           "Please select one of the below solutions and continue.").arg(DSysInfo::productType() == DSysInfo::Deepin ? tr("Deepin") : tr("UOS")));
    list_title1_->setText(QString("A.%1").arg(tr("Disable UEFI")));
    list_item1_->setText(
        QString("1.%1\n2.%2")
            .arg(tr("Reboot, enter BIOS, and disable UEFI"))
            .arg(tr("Exit BIOS, and install %1 again").arg(DSysInfo::productType() == DSysInfo::Deepin ? tr("Deepin") : tr("UOS"))));
    list_title2_->setText(QString("B.%1").arg(tr("Format the disk")));
    list_item2_->setText(
        QString("1.%1\n2.%2")
            .arg(tr("Make a backup of all your data to avoid data loss"))
            .arg(tr("After the backup, reboot and enter this interface again")));
    list_title3_->setText(tr("C.%1").arg(tr("Continue")));
    list_item3_->setText(
        QString("1.%1\n2.%2")
            .arg(tr("Make sure you have backed up all data before proceeding"))
            .arg(tr("Continuing installation will format your disk")));
    reject_button_->setText(tr("Reboot"));
    accept_button_->setText(tr("Continue"));
    cancel_button_->setText(tr("Next"));
  } else {
    QFrame::changeEvent(event);
  }
}

void PartitionTableWarningFrame::initConnections() {
  connect(next_button_, &QPushButton::clicked,
          this, &PartitionTableWarningFrame::onNextButtonClicked);

  connect(m_buttonBox, &DButtonBox::buttonClicked, this
          , &PartitionTableWarningFrame::onButtonGroupToggled);
}

void PartitionTableWarningFrame::initUI() {
  QLabel* warning_label = new QLabel();
  warning_label->setFixedSize(QSize(kWarningLabelSize, kWarningLabelSize));
  QPixmap pixmap = installer::renderPixmap(":/images/warning.svg");
  pixmap = pixmap.scaled(warning_label->size(), Qt::KeepAspectRatio
                         , Qt::SmoothTransformation);
  warning_label->setPixmap(pixmap);

  title_label_ = new TitleLabel(tr("Warning"));
  title_label_->setContentsMargins(0, 0, 0, 0);
  title_label_->adjustSize();

  QHBoxLayout* title_layout = new QHBoxLayout();
  title_layout->setContentsMargins(0, 0, 0, 0);
  title_layout->setSpacing(0);
  title_layout->addStretch();
  title_layout->addWidget(warning_label);
  title_layout->addSpacing(8);
  title_layout->addWidget(title_label_);
  title_layout->addStretch();

  comment_label_ = new CommentLabel(
      tr("You have an EFI boot loader but an MBR disk, thus you cannot install %1 directly. "
         "Please select one of the below solutions and continue.").arg(DSysInfo::productType() == DSysInfo::Deepin ? tr("Deepin") : tr("UOS")));
  QHBoxLayout* comment_layout = new QHBoxLayout();
  comment_layout->setContentsMargins(0, 0, 0, 0);
  comment_layout->setSpacing(0);
  comment_layout->addWidget(comment_label_);

  QHBoxLayout* listTitleLayouA_ = new QHBoxLayout;
  list_title1_ = new QLabel(QString("A.%1").arg(tr("Disable UEFI")), this);
  list_title1_->setObjectName("list_title1");
  listTitleLayouA_->addSpacing(15);
  listTitleLayouA_->addWidget(list_title1_);

  list_item1_ = new QLabel(
      QString("1.%1\n2.%2")
          .arg(tr("Reboot, enter BIOS, and disable UEFI"))
          .arg(tr("Exit BIOS, and install %1 again").arg(DSysInfo::productType() == DSysInfo::Deepin ? tr("Deepin") : tr("UOS"))),
      this);
  list_item1_->setObjectName("list_item1");
  list_item1_->setWordWrap(true);

  QHBoxLayout* listTitleLayouB_ = new QHBoxLayout;
  list_title2_ = new QLabel(QString("B.%1").arg(tr("Format the disk")), this);
  list_title2_->setObjectName("list_title2");
  listTitleLayouB_->addSpacing(15);
  listTitleLayouB_->addWidget(list_title2_);
  list_item2_ = new QLabel(
      QString("1.%1\n2.%2")
          .arg(tr("Make a backup of all your data to avoid data loss"))
          .arg(tr("After the backup, reboot and enter this interface again")),
          this);
  list_item2_->setObjectName("list_item2");
  list_item2_->setWordWrap(true);
  QVBoxLayout* left_frame_layout = new QVBoxLayout();
  left_frame_layout->setContentsMargins(20, 20, 20, 20);
  left_frame_layout->setSpacing(0);
  left_frame_layout->addLayout(listTitleLayouA_);
  left_frame_layout->addWidget(list_item1_);
  left_frame_layout->addStretch();
  left_frame_layout->addSpacing(20);
  left_frame_layout->addLayout(listTitleLayouB_);
  left_frame_layout->addWidget(list_item2_);
  left_frame_layout->addStretch();
  DFrame* left_frame = new DFrame();
  left_frame->setObjectName("left_frame");
  left_frame->setLayout(left_frame_layout);

  QHBoxLayout* listTitleLayou3_ = new QHBoxLayout;
  list_title3_ = new QLabel(tr("Continue"), this);
  list_title3_->setObjectName("list_title3");
  listTitleLayou3_->addSpacing(15);
  listTitleLayou3_->addWidget(list_title3_);
  list_item3_ = new QLabel(
      QString("1.%1\n2.%2")
          .arg(tr("Please make sure all data were made a backup, "
                  "then continue"))
          .arg(tr("Continuing installation will format your disk")),
          this);
  list_item3_->setObjectName("list_item3");
  list_item3_->setWordWrap(true);
  QVBoxLayout* right_frame_layout = new QVBoxLayout();
  right_frame_layout->setContentsMargins(20, 20, 20, 20);
  right_frame_layout->setSpacing(0);
  right_frame_layout->addLayout(listTitleLayou3_);
  right_frame_layout->addWidget(list_item3_);
  right_frame_layout->addStretch();
  DFrame* right_frame = new DFrame();
  right_frame->setObjectName("right_frame");
  right_frame->setLayout(right_frame_layout);

  m_warningWidget3 = new PartitionTableWarningWidget;
  m_warningWidget3->setTitle(QString("%1").arg(tr("Cancel")));
  m_warningWidget3->setDesc(QString("1.%1")
                            .arg(tr("Nothing to do")));

  m_buttonBox = new DButtonBox(this);
  m_buttonBox->setButtonList({m_warningWidget1, m_warningWidget2, m_warningWidget3}, true);
  m_warningWidget1->setChecked(true);
  m_warningWidget1->updateCheckedAppearance();
  // default setting
  m_currentButton = m_warningWidget1;

  next_button_ = new QPushButton(tr("Next"));
  next_button_->setFixedSize(QSize(310, 36));

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addSpacing(kMainLayoutSpacing);
  layout->addLayout(title_layout);
  layout->addSpacing(kMainLayoutSpacing);
  layout->addWidget(comment_label_, 0, Qt::AlignHCenter);
  layout->addStretch();
  layout->addSpacing(kMainLayoutSpacing);
  layout->addWidget(m_warningWidget1, 0, Qt::AlignHCenter);
  layout->addSpacing(kMainLayoutSpacing);
  layout->addWidget(m_warningWidget2, 0, Qt::AlignHCenter);
  layout->addSpacing(kMainLayoutSpacing);
  layout->addWidget(m_warningWidget3, 0, Qt::AlignHCenter);
  layout->addSpacing(kMainLayoutSpacing);
  layout->addStretch();
  layout->addWidget(next_button_, 0, Qt::AlignHCenter);
  layout->addSpacing(10);

  setLayout(layout);
  setContentsMargins(0, 0, 0, 0);
}

void PartitionTableWarningFrame::onConfirmButtonClicked() {
    emit this->confirmed(device_path_);
}

void PartitionTableWarningFrame::onButtonGroupToggled(QAbstractButton *button)
{
    m_warningWidget1->updateCheckedAppearance();
    m_warningWidget2->updateCheckedAppearance();
    m_warningWidget3->updateCheckedAppearance();
    m_currentButton = button;
}

void PartitionTableWarningFrame::onNextButtonClicked()
{
    if (m_currentButton == m_warningWidget1) {
        emit reboot();
    }
    else if (m_currentButton == m_warningWidget2) {
        emit confirmed(device_path_);
    }
    else {
        emit canceled();
    }
}

}  // namespace installer
