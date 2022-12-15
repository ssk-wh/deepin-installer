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
#include <qcheckbox.h>
#include <qnamespace.h>
#include <qpushbutton.h>

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
#include <QProcess>

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
#include "resize_root_frame.h"

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

bool FullDiskFrame::isEnSaveData() const
{
    return m_saveDataCheck->isChecked();
}

void FullDiskFrame::setSaveDataCheckEnabel(bool isenabel)
{
    m_saveDataCheck->setEnabled(isenabel);
}

bool FullDiskFrame::focusSwitch()
{
    if (m_saveDataCheck->hasFocus()) {
        if (m_encryptCheck->isVisible() && m_encryptCheck->isEnabled()) {
            m_encryptCheck->setFocus();
            return false;
        } else if (m_resizeCheck->isVisible() && m_resizeCheck->isEnabled()) {
            m_resizeCheck->setFocus();
            return false;
        } else {
            return true;
        }
    } else if (m_encryptCheck->hasFocus()) {
        return true;
    } else if (m_resizeCheck->hasFocus()) {
        return true;
    } else {
        if (m_disk_layout->currentWidget() == m_diskInstallationWidget) {
            if(m_diskInstallationWidget->isLeftRightWidgetHasFocus()) {
                if(m_diskInstallationWidget->focusSwitch()) {
                    if(m_encryptCheck->isVisible()) {
                        m_encryptCheck->setFocus();
                    } else if (m_resizeCheck->isVisible()){
                        m_resizeCheck->setFocus();
                    } else {
                        return true;
                    }
                }
            } else {
                m_diskInstallationWidget->setFocus();
            }
        } else if (m_disk_layout->currentWidget() == m_grid_wrapper) {
            if(m_grid_wrapper->hasFocus()) {
                if (m_saveDataCheck->isVisible()) {
                    m_saveDataCheck->setFocus();
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
    if (m_saveDataCheck->hasFocus()) {
        if (m_saveDataCheck->checkState() == Qt::Checked) {
            m_saveDataCheck->setCheckState(Qt::Unchecked);
            m_encryptCheck->setEnabled(true);
            m_resizeCheck->setEnabled(m_devices.length() == 1);
        } else {
            m_saveDataCheck->setCheckState(Qt::Checked);
            m_encryptCheck->setCheckState(Qt::Unchecked);
            m_encryptCheck->setEnabled(false);
            m_resizeCheck->setCheckState(Qt::Unchecked);
            m_resizeCheck->setEnabled(false);
        }
    } else if(m_encryptCheck->hasFocus()) {
        if (m_encryptCheck->checkState() == Qt::Checked) {
            m_encryptCheck->setCheckState(Qt::Unchecked);
        } else {
            m_encryptCheck->setCheckState(Qt::Checked);
        }
    } else if(m_resizeCheck->hasFocus()) {
        if (m_resizeCheck->checkState() == Qt::Checked) {
            m_resizeCheck->setCheckState(Qt::Unchecked);
        } else {
            m_resizeCheck->setCheckState(Qt::Checked);
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

bool FullDiskFrame::isSaveData()
{
    return GetSettingsBool(KIsSaveData);
}

void FullDiskFrame::clearPartInfos()
{
    if (m_diskPartitionWidget != nullptr) {
        m_diskPartitionWidget->clearView();
    }
}

void FullDiskFrame::setDeviceByAutoSelectDisk()
{
    if (m_diskInstallationWidget) {
        m_diskInstallationWidget->setDeviceByAutoSelectDisk();
    }
}

void FullDiskFrame::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        for (auto it = m_trList.begin(); it != m_trList.end(); ++it) {
            it->first(qApp->translate("QObject", it->second.toUtf8()));
        }
        m_saveDataCheck->setText(::QObject::tr("Keep User Data"));
        m_encryptCheck->setText(::QObject::tr("Encrypt This Disk"));
        m_errorTip->setText(::QObject::tr("Please select a disk to start installation"));
        m_resizeCheck->setText(::QObject::tr("Resize Root partition"));

        if ( !m_tip_label->text().isEmpty() ) {
            showInstallTip(true);
        }

        int min_size = GetSettingsInt(kPartitionFullDiskMiniSpace);
        int fullDiskMin = GetSettingsInt(kPartitionFullDiskMiniSpace);
        m_diskTooSmallTip->setText(::QObject::tr("The free disk space is less than %1 GB,"
                                                 " so you should configure partitions manually."
                                                 " To use the full-disk installation, a minimum of %2 GB is required.")
                                   .arg(min_size)
                                   .arg(fullDiskMin));
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
  connect(m_delegate, &FullDiskDelegate::deviceRefreshed,
          m_diskInstallationWidget, &MultipleDiskInstallationWidget::onDeviceListChanged);
  connect(m_diskInstallationWidget, &MultipleDiskInstallationWidget::currentDeviceChanged,
          this, &FullDiskFrame::onCurrentDeviceChanged);
  connect(m_diskInstallationWidget, &MultipleDiskInstallationWidget::autoSelectDisk,
          this, &FullDiskFrame::showAutoSelectWidget);
  connect(m_saveDataCheck, &QCheckBox::clicked, this, &FullDiskFrame::saveDataStateChanged);
  connect(m_delegate, &FullDiskDelegate::selectedDevicesChanged, this, [=](const DeviceList& devices) {
    m_devices = devices;
    m_resizeCheck->setEnabled(devices.length() == 1);
    if (devices.length() == 2) {
        emit m_resizeCheck->clicked(false);
        m_resizeCheck->setChecked(false);
    }
  });
  connect(m_resizeCheck, &QCheckBox::clicked, this, [=](bool checked) {
      if (checked) {
          emit showResizeRootWidget();
      }
  }, Qt::QueuedConnection);
}

void FullDiskFrame::initUI() {
  m_button_group = new QButtonGroup(this);

  m_saveDataCheck = new QCheckBox;
  m_saveDataCheck->setObjectName("check_box");
  m_saveDataCheck->setCheckable(true);
  m_saveDataCheck->setChecked(false);
  m_saveDataCheck->setEnabled(false);
  m_saveDataCheck->setVisible(isSaveData());
  addTransLate(m_trList, std::bind(&QCheckBox::setText, m_saveDataCheck, std::placeholders::_1), ::QObject::tr("Save User Data"));

  m_encryptCheck = new QCheckBox;
  m_encryptCheck->setObjectName("check_box");
  m_encryptCheck->setCheckable(true);
  m_encryptCheck->setChecked(false);
  //m_encryptCheck->setFocusPolicy(Qt::TabFocus);
  addTransLate(m_trList, std::bind(&QCheckBox::setText, m_encryptCheck, std::placeholders::_1), ::QObject::tr("Encrypt This Disk"));

  m_resizeCheck = new QCheckBox;
  m_resizeCheck->setObjectName("resize_button");
  m_resizeCheck->setChecked(false);
  m_resizeCheck->setCheckable(true);
  m_resizeCheck->setVisible(GetSettingsBool(kEnableChangeRootSize));
  m_resizeCheck->setEnabled(false);

  m_errorTip = new QLabel;
  m_errorTip->setObjectName("msg_label");
  m_errorTip->hide();
  addTransLate(m_trList, std::bind(&QLabel::setText, m_errorTip, std::placeholders::_1), ::QObject::tr("Please select a disk to start installation"));

  m_diskTooSmallTip = new QLabel;
  m_diskTooSmallTip->setObjectName("msg_label");
  m_diskTooSmallTip->setFixedWidth(kWindowWidth);
  m_diskTooSmallTip->setWordWrap(true);
  m_diskTooSmallTip->setAlignment(Qt::AlignCenter);
  m_diskTooSmallTip->hide();

  QHBoxLayout* tipLayout = new QHBoxLayout();
  tipLayout->setContentsMargins(0, 0, 0, 0);
  tipLayout->setSpacing(0);
  tipLayout->addWidget(m_diskTooSmallTip);

  m_tip_icon = new QLabel;
  m_tip_icon->setPixmap(installer::renderPixmap(":/images/install_icon.svg"));

  QPalette palette;
  palette.setColor(QPalette::Text, QColor("#ff8000"));
  m_tip_label = new QLabel("");
  m_tip_label->setObjectName("tip_label");
  m_tip_label->setPalette(palette);
  m_tip_label->setForegroundRole(QPalette::Text);

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
  m_diskPartitionWidget->setMinimumHeight(88);

  QVBoxLayout* main_layout = new QVBoxLayout();
  main_layout->setContentsMargins(0, 0, 0, 0);
  main_layout->setSpacing(2);
  main_layout->addWidget(scroll_frame, 0, Qt::AlignHCenter);
  main_layout->addWidget(m_diskPartitionWidget, 0, Qt::AlignHCenter);
  main_layout->addStretch();
  QHBoxLayout* h_layout = new QHBoxLayout();
  h_layout->addWidget(m_saveDataCheck);
  h_layout->addWidget(m_encryptCheck);
  h_layout->addWidget(m_resizeCheck);
  h_layout->setSpacing(91);
  main_layout->addSpacing(10);
  main_layout->addLayout(h_layout);
  main_layout->addSpacing(10);
  main_layout->setAlignment(h_layout, Qt::AlignHCenter);
  main_layout->addWidget(m_errorTip, 0, Qt::AlignHCenter);
  main_layout->addLayout(tipLayout);
  main_layout->addSpacing(10);

  m_encryptCheck->setVisible(!GetSettingsBool(KPartitionSkipFullCryptPage));
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

bool FullDiskFrame::isExistDataPart(const QString &devicepath)
{    
    QString cmd = QString("lsblk -f %1 | grep _dde_data").arg(devicepath);
    QProcess testprocess;
    testprocess.start("/bin/bash",{"-c", cmd});
    testprocess.waitForFinished();
    QString dataindevices = testprocess.readAll();
    qWarning() << "dataindevices = " << dataindevices;
    if (dataindevices.compare("")) {
        return true;
    } else {
        return false;
    }
}

bool FullDiskFrame::isSystemDisk(const QString &devicepath)
{
    QString cmd = QString("lsblk -o LABEL -lnf %1 | grep [a-z/A-Z] | xargs").arg(devicepath);
    QProcess testprocess;
    testprocess.start("/bin/bash",{"-c", cmd});
    testprocess.waitForFinished();
    QString devicelabels = testprocess.readAll();
    qWarning() << "devicelabels = " << devicelabels;
    QStringList devicelabelslist = devicelabels.split(" ");

    int sysytemdisknums = 0;

    if (devicelabelslist.contains("Roota"))
        sysytemdisknums++;

    if (sysytemdisknums == 1) {
        return true;
    } else {
        return false;
    }
}

bool FullDiskFrame::isFullDiskEncrypt(const QString &devicepath)
{
    QString cmd = QString("lsblk -f %1 | grep crypto_LUKS").arg(devicepath);
    QProcess testprocess;
    testprocess.start("/bin/bash",{"-c", cmd});
    testprocess.waitForFinished();
    QString cryptodevices = testprocess.readAll();
    qWarning() << "cryptodevices = " << cryptodevices;
    if (cryptodevices.compare("")) {
        return true;
    } else {
        return false;
    }
}

void FullDiskFrame::setSaveDataCheckboxStat(const Device::Ptr device,  const int type)
{
    // 判断是否关闭了保留用户数据开关
    if (!isSaveData()) {
        WriteSaveUserData(false);
        return;
    }
    // 判断是否是一个系统盘
    bool testissystemdisk = false;
    if (static_cast<int>(DiskModelType::SystemDisk) == type) {
        testissystemdisk = isSystemDisk(device->path);
    } else if (static_cast<int>(DiskModelType::DataDisk) == type) {
        testissystemdisk = isSystemDisk(m_delegate->selectedDisks().at(0));
    }

    bool testexistdata = isExistDataPart(device->path);
    bool testfulldiskencrypt = isFullDiskEncrypt(device->path);

    bool testsametabletype = true;
    PartitionTableType currenttable = IsEfiEnabled() ? PartitionTableType::GPT : PartitionTableType::MsDos;
    //如果直接使用传进来的device来判断table，存在table被修改的可能，所以这里从realdevice中找到需要的device
    const DeviceList realDevices = m_delegate->realDevices();
    for (int i = 0; i < realDevices.size(); i++) {
        if (!device->path.compare(realDevices.at(i)->path)) {
            // 检测磁盘格式和引导模式是否一致
            testsametabletype = (realDevices.at(i)->table == currenttable) ? true : false;
            qWarning() << "device table : " << realDevices.at(i)->table << " currenttable : " << currenttable;
            break;
        }
    }

    // 存在data分区并且在选择系统盘时所选盘存在系统分区
    // 或者存在data分区并且选择数据盘时，选择的系统盘存在系统分区
    if ( testexistdata && (!testfulldiskencrypt) && testsametabletype && testissystemdisk ) {
        m_saveDataCheck->setEnabled(!m_encryptCheck->isChecked() && !m_resizeCheck->isChecked());
        emit showSaveDataPopWidget();
    } else {
        m_saveDataCheck->setChecked(false);
        m_saveDataCheck->setEnabled(false);
        m_encryptCheck->setEnabled(true);
        m_resizeCheck->setEnabled(m_devices.length() == 1);
        WriteSaveUserData(false);
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
    showInstallTip(false);
  } else {    

    m_delegate->addSystemDisk(part_button->device()->path);

    // 设置保留用户数据勾选控件
    setSaveDataCheckboxStat(part_button->device());

    const QString path = part_button->device()->path;
    qDebug() << "selected device path:" << path;
    part_button->setSelected(true);
    // Show install-tip at bottom of current checked button.
    showInstallTip(true);

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

    // 设置保留用户数据勾选控件
    setSaveDataCheckboxStat(device, type);

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

void FullDiskFrame::saveDataStateChanged(bool savedata)
{
    if (savedata) {
        m_saveDataCheck->setEnabled(true);
        m_encryptCheck->setChecked(false);
        m_encryptCheck->setEnabled(false);
        m_resizeCheck->setChecked(false);
        m_resizeCheck->setEnabled(false);
    } else {
        m_encryptCheck->setEnabled(true);
        m_resizeCheck->setEnabled(m_devices.length() == 1);
    }

    m_saveDataCheck->setChecked(savedata);
    WriteSaveUserData(savedata);
}

void FullDiskFrame::cryptoStateChanged(bool crypto)
{
    if (crypto) {
        m_saveDataCheck->setChecked(false);
        m_saveDataCheck->setEnabled(false);
        WriteSaveUserData(false);
    } else {
        // 当撤销“全盘加密”的勾选状态时，“保留用户数据”的可用状态根据重新判断所选的系统盘和数据盘确定
        DeviceList testdevicelist = m_delegate->selectedDevices();
        for (int i = 0; i < testdevicelist.size(); i++) {
            setSaveDataCheckboxStat(testdevicelist.at(i), i);
        }
    }

    m_encryptCheck->setChecked(crypto);
}

void FullDiskFrame::onResizeRootFrameFinished() {
    auto oldSelectedDisks = m_delegate->selectedDisks();
    m_delegate->formatWholeDeviceMultipleDisk();

    m_delegate->removeAllSelectedDisks();

    if (oldSelectedDisks.length() == 1) {
        m_delegate->addSystemDisk(oldSelectedDisks.first());
    }
    else {
        m_delegate->addSystemDisk(oldSelectedDisks.first());
        m_delegate->addDataDisk(oldSelectedDisks.last());
    }

    m_diskPartitionWidget->setDevices(m_delegate->selectedDevices());
}

void FullDiskFrame::onResizeRootFrameCanceled()
{
    m_resizeCheck->setChecked(false);
}

}  // namespace installer
