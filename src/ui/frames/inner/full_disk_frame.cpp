/*
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
#include <DFrame>

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
#include "ui/delegates/license_delegate.h"

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace installer {

namespace {

// 4 partitions are displays at each row.
const int kDiskColumns = 1;

const int kWindowWidth = 560;

enum class DiskCountType : int
{
      SingleDisk = 0
    , MultipleDisk
};

}  // namespace

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
    emit enableNextButton(!m_diskTooSmallTip->isVisible());

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
            emit enableNextButton(!m_diskTooSmallTip->isVisible());
            qWarning() << QString("MULTIDISK: disk too small:size:{%1}.").arg(device->getByteLength());
            return false;
        }
    }

    if (!m_delegate->formatWholeDeviceMultipleDisk()) {
        m_diskTooSmallTip->show();
        emit enableNextButton(!m_diskTooSmallTip->isVisible());
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

bool FullDiskFrame::focusSwitch()
{
    if(m_encryptCheck->hasFocus()) {
        return true;
    } else {
        if (m_disk_layout->currentWidget() == m_diskInstallationWidget) {
            if(m_diskInstallationWidget->isLeftRightWidgetHasFocus()) {
                if(m_diskInstallationWidget->focusSwitch()) {
                    if(m_encryptCheck->isVisible()) {
                        m_encryptCheck->setFocus();
                    } else {
                        return true;
                    }
                }
            } else {
                m_diskInstallationWidget->setFocus();
            }
        } else if (m_disk_layout->currentWidget() == m_grid_wrapper) {
            if(m_grid_wrapper->hasFocus()) {
                if (m_encryptCheck->isVisible()) {
                    m_encryptCheck->setFocus();
                } else {
                    return true;
                }
            } else {
                m_grid_wrapper->setFocus();
            }
        }
        return false;
    }
}

bool FullDiskFrame::doSpace()
{
    if(m_encryptCheck->hasFocus()) {
        if (m_encryptCheck->checkState() == Qt::Checked) {
            m_encryptCheck->setCheckState(Qt::Unchecked);
        } else {
            m_encryptCheck->setCheckState(Qt::Checked);
        }
    } else if (m_grid_wrapper->hasFocus()) {
        if(m_button_group->buttons().size() > 0) {
            m_button_group->buttons().at(0)->toggle();
        }
    }
    return true;
}

bool FullDiskFrame::doSelect()
{
    if (m_grid_wrapper->hasFocus()) {
        if(m_button_group->buttons().size() > 0) {
            m_button_group->buttons().at(0)->toggle();
        }
    }
    return true;
}

bool FullDiskFrame::directionKey(int keyvalue)
{
    if (m_diskInstallationWidget->isLeftRightWidgetHasFocus()) {
        return m_diskInstallationWidget->directionKey(keyvalue);
    } else {
        return true;
    }
}

void FullDiskFrame::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        for (auto it = m_trList.begin(); it != m_trList.end(); ++it) {
            it->first(qApp->translate("QObject", it->second.toUtf8()));
        }
        m_installNvidiaCheck->setText(::QObject::tr("Install NVIDIA closed source driver"));
        m_encryptCheck->setText(::QObject::tr("Encrypt This Disk"));
        m_errorTip->setText(::QObject::tr("Please select a disk to start installation"));

        if ( !m_tip_label->text().isEmpty() ) {
            showInstallTip(true);
        }

        int min_size = GetSettingsInt(kPartitionFullDiskMiniSpace);
        int recommend_size = GetSettingsInt(kPartitionRecommendedDiskSpace);
        m_diskTooSmallTip->setText(::QObject::tr("You need at least %1 GB disk space to install %2. To get better performance, %3 GB or more is recommended").arg(min_size).arg(DSysInfo::productType() == DSysInfo::Deepin ? ::QObject::tr("Deepin") : LicenseDelegate::product()).arg(recommend_size));
    }
    else {
        QFrame::changeEvent(event);
    }
}

void FullDiskFrame::showEvent(QShowEvent *event)
{
    emit enableNextButton(!m_diskTooSmallTip->isVisible());

    QFrame::showEvent(event);
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

  m_encryptCheck = new QCheckBox;
  m_encryptCheck->setObjectName("check_box");
  m_encryptCheck->setCheckable(true);
  m_encryptCheck->setChecked(false);
  //m_encryptCheck->setFocusPolicy(Qt::TabFocus);
  addTransLate(m_trList, std::bind(&QCheckBox::setText, m_encryptCheck, std::placeholders::_1), ::QObject::tr("Encrypt This Disk"));

  m_installNvidiaCheck = new QCheckBox;
  m_installNvidiaCheck->setObjectName("check_box");
  m_installNvidiaCheck->setCheckable(true);
  m_installNvidiaCheck->setChecked(false);
  //m_installNvidiaCheck->setFocusPolicy(Qt::TabFocus);
  m_installNvidiaCheck->setText(::QObject::tr("Install NVIDIA closed source driver"));

  m_errorTip = new QLabel;
  m_errorTip->setObjectName("msg_label");
  m_errorTip->hide();
  addTransLate(m_trList, std::bind(&QLabel::setText, m_errorTip, std::placeholders::_1), ::QObject::tr("Please select a disk to start installation"));

  m_diskTooSmallTip = new QLabel;
  m_diskTooSmallTip->setObjectName("msg_label");
  m_diskTooSmallTip->setFixedWidth(kWindowWidth);
  m_diskTooSmallTip->setFixedHeight(40);
//  m_diskTooSmallTip->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  m_diskTooSmallTip->setWordWrap(true);
  m_diskTooSmallTip->setAlignment(Qt::AlignCenter);
  m_diskTooSmallTip->hide();
  addTransLate(m_trList, [ = ] (const QString& msg) {
      int min_size = GetSettingsInt(kPartitionFullDiskMiniSpace);
      int recommend_size = GetSettingsInt(kPartitionRecommendedDiskSpace);
      m_diskTooSmallTip->setText(msg.arg(min_size).arg(DSysInfo::productType() == DSysInfo::Deepin ? ::QObject::tr("Deepin") : LicenseDelegate::product()).arg(recommend_size));
  }, ::QObject::tr("You need at least %1 GB disk space to install %2. To get better performance, %3 GB or more is recommended"));

  m_tip_icon = new QLabel;
  m_tip_icon->setPixmap(installer::renderPixmap(":/images/install_icon.svg"));

  QPalette palette;
  palette.setColor(QPalette::Text, QColor("#ff8000"));
  m_tip_label = new QLabel("");
  m_tip_label->setObjectName("tip_label");
  m_tip_label->setPalette(palette);
  addTransLate(m_trList, std::bind(&QLabel::setText, m_tip_label, std::placeholders::_1), ::QObject::tr("Install here"));

  QHBoxLayout* tip_layout = new QHBoxLayout;
  tip_layout->setContentsMargins(0, 0, 0, 0);
  tip_layout->setSpacing(0);
  tip_layout->addWidget(m_tip_icon, 0, Qt::AlignVCenter);
  tip_layout->addWidget(m_tip_label, 0, Qt::AlignVCenter);

  m_install_tip = new QFrame(this);
  m_install_tip->setObjectName("install_tip");
  m_install_tip->setContentsMargins(0, 0, 0, 0);
  // Same width as SimplePartitionButton.
  m_install_tip->setLayout(tip_layout);
  showInstallTip(false);

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
  m_grid_wrapper->setStyleSheet("QWidget#grid_wrapper::focus{border:1px solid; border-color:rgb(1, 128, 255); border-radius:5px; padding:2px 4px;}");

  m_diskInstallationWidget = new MultipleDiskInstallationWidget();

  m_disk_layout = new QStackedLayout();
  m_disk_layout->setSpacing(0);
  m_disk_layout->setContentsMargins(0, 0, 0, 0);
  m_disk_layout->addWidget(m_grid_wrapper);
  m_disk_layout->addWidget(m_diskInstallationWidget);

  QVBoxLayout* vDiskLayout = new QVBoxLayout;
  vDiskLayout->setContentsMargins(0, 0, 0, 0);
  vDiskLayout->setSpacing(0);
  vDiskLayout->addLayout(m_disk_layout);
  vDiskLayout->addWidget(m_install_tip, 0, Qt::AlignHCenter);

  DFrame* scroll_frame = new DFrame();
  scroll_frame->setFrameRounded(true);
  scroll_frame->setObjectName("scroll_frame");
  scroll_frame->setContentsMargins(0, 0, 0, 0);
  scroll_frame->setLayout(vDiskLayout);
  scroll_frame->setMaximumHeight(530);

  m_diskPartitionWidget = new FullDiskPartitionWidget;
  m_diskPartitionWidget->setObjectName("m_diskPartitionWidget");
  m_diskPartitionWidget->setMinimumHeight(85);

  QVBoxLayout* main_layout = new QVBoxLayout();
  main_layout->setContentsMargins(0, 0, 0, 0);
  main_layout->setSpacing(2);
  main_layout->addWidget(scroll_frame, 0, Qt::AlignHCenter);
  main_layout->addWidget(m_diskPartitionWidget, 0, Qt::AlignHCenter);
  main_layout->addStretch();
  QHBoxLayout* h_layout = new QHBoxLayout();
  h_layout->addWidget(m_encryptCheck);
  h_layout->addWidget(m_installNvidiaCheck);
  main_layout->addSpacing(10);
  main_layout->addLayout(h_layout);
  main_layout->addSpacing(10);
  main_layout->setAlignment(h_layout, Qt::AlignHCenter);
  main_layout->addWidget(m_errorTip, 0, Qt::AlignHCenter);
  main_layout->addWidget(m_diskTooSmallTip, 0, Qt::AlignHCenter);
  main_layout->addSpacing(10);

  m_encryptCheck->setVisible(!GetSettingsBool(KPartitionSkipFullCryptPage));
  m_installNvidiaCheck->setVisible(m_delegate->scanNvidia());
  WriteEnableNvidiaDriver(false);

  this->setLayout(main_layout);
  this->setContentsMargins(0, 0, 0, 0);
  this->setFocusProxy(m_diskInstallationWidget);
}

void FullDiskFrame::repaintDevices() {
  // Clear grid layout.
  ClearLayout(m_grid_layout);

  // Clear button group.
  for (QAbstractButton* button : m_button_group->buttons()) {
    m_button_group->removeButton(button);
  }

  // Hide tooltip frame
  showInstallTip(false);

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
    SimpleDiskButton* button = new SimpleDiskButton(device, this);

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

void FullDiskFrame::showInstallTip(bool isshow) {
    if (isshow) {
        m_tip_icon->show();
        m_tip_label->setText(::QObject::tr("Install here"));
    } else {
        m_tip_icon->hide();
        m_tip_label->setText("");
    }
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
  emit enableNextButton(!m_diskTooSmallTip->isVisible());
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
    showInstallTip(false);

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

    // If validate device failed, then clear last device partition info in the widget.
    if(!validate()) {
        qWarning() << QString("MULTIDISK:validate FAILED:{%1}").arg(m_delegate->selectedDisks().join(","));
        m_diskPartitionWidget->clearView();
    }
    else {
        m_diskPartitionWidget->setDevices(m_delegate->selectedDevices());
        emit showDeviceInfomation();
    }
}

void FullDiskFrame::installNvidiaStateChanged(bool install_nvidia)
{
    WriteEnableNvidiaDriver(install_nvidia);
}

}  // namespace installer
