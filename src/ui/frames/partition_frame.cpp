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

#include "ui/frames/partition_frame.h"

#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/advanced_partition_delegate.h"
#include "ui/delegates/full_disk_delegate.h"
#include "ui/delegates/simple_partition_delegate.h"
#include "ui/frames/dynamic_disk_warning_frame.h"
#include "ui/delegates/partition_util.h"
#include "ui/frames/consts.h"
#include "ui/frames/inner/advanced_partition_frame.h"
#include "ui/frames/inner/edit_partition_frame.h"
#include "ui/frames/inner/full_disk_frame.h"
#include "ui/frames/inner/new_partition_frame.h"
#include "ui/frames/inner/new_table_loading_frame.h"
#include "ui/frames/inner/new_table_warning_frame.h"
#include "ui/frames/inner/partition_loading_frame.h"
#include "ui/frames/inner/partition_number_limitation_frame.h"
#include "ui/frames/inner/partition_table_warning_frame.h"
#include "ui/frames/inner/prepare_install_frame.h"
#include "ui/frames/inner/select_bootloader_frame.h"
#include "ui/frames/inner/simple_partition_frame.h"
#include "ui/frames/inner/full_disk_encrypt_frame.h"
#include "ui/models/partition_model.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/pointer_button.h"
#include "ui/interfaces/frameinterfaceprivate.h"

#include <QButtonGroup>
#include <QEvent>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QProcess>
#include <QTextStream>
#include <DButtonBox>

DWIDGET_USE_NAMESPACE

namespace installer {

namespace {

}  // namespace

class PartitionFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT

    friend PartitionFrame;

public:
    explicit PartitionFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate (parent)
        , q_ptr(qobject_cast<PartitionFrame* >(parent))
        ,partition_model_(new PartitionModel(this))
        ,advanced_delegate_(new AdvancedPartitionDelegate(this))
        ,full_disk_delegate_(new FullDiskDelegate(this))
        ,simple_partition_delegate_(new SimplePartitionDelegate(this))
    {}

    ~PartitionFramePrivate() override;

     void initConnections();
     void initUI();

     // Check current partition mode is simple mode or not.
     bool isSimplePartitionMode();
     bool isFullDiskPartitionMode();

     bool isRawDevice(const QList<Device::Ptr> list);

     void onButtonGroupToggled(QAbstractButton *button);
     void onNextButtonClicked();
     void onFullDiskCryptoButtonClicked(bool encrypto);

     // Write partitioning settings to conf file and emit manualPartDone() signal.
     void onManualPartDone(bool ok, const DeviceList& devices);

     // Notify delegate to do manual part.
     void onPrepareInstallFrameFinished();

     void showEditPartitionFrame(const Partition::Ptr partition);
     void showMainFrame();
     void showNewPartitionFrame(const Partition::Ptr partition);
     void showNewTableLoadingFrame();
     void showNewTableWarningFrame(const QString& device_path);
     void showPartitionNumberLimitationFrame();
     void showPartitionTableWarningFrame(const QString& device_path);
     void showSelectBootloaderFrame();
     void showEncryptFrame();
     void showDynamicDiskFrame();
     void showPrepareInstallFrame();

     PartitionFrame* q_ptr=nullptr;

     AdvancedPartitionFrame* advanced_partition_frame_ = nullptr;
     EditPartitionFrame* edit_partition_frame_ = nullptr;
     FullDiskFrame* full_disk_partition_frame_ = nullptr;
     NewPartitionFrame* new_partition_frame_ = nullptr;
     NewTableLoadingFrame* new_table_loading_frame_ = nullptr;
     NewTableWarningFrame* new_table_warning_frame_ = nullptr;
     PartitionLoadingFrame* partition_loading_frame_ = nullptr;
     PartitionNumberLimitationFrame* partition_number_limitation_frame_ = nullptr;
     PartitionTableWarningFrame* partition_table_warning_frame_ = nullptr;
     PrepareInstallFrame* prepare_install_frame_ = nullptr;
     SelectBootloaderFrame* select_bootloader_frame_ = nullptr;
     SimplePartitionFrame* simple_partition_frame_ = nullptr;
     Full_Disk_Encrypt_frame* full_disk_encrypt_frame_ = nullptr;
     DynamicDiskWarningFrame* dynamic_disk_warning_frame_ = nullptr;

     TitleLabel* title_label_ = nullptr;
     CommentLabel* comment_label_ = nullptr;
     QFrame* main_frame_ = nullptr;
     QStackedLayout* partition_stacked_layout_ = nullptr;
     QStackedLayout* main_layout_ = nullptr;
     DButtonBox* m_buttonGroup = nullptr;
     DButtonBoxButton* full_disk_frame_button_ = nullptr;
     DButtonBoxButton* simple_frame_button_ = nullptr;
     DButtonBoxButton* advanced_frame_button_ = nullptr;

     PartitionModel* partition_model_ = nullptr;
     AdvancedPartitionDelegate* advanced_delegate_ = nullptr;
     FullDiskDelegate* full_disk_delegate_ = nullptr;
     SimplePartitionDelegate* simple_partition_delegate_ = nullptr;
};

PartitionFrame::PartitionFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new PartitionFramePrivate(this))
{
    setObjectName("partition_frame");

    m_private->initUI();
    m_private->initConnections();
}

PartitionFrame::~PartitionFrame()
{

}

void PartitionFrame::init()
{

}

void PartitionFrame::finished()
{

}

bool PartitionFrame::shouldDisplay() const{
  return !GetSettingsBool(kSkipPartitionPage);
}

void PartitionFrame::autoPart() {
    WriteFullDiskMode(true);
    m_private->partition_model_->autoPart();
}

void PartitionFrame::scanDevices()  const{
    m_private->partition_model_->scanDevices();
}

void PartitionFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    m_private->title_label_->setText(tr("Select Installation Location"));
    m_private->comment_label_->setText(
        tr("Make sure you have backed up important data, then select the disk to install"));
    m_private->simple_frame_button_->setText(tr("Simple"));
    m_private->advanced_frame_button_->setText(tr("Advanced"));
    m_private->full_disk_frame_button_->setText(tr("Full Disk"));
    m_private->nextButton->setText(tr("Start installation"));
  } else {
      FrameInterface::changeEvent(event);
  }
}

PartitionFramePrivate::~PartitionFramePrivate()
{

}

void PartitionFramePrivate::initConnections() {
  disconnect(nextButton, 0, 0, 0);
  connect(nextButton, &QPushButton::clicked, this, [=] {
       if (partition_stacked_layout_->currentWidget() == full_disk_partition_frame_ && full_disk_partition_frame_->isEncrypt()) {
           showEncryptFrame();
       }
       else {
           onNextButtonClicked();
       }
  });

  connect(m_buttonGroup, &DButtonBox::buttonClicked, this, &PartitionFramePrivate::onButtonGroupToggled);

  // Show main frame when device is refreshed.
  connect(partition_model_, &PartitionModel::deviceRefreshed,
          this, &PartitionFramePrivate::showMainFrame);
  connect(partition_model_, &PartitionModel::autoPartDone,
          q_ptr, &PartitionFrame::autoPartDone);
  connect(partition_model_, &PartitionModel::manualPartDone,
          this, &PartitionFramePrivate::onManualPartDone);

  connect(advanced_partition_frame_,
          &AdvancedPartitionFrame::requestEditPartitionFrame,
          this, &PartitionFramePrivate::showEditPartitionFrame);
  connect(advanced_partition_frame_,
          &AdvancedPartitionFrame::requestNewPartitionFrame,
          this, &PartitionFramePrivate::showNewPartitionFrame);
  connect(advanced_partition_frame_, &AdvancedPartitionFrame::requestNewTable,
          this, &PartitionFramePrivate::showPartitionTableWarningFrame);
  connect(advanced_partition_frame_,
          &AdvancedPartitionFrame::requestPartitionNumberLimitationFrame,
          this, &PartitionFramePrivate::showPartitionNumberLimitationFrame);
  connect(advanced_partition_frame_,
          &AdvancedPartitionFrame::requestSelectBootloaderFrame,
          this, &PartitionFramePrivate::showSelectBootloaderFrame);

  connect(edit_partition_frame_, &EditPartitionFrame::finished, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
  });
  connect(new_partition_frame_, &NewPartitionFrame::finished, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
  });

  connect(new_table_warning_frame_, &NewTableWarningFrame::canceled,
          this, &PartitionFramePrivate::showMainFrame);
  connect(new_table_warning_frame_, &NewTableWarningFrame::confirmed,
          this, &PartitionFramePrivate::showNewTableLoadingFrame);
  connect(new_table_warning_frame_, &NewTableWarningFrame::confirmed,
          partition_model_, &PartitionModel::createPartitionTable);

  connect(partition_number_limitation_frame_,
          &PartitionNumberLimitationFrame::finished,
          this, &PartitionFramePrivate::showMainFrame);

  connect(partition_table_warning_frame_, &PartitionTableWarningFrame::canceled,
          this, &PartitionFramePrivate::showMainFrame);
  connect(partition_table_warning_frame_,
          &PartitionTableWarningFrame::confirmed,
          this, &PartitionFramePrivate::showNewTableWarningFrame);
  connect(partition_table_warning_frame_, &PartitionTableWarningFrame::reboot,
          q_ptr, &PartitionFrame::reboot);

  connect(prepare_install_frame_, &PrepareInstallFrame::aborted,
          this, &PartitionFramePrivate::showMainFrame);
  connect(prepare_install_frame_, &PrepareInstallFrame::finished,
          this, &PartitionFramePrivate::onPrepareInstallFrameFinished);

  connect(select_bootloader_frame_, &SelectBootloaderFrame::bootloaderUpdated,
          advanced_partition_frame_,
          &AdvancedPartitionFrame::setBootloaderPath);
  connect(select_bootloader_frame_, &SelectBootloaderFrame::bootloaderUpdated,
          advanced_delegate_, &AdvancedPartitionDelegate::setBootloaderPath);
  connect(select_bootloader_frame_, &SelectBootloaderFrame::finished,
          this, &PartitionFramePrivate::showMainFrame);
  connect(advanced_delegate_, &AdvancedPartitionDelegate::deviceRefreshed,
          select_bootloader_frame_, &SelectBootloaderFrame::deviceRefreshed);

  connect(partition_model_, &PartitionModel::deviceRefreshed,
          advanced_delegate_, &AdvancedPartitionDelegate::onDeviceRefreshed);

  if (!GetSettingsBool(kPartitionSkipSimplePartitionPage)) {
    connect(partition_model_, &PartitionModel::deviceRefreshed,
            simple_partition_delegate_,
            &SimplePartitionDelegate::onDeviceRefreshed);

  }
  if (!GetSettingsBool(kPartitionSkipFullDiskPartitionPage)) {
    connect(partition_model_, &PartitionModel::deviceRefreshed,
            full_disk_delegate_, &FullDiskDelegate::onDeviceRefreshed);
  }

  // TODO(Shaohua): Show warning page both in full-disk frame and
  // simple-partition frame.
  connect(simple_partition_frame_, &SimplePartitionFrame::requestNewTable,
          this, &PartitionFramePrivate::showPartitionTableWarningFrame);

  connect(full_disk_encrypt_frame_, &Full_Disk_Encrypt_frame::cancel,
          this, &PartitionFramePrivate::showMainFrame);

  connect(full_disk_encrypt_frame_, &Full_Disk_Encrypt_frame::finished, q_ptr, [=] {
      q_ptr->autoPart();
      q_ptr->m_proxy->nextFrame();
  });

  connect(dynamic_disk_warning_frame_, &DynamicDiskWarningFrame::requestCancel, this,
          &PartitionFramePrivate::showMainFrame);

  connect(dynamic_disk_warning_frame_, &DynamicDiskWarningFrame::requestNext, this,
          &PartitionFramePrivate::showPrepareInstallFrame);

  connect(full_disk_partition_frame_, &FullDiskFrame::showDeviceInfomation,
          full_disk_encrypt_frame_, &Full_Disk_Encrypt_frame::onShowDeviceInfomation);

  connect(full_disk_partition_frame_, &FullDiskFrame::cryptoStateChanged,
          this, &PartitionFramePrivate::onFullDiskCryptoButtonClicked);
}

void PartitionFramePrivate::initUI() {

  advanced_partition_frame_ =
      new AdvancedPartitionFrame(advanced_delegate_, q_ptr);
  edit_partition_frame_ = new EditPartitionFrame(q_ptr->m_proxy, advanced_delegate_);
  full_disk_partition_frame_ = new FullDiskFrame(full_disk_delegate_, q_ptr);
  new_partition_frame_ = new NewPartitionFrame(q_ptr->m_proxy, advanced_delegate_);
  new_table_loading_frame_ = new NewTableLoadingFrame(q_ptr);
  new_table_warning_frame_ = new NewTableWarningFrame(q_ptr);
  partition_loading_frame_ = new PartitionLoadingFrame(q_ptr);
  partition_number_limitation_frame_ = new PartitionNumberLimitationFrame(q_ptr);
  partition_table_warning_frame_ = new PartitionTableWarningFrame(q_ptr);
  prepare_install_frame_ = new PrepareInstallFrame(q_ptr);
  select_bootloader_frame_ = new SelectBootloaderFrame(q_ptr);
  simple_partition_frame_ =
      new SimplePartitionFrame(simple_partition_delegate_, q_ptr);

  full_disk_encrypt_frame_ = new Full_Disk_Encrypt_frame(q_ptr->m_proxy, full_disk_delegate_);

  dynamic_disk_warning_frame_ = new DynamicDiskWarningFrame(q_ptr);

  title_label_ = new TitleLabel(tr("Select Installation Location"));
  comment_label_ = new CommentLabel(
      tr("Make sure you have backed up important data, then select the disk to install"));
  QHBoxLayout* comment_layout = new QHBoxLayout();
  comment_layout->setContentsMargins(0, 0, 0, 0);
  comment_layout->setSpacing(0);
  comment_layout->addWidget(comment_label_);

  m_buttonGroup = new DButtonBox(q_ptr);
  simple_frame_button_ = new DButtonBoxButton(tr("Simple"), q_ptr);
  simple_frame_button_->setMinimumWidth(86);
  advanced_frame_button_ = new DButtonBoxButton(tr("Advanced"), q_ptr);
  advanced_frame_button_->setMinimumWidth(86);
  full_disk_frame_button_ = new DButtonBoxButton(tr("Full Disk"), q_ptr);
  full_disk_frame_button_->setMinimumWidth(86);

  m_buttonGroup->setButtonList({advanced_frame_button_, full_disk_frame_button_}, true);
  m_buttonGroup->setVisible(true);

  QHBoxLayout* button_layout = new QHBoxLayout();
  button_layout->setContentsMargins(0, 0, 0, 0);
  button_layout->setSpacing(0);
  button_layout->addStretch();
  button_layout->addWidget(m_buttonGroup, 0, Qt::AlignCenter);
  button_layout->addStretch();

  partition_stacked_layout_ = new QStackedLayout();
  partition_stacked_layout_->setContentsMargins(0, 0, 0, 0);
  partition_stacked_layout_->setSpacing(0);

  partition_stacked_layout_->addWidget(simple_partition_frame_);
  partition_stacked_layout_->addWidget(advanced_partition_frame_);
  partition_stacked_layout_->addWidget(full_disk_partition_frame_);

  if (GetSettingsBool(kPartitionSkipSimplePartitionPage)) {
      simple_frame_button_->hide();
      simple_frame_button_->setChecked(false);
      simple_partition_frame_->hide();
      full_disk_frame_button_->setChecked(true);
      partition_stacked_layout_->setCurrentWidget(full_disk_partition_frame_);
  }
  else {
      simple_frame_button_->setChecked(true);
  }

  if (GetSettingsBool(kPartitionSkipFullDiskPartitionPage)) {
      full_disk_frame_button_->hide();
      full_disk_frame_button_->setChecked(false);
      full_disk_partition_frame_->hide();
  }

  if (GetSettingsBool(kPartitionSkipFullDiskPartitionPage) &&
          GetSettingsBool(kPartitionSkipSimplePartitionPage)) {
      advanced_frame_button_->setChecked(true);
      partition_stacked_layout_->setCurrentWidget(advanced_partition_frame_);
  }

  // NOTE(justforlxz): 0是简单分区，1是手动分区，2是全盘分区
  switch (GetSettingsInt(kPartitionDefaultButton)) {
      case 0:
          if (GetSettingsBool(kPartitionSkipSimplePartitionPage)) {
              simple_partition_frame_->show();
              simple_frame_button_->show();
          }
          simple_frame_button_->setChecked(true);
          partition_stacked_layout_->setCurrentWidget(simple_partition_frame_);
          break;
      case 1:
          advanced_frame_button_->setChecked(true);
          partition_stacked_layout_->setCurrentWidget(advanced_partition_frame_);
          break;
      case 2:
          if (GetSettingsBool(kPartitionSkipFullDiskPartitionPage)) {
              full_disk_partition_frame_->show();
              full_disk_frame_button_->show();
          }
          full_disk_frame_button_->setChecked(true);
          partition_stacked_layout_->setCurrentWidget(full_disk_partition_frame_);
          break;
      default: break;
  }

  QHBoxLayout* partition_stacked_wrapper_layout = new QHBoxLayout();
  partition_stacked_wrapper_layout->setContentsMargins(0, 0, 0, 0);
  partition_stacked_wrapper_layout->setSpacing(0);
  partition_stacked_wrapper_layout->addLayout(partition_stacked_layout_);

  // and advanced partition page.
  nextButton->setText(tr("Start installation"));
  QHBoxLayout* next_layout = new QHBoxLayout();
  next_layout->setContentsMargins(0, 0, 0, 0);
  next_layout->addWidget(nextButton);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(title_label_, 0, Qt::AlignHCenter);
  layout->addSpacing(kMainLayoutSpacing);
  layout->addLayout(comment_layout);
  layout->addSpacing(kMainLayoutSpacing);
  if ((!GetSettingsBool(kPartitionSkipSimplePartitionPage) ||
       !GetSettingsBool(kPartitionSkipFullDiskPartitionPage))) {
    // Add control button groups only if both simple mode and advanced mode
    // are enabled.
    layout->addLayout(button_layout);
  }
  layout->addSpacing(20 + kMainLayoutSpacing);
  layout->addLayout(partition_stacked_wrapper_layout);
  layout->addLayout(next_layout);

  main_frame_ = new QFrame();
  main_frame_->setContentsMargins(0, 0, 0, 0);
  main_frame_->setLayout(layout);

  main_layout_ = new QStackedLayout();
  main_layout_->setContentsMargins(0, 0, 0, 0);
  // Keep PartitionLoadingFrame is the first display page.
  main_layout_->addWidget(partition_loading_frame_);
  main_layout_->addWidget(main_frame_);
  main_layout_->addWidget(new_table_loading_frame_);
  main_layout_->addWidget(new_table_warning_frame_);
  main_layout_->addWidget(partition_number_limitation_frame_);
  main_layout_->addWidget(partition_table_warning_frame_);
  main_layout_->addWidget(prepare_install_frame_);
  main_layout_->addWidget(select_bootloader_frame_);
  main_layout_->addWidget(dynamic_disk_warning_frame_);

  centerLayout->addLayout(main_layout_);
  q_ptr->setContentsMargins(0, 0, 0, 0);
}

bool PartitionFramePrivate::isFullDiskPartitionMode() {
  return full_disk_frame_button_->isChecked();
}

bool PartitionFramePrivate::isSimplePartitionMode() {
  return simple_frame_button_->isChecked();
}

bool PartitionFramePrivate::isRawDevice(const QList<Device::Ptr> list) {
    for (const Device::Ptr device : list) {
        QProcess process;
        process.start("fdisk", QStringList() << "-l" << device->path);
        process.waitForFinished();
        const QString& result = process.readAllStandardOutput();

        if (result.isEmpty()) {
            continue;
        }

        QTextStream stream(result.toUtf8());
        QString     line;
        while (stream.readLineInto(&line)) {
            const QStringList& args = line.simplified().split(" ");

            if (args.length() < 2) {
              continue;
            }

            // SFS(42h): https://en.wikipedia.org/wiki/Partition_type
            if (args.last().contains("SFS") || args[args.length() - 2] == "42") {
                return true;
            }
        }
    }

    return false;
}

void PartitionFramePrivate::onButtonGroupToggled(QAbstractButton *button)
{
    if (button == full_disk_frame_button_){
        qDebug() << "on fulldisk button toggled";
        partition_stacked_layout_->setCurrentWidget(full_disk_partition_frame_);
    }
    else if (button == simple_frame_button_){
        qDebug() << "on simple button toggled";
        partition_stacked_layout_->setCurrentWidget(simple_partition_frame_);
    }
    else {
        qDebug() << "on advanced button toggled";
        // Refresh device list before showing advanced partition frame.
        // Because mount-point of partitions might have be updated.
        advanced_delegate_->refreshVisual();
        partition_stacked_layout_->setCurrentWidget(advanced_partition_frame_);
    }
}

void PartitionFramePrivate::onNextButtonClicked() {
  WriteFullDiskMode(isFullDiskPartitionMode());

  if (isSimplePartitionMode()) {
    // Validate simple partition frame.
    if (!simple_partition_frame_->validate()) {
      return;
    }
  } else if (isFullDiskPartitionMode()) {
    if (!full_disk_partition_frame_->validate()) {
      return;
    }
  } else {
    // Validate advanced partition frame.
    if (!advanced_partition_frame_->validate()) {
      return;
    }
  }

  // check disk is raw
  QList<Device::Ptr> device;
  if (isSimplePartitionMode()) {
     device << simple_partition_frame_->selectedDevice();
     dynamic_disk_warning_frame_->setWarningTip(tr("The target disk is dynamic, and your data may be lost if proceeding. Please make a backup of your important files first."));
  }
  else if (!isFullDiskPartitionMode()) {
    device = advanced_partition_frame_->getAllUsedDevice();
    dynamic_disk_warning_frame_->setWarningTip(tr("The target disk is dynamic which will be formatted if proceeding. Please make a backup of your important files first."));
  }

  if (!device.isEmpty() && isRawDevice(device)) {
    dynamic_disk_warning_frame_->setDevice(device);
    showDynamicDiskFrame();
    return;
  }

    showPrepareInstallFrame();
}

void PartitionFramePrivate::onFullDiskCryptoButtonClicked(bool encrypto)
{
    nextButton->setText(encrypto ? tr("Next") : tr("Start installation"));
}

void PartitionFramePrivate::onManualPartDone(bool ok, const DeviceList& devices) {
  if (ok) {
    // Write settings to file.
    if (isSimplePartitionMode()) {
      simple_partition_delegate_->onManualPartDone(devices);
    } else if (isFullDiskPartitionMode()) {
      full_disk_delegate_->onManualPartDone(devices);
    } else {
      advanced_delegate_->onManualPartDone(devices);
    }
  }

  emit q_ptr->manualPartDone(ok);
}

void PartitionFramePrivate::onPrepareInstallFrameFinished() {
    // First, update boot flag.
    bool found_boot;
    if (isSimplePartitionMode()) {
        found_boot = simple_partition_delegate_->setBootFlag();
    }
    else if (isFullDiskPartitionMode() && !full_disk_partition_frame_->isEncrypt()){
        found_boot = full_disk_delegate_->setBootFlag();
    }
    else {
        found_boot = advanced_delegate_->setBootFlag();
    }

    if (!found_boot && !isFullDiskPartitionMode()) {
        qCritical() << "No boot partition found, we shall never reach here!";
        return;
    }

    // Get operation list.
    OperationList operations;
    if (isSimplePartitionMode()) {
        operations = simple_partition_delegate_->operations();
    }
    else if (isFullDiskPartitionMode() && !full_disk_partition_frame_->isEncrypt()) {
        operations = full_disk_delegate_->operations();
    }
    else {
        operations = advanced_delegate_->operations();
    }

    // full disk encrypt operations is empty.
    if (isFullDiskPartitionMode() && full_disk_partition_frame_->isEncrypt()) {
        q_ptr->m_proxy->nextFrame();
    }
    else if (operations.isEmpty()) {
        qCritical() << "Operation list is empty";
        return;
    }
    else {
        partition_model_->manualPart(operations);
        q_ptr->m_proxy->nextFrame();
    }
}

void PartitionFramePrivate::showEditPartitionFrame(const Partition::Ptr partition) {
  edit_partition_frame_->setPartition(partition);
  q_ptr->m_proxy->showChindFrame(edit_partition_frame_);
}

void PartitionFramePrivate::showMainFrame() {
  main_layout_->setCurrentWidget(main_frame_);
}

void PartitionFramePrivate::showNewPartitionFrame(
    const Partition::Ptr partition) {
  new_partition_frame_->setPartition(partition);
  q_ptr->m_proxy->showChindFrame(new_partition_frame_);
}

void PartitionFramePrivate::showNewTableLoadingFrame() {
  main_layout_->setCurrentWidget(new_table_loading_frame_);
}

void PartitionFramePrivate::showNewTableWarningFrame(const QString& device_path) {
  DeviceList devices;
  if (isSimplePartitionMode()) {
    devices = simple_partition_delegate_->realDevices();
  } else if (isFullDiskPartitionMode()) {
    qCritical() << "Never show new table warning frame for simple disk frame";
    return;
  } else {
    devices = advanced_delegate_->realDevices();
  }

  const int device_index = DeviceIndex(devices, device_path);
  Q_ASSERT(device_index > -1);
  if (device_index == -1) {
    qCritical() << "Failed to find device at:" << device_path;
    return;
  }
  const Device::Ptr device = devices.at(device_index);
  const QString device_info = GetDeviceModelCapAndPath(device);

  new_table_warning_frame_->setDevicePath(device_path, device_info);
  main_layout_->setCurrentWidget(new_table_warning_frame_);
}

void PartitionFramePrivate::showPartitionNumberLimitationFrame() {
  main_layout_->setCurrentWidget(partition_number_limitation_frame_);
}

void PartitionFramePrivate::showPartitionTableWarningFrame(
    const QString& device_path) {
  partition_table_warning_frame_->setDevicePath(device_path);
  main_layout_->setCurrentWidget(partition_table_warning_frame_);
}

void PartitionFramePrivate::showSelectBootloaderFrame() {
    main_layout_->setCurrentWidget(select_bootloader_frame_);
}

void PartitionFramePrivate::showEncryptFrame()
{
    if (full_disk_partition_frame_->validate()) {
        if (!GetSettingsBool(KPartitionSkipFullCryptPage) && full_disk_partition_frame_->isEncrypt()) {
            q_ptr->m_proxy->showChindFrame(full_disk_encrypt_frame_);
        }
        else {
            q_ptr->autoPart();
            onPrepareInstallFrameFinished();
        }
    }
}

void PartitionFramePrivate::showDynamicDiskFrame() {
  main_layout_->setCurrentWidget(dynamic_disk_warning_frame_);
}

void PartitionFramePrivate::showPrepareInstallFrame()
{
    QStringList descriptions;
    if (isSimplePartitionMode()) {
      descriptions = simple_partition_delegate_->getOptDescriptions();
    }
    else if (isFullDiskPartitionMode()) {
        descriptions = full_disk_delegate_->getOptDescriptions();
    } else {
      descriptions = advanced_delegate_->getOptDescriptions();
    }

    qDebug() << "descriptions: " << descriptions;

    prepare_install_frame_->updateDescription(descriptions);
    main_layout_->setCurrentWidget(prepare_install_frame_);
}

}  // namespace installer
#include "partition_frame.moc"
