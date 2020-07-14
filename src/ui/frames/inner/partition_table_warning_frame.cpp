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
#include "ui/widgets/operator_widget.h"
#include "ui/delegates/license_delegate.h"

#include <QDebug>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <DFrame>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

namespace {
    const int kTitleCommentWidth = 384;
    const int kWarningLabelSize = 30;
    const int kWarnItemWidth = 558;
    const int kWarnItemHeight = 100;
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
    title_label_->setText(::QObject::tr("Warning"));
    comment_label_->setText(
        ::QObject::tr("You have an EFI boot loader but an MBR disk, thus you cannot install %1 directly. "
           "Please select one of the below solutions and continue.").arg(DSysInfo::productType() == DSysInfo::Deepin ? ::QObject::tr("Deepin") : LicenseDelegate::product()));
    m_warningWidget1->setTitle(QString("%1").arg(::QObject::tr("Disable UEFI")));
    m_warningWidget1->setBody(QString("1.%1\n2.%2")
                              .arg(::QObject::tr("Reboot, enter BIOS, and disable UEFI"))
                              .arg(::QObject::tr("Exit BIOS, and install UOS again")));
    m_warningWidget2->setTitle(QString("%1").arg(::QObject::tr("Continue")));
    m_warningWidget2->setBody(QString("1.%1\n2.%2")
                              .arg(::QObject::tr("Make sure you have backed up all data before proceeding"))
                              .arg(::QObject::tr("Continuing installation will format your disk")));
    m_warningWidget3->setTitle(QString("%1").arg(::QObject::tr("Cancel")));
//    m_warningWidget3->setBody(QString("1.%1")
//                              .arg(::QObject::tr("Nothing to do")));

    next_button_->setText(::QObject::tr("Next"));
    m_buttonBox->hide();
  } else {
    QFrame::changeEvent(event);
  }
}

void PartitionTableWarningFrame::showEvent(QShowEvent *event)
{
    if (m_currentButton != nullptr) {
        static_cast<OperatorWidget *>(m_currentButton)->setSelect(false);
        m_currentButton = nullptr;
    }

    next_button_->setEnabled(false);

    QFrame::showEvent(event);
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

  title_label_ = new TitleLabel(::QObject::tr("Warning"));
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
      ::QObject::tr("You have an EFI boot loader but an MBR disk, thus you cannot install UOS directly. "
         "Please select one of the below solutions and continue."));
  comment_label_->setFixedWidth(kTitleCommentWidth);
  comment_label_->setWordWrap(true);
  comment_label_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  m_warningWidget1 = new OperatorWidget;
  m_warningWidget1->setFixedSize(kWarnItemWidth, kWarnItemHeight);
  m_warningWidget1->setSelectIcon(":/images/select_blue.svg");
  m_warningWidget1->setTitle(QString("%1").arg(::QObject::tr("Disable UEFI")));
  m_warningWidget1->setBody(QString("1.%1\n2.%2")
                            .arg(::QObject::tr("Reboot, enter BIOS, and disable UEFI"))
                            .arg(::QObject::tr("Exit BIOS, and install UOS again")));

  m_warningWidget2 = new OperatorWidget;
  m_warningWidget2->setFixedSize(kWarnItemWidth, kWarnItemHeight);
  m_warningWidget2->setSelectIcon(":/images/select_blue.svg");
  m_warningWidget2->setTitle(QString("%1").arg(::QObject::tr("Continue")));
  m_warningWidget2->setBody(QString("1.%1\n2.%2")
                            .arg(::QObject::tr("Make sure you have backed up all data before proceeding"))
                            .arg(::QObject::tr("Continuing installation will format your disk")));

  m_warningWidget3 = new OperatorWidget;
  m_warningWidget3->setFixedSize(kWarnItemWidth, kWarnItemHeight);
  m_warningWidget3->setSelectIcon(":/images/select_blue.svg");
  m_warningWidget3->setTitle(QString("%1").arg(::QObject::tr("Cancel")));
//  m_warningWidget3->setBody(QString("1.%1")
//                            .arg(::QObject::tr("Nothing to do")));

  m_buttonBox = new DButtonBox(this);
  m_buttonBox->setButtonList({m_warningWidget1, m_warningWidget2, m_warningWidget3}, true);

  // default setting is null.
  m_currentButton = nullptr;

  next_button_ = new QPushButton(::QObject::tr("Next"));
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
    m_currentButton = button;
    if (m_currentButton == m_warningWidget1) {
        m_warningWidget2->setSelect(false);
        m_warningWidget3->setSelect(false);
    }
    else if (m_currentButton == m_warningWidget2) {
        m_warningWidget1->setSelect(false);
        m_warningWidget3->setSelect(false);
    }
    else {
        m_warningWidget1->setSelect(false);
        m_warningWidget2->setSelect(false);
    }

    next_button_->setEnabled(m_currentButton != nullptr);
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
