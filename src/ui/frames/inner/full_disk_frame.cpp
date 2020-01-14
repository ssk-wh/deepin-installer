﻿/*
 * Copyright (C) 2018 Deepin Technology Co., Ltd.
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

#include "ui/frames/inner/full_disk_frame.h"

#include <QButtonGroup>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QCheckBox>
#include <QApplication>
#include <QStackedLayout>
#include <DSysInfo>

#include "base/file_util.h"
#include "partman/device.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/full_disk_delegate.h"
#include "ui/delegates/partition_util.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/simple_disk_button.h"
#include "ui/widgets/full_disk_partition_colorbar.h"
#include "ui/widgets/multiple_disk_installation_widget.h"

DCORE_USE_NAMESPACE

namespace installer {

namespace {

// 4 partitions are displays at each row.
const int kDiskColumns = 1;

const int kWindowWidth = 800;

enum class DiskCountType : int
{
      SingleDisk = 0
    , MultipleDisk
};

}  // namespace

#if 0
QT_TRANSLATE_NOOP("FullDiskFrame", "Install here")
QT_TRANSLATE_NOOP("FullDiskFrame", "Encrypt this disk")
QT_TRANSLATE_NOOP("FullDiskFrame", "Encrypt This Disk")
QT_TRANSLATE_NOOP("FullDiskFrame", "Please select a disk to start installation")
QT_TRANSLATE_NOOP("FullDiskFrame", "You need at least %1 GB disk space to install %2. To get better performance, %3 GB or more is recommended")
#endif

FullDiskFrame::FullDiskFrame(FullDiskDelegate* delegate, QWidget* parent)
    : QFrame(parent)
    , m_delegate(delegate)
{
  this->setObjectName("simple_disk_frame");

  this->initUI();
  this->initConnections();

  m_delegate->saveSwapSize();
}

FullDiskFrame::~FullDiskFrame() {

}

bool FullDiskFrame::validate() const {
    m_errorTip->hide();
    m_diskTooSmallTip->hide();

    if (m_delegate->selectedDisks().isEmpty()) {
        m_errorTip->show();
        return false;
    }

    int index = DeviceIndex(m_delegate->virtualDevices(), m_delegate->selectedDisks()[0]);
    if (index < 0) {
        qWarning() << QString("MULTIDISK:DeviceIndex failed:{%1}").arg(m_delegate->selectedDisks()[0]);
        return false;
    }

    Device::Ptr device(new Device(*m_delegate->virtualDevices()[index]));
    if (m_delegate->selectedDisks().count() < 2) {
        const qint64 root_required = GetSettingsInt(kPartitionFullDiskMiniSpace);
        const qint64 root_required_bytes = kGibiByte * root_required;
        if (device->getByteLength() < root_required_bytes) {
            m_diskTooSmallTip->show();
            qWarning() << QString("MULTIDISK: disk too small:size:{%1}.").arg(device->getByteLength());
            return false;
        }
    }

    if (!m_delegate->formatWholeDeviceMultipleDisk()) {
        m_diskTooSmallTip->show();
        qWarning() << "MULTIDISK: Failed to formatWholeDeviceMultipleDisk.";
        return false;
    }

    return true;
}

bool FullDiskFrame::isEncrypt() const
{
    return m_encryptCheck->isChecked();
}

bool FullDiskFrame::isInstallNvidia() const
{
    return m_installNvidiaCheck->isChecked();
}

void FullDiskFrame::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        for (auto it = m_trList.begin(); it != m_trList.end(); ++it) {
            it->first(qApp->translate("installer::FullDiskFrame", it->second.toUtf8()));
        }
        m_installNvidiaCheck->setText(tr("Install NVIDIA closed source driver"));
    }
    else {
        QFrame::changeEvent(event);
    }
}

void FullDiskFrame::initConnections() {
  // Repaint layout only when real device list is updated.
  connect(m_delegate, &FullDiskDelegate::deviceRefreshed,
          this, &FullDiskFrame::onDeviceRefreshed);
  connect(m_button_group,
          static_cast<void(QButtonGroup::*)(QAbstractButton*, bool)>
          (&QButtonGroup::buttonToggled),
          this, &FullDiskFrame::onPartitionButtonToggled);
  connect(m_encryptCheck, &QCheckBox::clicked, this, &FullDiskFrame::cryptoStateChanged);
  connect(m_installNvidiaCheck, &QCheckBox::clicked, this, &FullDiskFrame::installNvidiaStateChanged);
  connect(m_delegate, &FullDiskDelegate::deviceRefreshed,
          m_diskInstallationWidget, &MultipleDiskInstallationWidget::onDeviceListChanged);
  connect(m_diskInstallationWidget, &MultipleDiskInstallationWidget::currentDeviceChanged,
          this, &FullDiskFrame::onCurrentDeviceChanged);
}

void FullDiskFrame::initUI() {
  m_button_group = new QButtonGroup(this);

  QLabel* tip_icon = new QLabel();
  tip_icon->setPixmap(installer::renderPixmap(":/images/install_icon.svg"));
  m_tip_label = new QLabel(tr("Install here"));
  m_tip_label->setObjectName("tip_label");
  m_tip_label->setFixedHeight(18);
  addTransLate(m_trList, std::bind(&QLabel::setText, m_tip_label, std::placeholders::_1), QString("Install here"));

  m_encryptCheck = new QCheckBox;
  m_encryptCheck->setObjectName("check_box");
  m_encryptCheck->setCheckable(true);
  m_encryptCheck->setChecked(false);
  m_encryptCheck->setFocusPolicy(Qt::NoFocus);
  addTransLate(m_trList, std::bind(&QCheckBox::setText, m_encryptCheck, std::placeholders::_1), QString("Encrypt this disk"));

  m_installNvidiaCheck = new QCheckBox;
  m_installNvidiaCheck->setObjectName("check_box");
  m_installNvidiaCheck->setCheckable(true);
  m_installNvidiaCheck->setChecked(false);
  m_installNvidiaCheck->setFocusPolicy(Qt::NoFocus);
  m_installNvidiaCheck->setText(tr("Install NVIDIA closed source driver"));

  m_errorTip = new QLabel;
  m_errorTip->setObjectName("msg_label");
  m_errorTip->hide();
  addTransLate(m_trList, std::bind(&QLabel::setText, m_errorTip, std::placeholders::_1), QString("Please select a disk to start installation"));

  m_diskTooSmallTip = new QLabel;
  m_diskTooSmallTip->setObjectName("msg_label");
  m_diskTooSmallTip->setMaximumWidth(kWindowWidth);
  m_diskTooSmallTip->setFixedHeight(20);
  m_diskTooSmallTip->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  m_diskTooSmallTip->hide();
  addTransLate(m_trList, [ = ] (const QString& msg) {
      int min_size = GetSettingsInt(kPartitionFullDiskMiniSpace);
      int recommend_size = GetSettingsInt(kPartitionRecommendedDiskSpace);
      m_diskTooSmallTip->setText(msg.arg(min_size).arg(DSysInfo::productType() == DSysInfo::Deepin ? tr("Deepin") : tr("UOS")).arg(recommend_size));
  }, QString("You need at least %1 GB disk space to install %2. To get better performance, %3 GB or more is recommended"));

  QHBoxLayout* tip_layout = new QHBoxLayout();
  tip_layout->setContentsMargins(0, 0, 0, 0);
  tip_layout->setSpacing(0);
  tip_layout->addStretch();
  tip_layout->addWidget(tip_icon, 0, Qt::AlignVCenter);
  tip_layout->addWidget(m_tip_label, 0, Qt::AlignVCenter);
  tip_layout->addStretch();

  m_install_tip = new QFrame();
  m_install_tip->setObjectName("install_tip");
  m_install_tip->setContentsMargins(0, 0, 0, 0);
  // Same width as SimplePartitionButton.
  m_install_tip->setFixedWidth(220);
  m_install_tip->setLayout(tip_layout);
  m_install_tip->hide();

  m_grid_layout = new QGridLayout();
  m_grid_layout->setSpacing(0);
  m_grid_layout->setContentsMargins(0, 0, 0, 0);
  m_grid_layout->setHorizontalSpacing(20);
  m_grid_layout->setVerticalSpacing(20);
  m_grid_layout->setColumnStretch(kDiskColumns, 1);

  m_grid_wrapper = new QFrame();
  m_grid_wrapper->setMaximumWidth(kWindowWidth);
  m_grid_wrapper->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
  m_grid_wrapper->setObjectName("grid_wrapper");
  m_grid_wrapper->setLayout(m_grid_layout);
  m_install_tip->setParent(m_grid_wrapper);

  m_diskInstallationWidget = new MultipleDiskInstallationWidget();
  m_disk_layout = new QStackedLayout();
  m_disk_layout->setSpacing(0);
  m_disk_layout->setContentsMargins(0,0,0,0);
  m_disk_layout->addWidget(m_grid_wrapper);
  m_disk_layout->addWidget(m_diskInstallationWidget);

  QHBoxLayout* hDiskLayout = new QHBoxLayout;
  hDiskLayout->setMargin(0);
  hDiskLayout->setSpacing(0);
  hDiskLayout->addLayout(m_disk_layout);

  QFrame* scroll_frame = new QFrame();
  scroll_frame->setObjectName("scroll_frame");
  scroll_frame->setContentsMargins(0, 0, 0, 0);
  scroll_frame->setLayout(hDiskLayout);

  QScrollArea* scroll_area = new QScrollArea();
  scroll_area->setObjectName("scroll_area");
  scroll_area->setWidget(scroll_frame);
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll_area->setWidgetResizable(true);
  scroll_area->setFixedWidth(800);
  scroll_area->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  m_diskPartitionWidget = new FullDiskPartitionWidget;

  QVBoxLayout* main_layout = new QVBoxLayout();
  main_layout->setContentsMargins(0, 0, 0, 0);
  main_layout->setSpacing(0);
  main_layout->addWidget(scroll_area, 0, Qt::AlignHCenter);
  main_layout->addWidget(m_diskPartitionWidget, 0, Qt::AlignHCenter);

  QHBoxLayout* h_layout = new QHBoxLayout();
  h_layout->addWidget(m_encryptCheck);
  h_layout->addSpacing(10);
  h_layout->addWidget(m_installNvidiaCheck);
  main_layout->addLayout(h_layout);
  main_layout->setAlignment(h_layout, Qt::AlignHCenter);

  main_layout->addSpacing(10);
  main_layout->addWidget(m_errorTip, 0, Qt::AlignHCenter);
  main_layout->addWidget(m_diskTooSmallTip, 0, Qt::AlignHCenter);
  main_layout->addSpacing(10);

  m_encryptCheck->setVisible(!GetSettingsBool(KPartitionSkipFullCryptPage));
  m_installNvidiaCheck->setVisible(m_delegate->scanNvidia());
  WriteEnableNvidiaDriver(false);

  this->setLayout(main_layout);
  this->setContentsMargins(0, 0, 0, 0);

  for (auto it = m_trList.begin(); it != m_trList.end(); ++it) {
      it->first(qApp->translate("installer::FullDiskFrame", it->second.toUtf8()));
  }
}

void FullDiskFrame::repaintDevices() {
  // Clear grid layout.
  ClearLayout(m_grid_layout);

  // Clear button group.
  for (QAbstractButton* button : m_button_group->buttons()) {
    m_button_group->removeButton(button);
  }

  // Hide tooltip frame
  m_install_tip->hide();

  // Draw partitions.
  int row = 0, column = 0;
  for (const Device::Ptr device : m_delegate->virtualDevices()) {
    bool partition_busy = false;
    for (const Partition::Ptr partition : device->partitions) {
      if (partition->busy) {
        partition_busy = true;
        break;
      }
    }
    if (partition_busy) {
      qWarning() << "Partition on device is busy!" << device->path;
      continue;
    }
    SimpleDiskButton* button = new SimpleDiskButton(device);

    m_button_group->addButton(button);
    m_grid_layout->addWidget(button, row, column, Qt::AlignHCenter);

    column += 1;
    // Add rows.
    if (column >= kDiskColumns) {
      column = 0;
      row += 1 ;
    }
  }

  // Add place holder. It is used for install_tip
  row += 1;
  QLabel* place_holder_label = new QLabel(this);
  place_holder_label->setObjectName("place_holder_label");
  place_holder_label->setFixedSize(kWindowWidth, 30);
  m_grid_layout->addWidget(place_holder_label, row, 0,
                          1, kDiskColumns, Qt::AlignHCenter);

  // Resize grid_wrapper explicitly.
  m_grid_wrapper->adjustSize();
}

void FullDiskFrame::showInstallTip(QAbstractButton* button) {
  // Move install_tip to bottom of button
  const QPoint pos = button->pos();
  m_install_tip->move(pos.x(), pos.y() - 10);
  m_install_tip->show();
}

void FullDiskFrame::onDeviceRefreshed() {
  this->repaintDevices();
  m_delegate->removeAllSelectedDisks();
  if (m_delegate->virtualDevices().size() > 1) {
      m_disk_layout->setCurrentWidget(m_diskInstallationWidget);
  }
  else {
      m_disk_layout->setCurrentWidget(m_grid_wrapper);
  }
  m_diskTooSmallTip->hide();
}

void FullDiskFrame::onPartitionButtonToggled(QAbstractButton* button,
                                             bool checked) {
  SimpleDiskButton* part_button = dynamic_cast<SimpleDiskButton*>(button);
  if (!part_button) {
    qCritical() << "no disk button is selected";
    return;
  }

  if (!checked) {
    // Deselect previous button.
    part_button->setSelected(false);
  } else {
    // Hide tooltip.
    m_install_tip->hide();

    m_delegate->addSystemDisk(part_button->device()->path);
    const QString path = part_button->device()->path;
    qDebug() << "selected device path:" << path;
    part_button->setSelected(true);
    // Show install-tip at bottom of current checked button.
    this->showInstallTip(part_button);

    if(!validate()) {
       qWarning() << QString("MULTIDISK:validate FAILED:{%1}").arg(m_delegate->selectedDisks().join(","));
    }

    m_diskPartitionWidget->setDevices(m_delegate->selectedDevices());
    emit showDeviceInfomation();
  }
}

void FullDiskFrame::onCurrentDeviceChanged(int type, const Device::Ptr device)
{
    if (static_cast<int>(DiskModelType::SystemDisk) == type) {
        m_delegate->addSystemDisk(device->path);
    }
    else if(static_cast<int>(DiskModelType::DataDisk) == type){
        m_delegate->addDataDisk(device->path);
    }
    else {
        qWarning() << QString("MULTIDISK:invalid type:{%1}").arg(type);
    }
    if(!validate()) {
       qWarning() << QString("MULTIDISK:validate FAILED:{%1}").arg(m_delegate->selectedDisks().join(","));
    }
    m_diskPartitionWidget->setDevices(m_delegate->selectedDevices());
    emit showDeviceInfomation();
}

void FullDiskFrame::installNvidiaStateChanged(bool install_nvidia)
{
    WriteEnableNvidiaDriver(install_nvidia);
}

}  // namespace installer
