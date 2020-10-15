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

#include "ui/frames/inner/new_partition_frame.h"

#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QScrollArea>
#include <QScrollBar>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <DPushButton>

#include "base/file_util.h"
#include "sysinfo/proc_meminfo.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/frames/consts.h"
#include "ui/delegates/advanced_partition_delegate.h"
#include "ui/delegates/partition_util.h"
#include "ui/models/fs_model.h"
#include "ui/models/mount_point_model.h"
#include "ui/models/partition_type_model.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/partition_size_slider.h"
#include "ui/widgets/table_combo_box.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/di_scrollarea.h"
#include "ui/widgets/select_button.h"

DWIDGET_USE_NAMESPACE

namespace installer {

namespace {
// Minimum size of new partition is 100 Mib.
const qint64 kMinimumPartitionSize = 100 * kMebiByte;
const int kButtonwidth = 197;

const int kMainFrameWidth = 560;
const int kMainFrameHeight = 500;

const int kHintLabelWidth = 80;
const int kHintLabelInputWidgetSpacing = 10;
const int kInputWidgetWidth = 310;
}  // namespace

NewPartitionFrame::NewPartitionFrame(FrameProxyInterface* frameProxyInterface, AdvancedPartitionDelegate* delegate,
                                     QWidget* parent)
    : ChildFrameInterface(frameProxyInterface, parent),
      delegate_(delegate),
      partition_(),
      last_slider_value_(0) {
  this->setObjectName("new_partition_frame");

  this->initUI();
  this->initConnections();
}

void NewPartitionFrame::setPartition(const Partition::Ptr partition) {
  // Update partition information.
  partition_ = partition;

  if (fs_model_) {
      fs_model_->deleteLater();
  }
  fs_model_ = new FsModel(delegate_->getFsTypeList(), this);
  fs_box_->setModel(fs_model_);

  if (mount_point_model_) {
      mount_point_model_->deleteLater();
  }
  QStringList mountPointList = delegate_->getMountPoints();
  if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Install) {
      if (mountPointList.indexOf("/") < 0) {
          mountPointList.removeOne(kMountPointAuto);
      }
  }

  if (GetCurrentType() == OSType::Server) {
      mountPointList.removeOne(kMountPointAuto);
  }

  Device::Ptr device = delegate_->findDevice(partition_->device_path);

  for (Partition::Ptr parttition : device->partitions) {
      if (parttition->fs == FsType::LinuxSwap) {
           mountPointList.removeOne(kMountPointAuto);
      }

      if (parttition->mount_point == kMountPointRoot) {
           mountPointList.removeOne(kMountPointAuto);
      }
  }

  mount_point_model_ = new MountPointModel(mountPointList, this);
  mount_point_box_->setModel(mount_point_model_);


  const bool primary_ok = delegate_->canAddPrimary(partition);
  const bool logical_ok = delegate_->canAddLogical(partition);
  if (! (primary_ok || logical_ok)) {
    // If neither primary partition nor logical partition can be added,
    // returns immediately.
    // We shall never reach here.
    qCritical() << "No more partition available!";
    emit this->finished();
    return;
  }

  type_model_->setPrimaryVisible(primary_ok);
  type_model_->setLogicalVisible(logical_ok);
  if (GetSettingsBool(kPartitionPreferLogicalPartition)) {
    // Select logical partition first if available.
    if (logical_ok) {
      type_box_->setCurrentIndex(type_model_->getLogicalIndex());
    } else {
      type_box_->setCurrentIndex(type_model_->getPrimaryIndex());
    }
  } else {
    // Select primary partition first.
    if (primary_ok) {
      type_box_->setCurrentIndex(type_model_->getPrimaryIndex());
    } else {
      type_box_->setCurrentIndex(type_model_->getLogicalIndex());
    }
  }

  // Select align-start.
  alignment_box_->setCurrentIndex(0);

  // Select default fs type.
  const FsType default_fs = GetDefaultFsType();
  const int default_fs_index = fs_model_->indexOf(default_fs);
  fs_box_->setCurrentIndex(default_fs_index);

  // Select empty mount-point.
  const int mount_point_index = mount_point_model_->indexOf("");
  mount_point_box_->setCurrentIndex(mount_point_index);

  // Set value range of size_slider_
  last_slider_value_ = partition->getByteLength();
  size_slider_->setMaximum(last_slider_value_);
  size_slider_->setValue(last_slider_value_);
  size_slider_->setMinimum(kMinimumPartitionSize);
}

void NewPartitionFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(::QObject::tr("Create New Partition"));
    type_label_->setText(::QObject::tr("Type"));
    alignment_label_->setText(::QObject::tr("Location"));
    fs_label_->setText(::QObject::tr("File system"));
    mount_point_label_->setText(::QObject::tr("Mount point"));
    size_label_->setText(::QObject::tr("Size"));
    alignment_box_->clear();
    alignment_box_->addItems({::QObject::tr("Start"), ::QObject::tr("End")});
    cancel_button_->setText(::QObject::tr("Cancel"));
    create_button_->setText(::QObject::tr("Create"));

    if (m_close_button) {
        const int marginSize = this->layout()->margin();
        m_close_button->move(width() - m_close_button->width() - marginSize, marginSize);
        m_close_button->raise();
        m_close_button->show();
    }
  } else {
    QWidget::changeEvent(event);
  }
}

void NewPartitionFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 25, 25);
    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);

    return QWidget::paintEvent(event);
}

void NewPartitionFrame::initConnections() {
  connect(fs_box_,
          static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &NewPartitionFrame::onFsChanged);
  connect(mount_point_box_,
          static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &NewPartitionFrame::onMountPointChanged);

  connect(size_slider_, &PartitionSizeSlider::valueChanged,
          this, &NewPartitionFrame::onSizeSliderValueChanged);

  connect(cancel_button_, &QPushButton::clicked,
          this, &NewPartitionFrame::finished);
  connect(create_button_, &QPushButton::clicked,
          this, &NewPartitionFrame::onCreateButtonClicked);

  connect(m_close_button, &DImageButton::clicked, [this]{Q_EMIT cancel_button_->click();});
#ifdef QT_DEBUG
  connect(m_close_button, &DImageButton::clicked, [this]{qDebug() << "close button!";});
#endif // QT_DEBUG
}

void NewPartitionFrame::initUI() {
  title_label_ = new TitleLabel(::QObject::tr("Create New Partition"));

  type_label_ = new QLabel(::QObject::tr("Type"));
  type_label_->setObjectName("type_label");
  type_box_ = new TableComboBox();
  type_model_ = new PartitionTypeModel(type_box_);
  type_box_->setModel(type_model_);
  type_label_->setFixedWidth(kHintLabelWidth);
  type_box_->setFixedWidth(kInputWidgetWidth);

  QHBoxLayout* type_layout = new QHBoxLayout;
  type_layout->addStretch();
  type_layout->addWidget(type_label_, 0, Qt::AlignLeft);
  type_layout->addSpacing(kHintLabelInputWidgetSpacing);
  type_layout->addWidget(type_box_, 0, Qt::AlignRight);
  type_layout->addStretch();

  alignment_label_ = new QLabel(::QObject::tr("Location"));
  alignment_label_->setObjectName("alignment_label");
  alignment_box_ = new TableComboBox();
  alignment_box_->addItems({::QObject::tr("Start"), ::QObject::tr("End")});
  alignment_label_->setFixedWidth(kHintLabelWidth);
  alignment_box_->setFixedWidth(kInputWidgetWidth);

  QHBoxLayout* alignment_layout = new QHBoxLayout;
  alignment_layout->addStretch();
  alignment_layout->addWidget(alignment_label_, 0, Qt::AlignLeft);
  alignment_layout->addSpacing(kHintLabelInputWidgetSpacing);
  alignment_layout->addWidget(alignment_box_, 0, Qt::AlignRight);
  alignment_layout->addStretch();

  fs_label_ = new QLabel(::QObject::tr("File system"));
  fs_label_->setObjectName("fs_label");
  fs_label_->setFixedWidth(kHintLabelWidth);
  fs_model_ = new FsModel(delegate_->getFsTypeList(), this);
  fs_box_ = new TableComboBox();
  fs_box_->setModel(fs_model_);
  fs_box_->setFixedWidth(kInputWidgetWidth);

  QHBoxLayout* fs_layout = new QHBoxLayout;
  fs_layout->addStretch();
  fs_layout->addWidget(fs_label_, 0, Qt::AlignLeft);
  fs_layout->addSpacing(kHintLabelInputWidgetSpacing);
  fs_layout->addWidget(fs_box_, 0, Qt::AlignRight);
  fs_layout->addStretch();

  mount_point_label_ = new QLabel(::QObject::tr("Mount point"));
  mount_point_label_->setObjectName("mount_point_label");
  mount_point_box_ = new TableComboBox();
  mount_point_model_ = new MountPointModel(delegate_->getMountPoints(),
                                           mount_point_box_);
  mount_point_box_->setModel(mount_point_model_);
  mount_point_label_->setFixedWidth(kHintLabelWidth);
  mount_point_box_->setFixedWidth(kInputWidgetWidth);

  QHBoxLayout* mount_layout = new QHBoxLayout;
  mount_layout->addStretch();
  mount_layout->addWidget(mount_point_label_, 0, Qt::AlignLeft);
  mount_layout->addSpacing(kHintLabelInputWidgetSpacing);
  mount_layout->addWidget(mount_point_box_, 0, Qt::AlignRight);
  mount_layout->addStretch();

  size_label_ = new QLabel(::QObject::tr("Size"));
  size_label_->setObjectName("size_label");
  size_label_->setFrameShape(QFrame::NoFrame);
  size_slider_ = new PartitionSizeSlider();
  size_slider_->setFixedWidth(mount_point_box_->width());
  size_label_->setFixedWidth(kHintLabelWidth);
  size_slider_->setFixedWidth(kInputWidgetWidth);

  QHBoxLayout* size_layout = new QHBoxLayout;
  size_layout->addStretch();
  size_layout->addWidget(size_label_, 0, Qt::AlignLeft);
  size_layout->addSpacing(kHintLabelInputWidgetSpacing);
  size_layout->addWidget(size_slider_, 0, Qt::AlignRight);
  size_layout->addStretch();

  QVBoxLayout* content_layout = new QVBoxLayout();
  content_layout->setSpacing(20);
  if (!delegate_->m_islvm) {
      content_layout->addLayout(type_layout);
      content_layout->addLayout(alignment_layout);
  }
  content_layout->addLayout(fs_layout);
  content_layout->addLayout(mount_layout);
  content_layout->addLayout(size_layout);

  QFrame* content_frame = new QFrame();
  content_frame->setObjectName("content_frame");
  content_frame->setLayout(content_layout);

  cancel_button_ = new SelectButton();
  cancel_button_->setFixedWidth(kButtonwidth);
  cancel_button_->setFocusPolicy(Qt::TabFocus);
  create_button_ = new DSuggestButton();
  create_button_->setFixedWidth(kButtonwidth);
  create_button_->setFocusPolicy(Qt::TabFocus);

  m_auto_label = new QLabel();

  QHBoxLayout* bt_layout = new QHBoxLayout;
  bt_layout->addStretch();
  bt_layout->addWidget(cancel_button_, 0, Qt::AlignHCenter);
  bt_layout->addSpacing(20);
  bt_layout->addWidget(create_button_, 0, Qt::AlignHCenter);
  bt_layout->addStretch();

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addSpacing(30);
  layout->addStretch();
  layout->addWidget(title_label_, 0, Qt::AlignCenter);
  layout->addSpacing(kMainLayoutSpacing);

  layout->addWidget(content_frame, 0, Qt::AlignHCenter);
  layout->addSpacing(54);
  layout->addWidget(m_auto_label, 0, Qt::AlignHCenter);
  layout->addSpacing(8);
  layout->addLayout(bt_layout);
  layout->addSpacing(20);

  this->setLayout(layout);
  this->setContentsMargins(0, 0, 0, 0);
  setFixedSize(QSize(kMainFrameWidth, kMainFrameHeight));

  setupCloseButton();
}

void NewPartitionFrame::updateSlideSize() {
  const int fs_index = fs_box_->currentIndex();
  const FsType fs_type = fs_model_->getFs(fs_index);
  const int mp_index = mount_point_box_->currentIndex();
  const QString mount_point = mount_point_model_->getMountPoint(mp_index);
  const bool is_logical = type_model_->isLogical(type_box_->currentIndex());
  // If fs_type is special, no need to display mount-point box.
  const bool visible = IsMountPointSupported(fs_type);
  mount_point_label_->setVisible(visible);
  mount_point_box_->setVisible(visible);

  qint64 offset_logical = 0;
  if (is_logical) {
      offset_logical = kMebiByte;
  }

  if (fs_type == FsType::EFI) {
    m_auto_label->setText("");
    create_button_->setEnabled(true);
    // Set default size of EFI partition.
    // NOTE(xushaohua): partition size might be less than |default_size|.
    // Its value will also be checked in AdvancedPartitionFrame.
    const qint64 default_size = GetSettingsInt(kPartitionDefaultEFISpace) *
                                kMebiByte;
    const qint64 real_size = qMin(default_size, partition_->getByteLength());
    size_slider_->setMinimum(real_size + offset_logical);

    // Block size_slider_ from emitting signals.
    size_slider_->blockSignals(true);
    size_slider_->setValue(real_size);
    size_slider_->blockSignals(false);
  } else if (mount_point == kMountPointBoot) {
    m_auto_label->setText("");
    create_button_->setEnabled(true);
    // Set default size for /boot.
    // NOTE(xushaohua): partition size might be less than |default_size|.
    // Its value will also be checked in AdvancedPartitionFrame.
    const qint64 default_size = GetSettingsInt(kPartitionDefaultBootSpace) *
                                kMebiByte;
    const qint64 real_size = qMin(default_size, partition_->getByteLength());
    size_slider_->setMinimum(real_size + offset_logical);
    size_slider_->blockSignals(true);
    size_slider_->setValue(real_size);
    size_slider_->blockSignals(false);
  } else if (mount_point == kMountPointAuto
             && fs_type != FsType::EFI
             && fs_type != FsType::LVM2PV
             && fs_type != FsType::Recovery
             && fs_type != FsType::LinuxSwap) {
    // Set default size for auto mount.
    // NOTE(huzhengming): partition size might be less than |default_size|.
    // Its value will also be checked in AdvancedPartitionFrame.
    static const MemInfo mem_info = GetMemInfo();
    qint64 swapeSpace =  mem_info.mem_total * 2;
    if (mem_info.mem_total > 2 *  kGibiByte) {
        swapeSpace = mem_info.mem_total + 2 *  kGibiByte;
    }
    const int root_required = GetSettingsInt(kPartitionRootMiniSpace);
    const int efi_recommended = GetSettingsInt(kPartitionDefaultEFISpace);
    qint64 sumSapce = root_required * kGibiByte + swapeSpace;

    PartitionTableType table = delegate_->findDevice(partition_->device_path)->table;

    if (table == PartitionTableType::GPT && !partition_->is_lvm) {
        sumSapce += efi_recommended * kMebiByte;
    }

    size_slider_->setMinimum(0);
    size_slider_->blockSignals(true);
    size_slider_->setValue(0);
    size_slider_->blockSignals(false);
    sumSapce = sumSapce / kGibiByte;
    sumSapce = sumSapce  * kGibiByte;
    const qint64 default_size = sumSapce + kGibiByte;
    QString msg = "";
    if (default_size > size_slider_->value() * kMebiByte) {
        msg = ::QObject::tr("Unable to mount automatically, as it requires at least %1 GB")
              .arg(QString::number(default_size / kGibiByte));
       create_button_->setEnabled(false);
    } else {
        create_button_->setEnabled(true);
    }

    m_auto_label->setText(msg);
  }
  else if (fs_type == FsType::Recovery) {
      m_auto_label->setText("");
      create_button_->setEnabled(true);
      const qint64 default_size = GetSettingsInt(kRecoveryDefaultSize) * kGibiByte;
      const qint64 real_size    = qMin(default_size, partition_->getByteLength());
      size_slider_->setMinimum(real_size + offset_logical);

      // Block size_slider_ from emitting signals.
      size_slider_->blockSignals(true);
      size_slider_->setValue(real_size);
      size_slider_->blockSignals(false);
  }
  else {
    m_auto_label->setText("");
    create_button_->setEnabled(true);
    // Reset minimum value of size_slider_.
    size_slider_->setMinimum(kMinimumPartitionSize + offset_logical);

    // And set current value to last value specified by user.
    size_slider_->setValue(last_slider_value_);
  }
}

void NewPartitionFrame::onCreateButtonClicked() {
  const bool is_primary = type_model_->isPrimary(type_box_->currentIndex());
  const bool is_logical = type_model_->isLogical(type_box_->currentIndex());
  if (!is_primary && !is_logical) {
    // We shall never reach here.
    qCritical() << "Both primary and logical partition are not available!";
    emit this->finished();
    return;
  }
  const PartitionType partition_type = is_primary ? PartitionType::Normal :
                                                    PartitionType::Logical;
  const bool align_start = (alignment_box_->currentIndex() == 0);
  const FsType fs_type = fs_model_->getFs(fs_box_->currentIndex());
  QString mount_point;
  if (mount_point_box_->isVisible()) {
    // Set mount_point only if mount_point_box_ is visible.
    const int index = mount_point_box_->currentIndex();
    mount_point = mount_point_model_->getMountPoint(index);
  }
  // TODO(xushaohua): Calculate exact sectors
  const qint64 total_sectors = size_slider_->value() / partition_->sector_size;

  if (mount_point == kMountPointAuto
               && fs_type != FsType::EFI
               && fs_type != FsType::LVM2PV
               && fs_type != FsType::Recovery
               && fs_type != FsType::LinuxSwap) {
      WriteIsAutoMount(true);
      Device::Ptr device = delegate_->findDevice(partition_->device_path);
      PartitionTableType table = device->table;
      const int efi_recommended = GetSettingsInt(kPartitionDefaultEFISpace);
      qint64 total_sectors_auto = efi_recommended * kMebiByte / partition_->sector_size;
      static const MemInfo mem_info = GetMemInfo();
      qint64 swapeSpace =  mem_info.mem_total * 2;
      if (mem_info.mem_total > 2 *  kGibiByte) {
          swapeSpace = mem_info.mem_total + 2 *  kGibiByte;
      }

      if (align_start) {//从起点开始创建
          //创建EFI分区
          if (table == PartitionTableType::GPT && device->device_type == DeviceType::NormalDevice) {
              delegate_->createPartition(partition_, partition_type, align_start, FsType::EFI,
                                         "", total_sectors_auto);
          }

          //创建/分区
          partition_ = device->partitions.back();
          qint64 rootSapce = size_slider_->value() - swapeSpace;
          if (table == PartitionTableType::GPT && device->device_type == DeviceType::NormalDevice) {
              rootSapce = rootSapce - efi_recommended * kMebiByte;
          }
          total_sectors_auto = rootSapce / partition_->sector_size;
          delegate_->createPartition(partition_, partition_type, align_start, fs_type,
                                     kMountPointRoot, total_sectors_auto);

          //创建交换分区
          partition_ = device->partitions.back();
          total_sectors_auto = swapeSpace / partition_->sector_size;
          delegate_->createPartition(partition_, partition_type, align_start, FsType::LinuxSwap,
                                     "", total_sectors_auto);
      } else {//从终点开始创建
          //创建交换分区
          int indexPartition = device->partitions.indexOf(partition_);
          total_sectors_auto = swapeSpace / partition_->sector_size;
          delegate_->createPartition(partition_, partition_type, align_start, FsType::LinuxSwap,
                                     "", total_sectors_auto);

          //创建/分区
          partition_ = device->partitions.at(indexPartition);
          qint64 rootSapce = total_sectors * partition_->sector_size - swapeSpace;
          if (table == PartitionTableType::GPT && device->device_type == DeviceType::NormalDevice) {
              rootSapce = rootSapce - efi_recommended * kMebiByte;
          }
          total_sectors_auto = rootSapce / partition_->sector_size;
          delegate_->createPartition(partition_, partition_type, align_start, fs_type,
                                     kMountPointRoot, total_sectors_auto);
          //创建EFI
          if (table == PartitionTableType::GPT && device->device_type == DeviceType::NormalDevice) {
              partition_ = device->partitions.at(indexPartition);
              total_sectors_auto = efi_recommended * kMebiByte / partition_->sector_size;
              delegate_->createPartition(partition_, partition_type, align_start, FsType::EFI,
                                         "", total_sectors_auto);
          }

      }

  } else {
      delegate_->createPartition(partition_, partition_type, align_start, fs_type,
                                 mount_point, total_sectors);      
  }

  delegate_->refreshVisual();
  emit this->finished();
}

void NewPartitionFrame::onFsChanged(int index) {
  Q_UNUSED(index);
  this->updateSlideSize();
}

void NewPartitionFrame::onMountPointChanged(int index) {
  Q_UNUSED(index);  
  this->updateSlideSize();
}

void NewPartitionFrame::onSizeSliderValueChanged(qint64 size) {
  // Memorize new value setup by user.
  last_slider_value_ = size;
  const int mp_index = mount_point_box_->currentIndex();
  const QString mount_point = mount_point_model_->getMountPoint(mp_index);
  const FsType fs_type = fs_model_->getFs(fs_box_->currentIndex());
  if (mount_point == kMountPointAuto
               && fs_type != FsType::EFI
               && fs_type != FsType::LVM2PV
               && fs_type != FsType::Recovery
               && fs_type != FsType::LinuxSwap) {
    // Set default size for auto mount.
    // NOTE(huzhengming): partition size might be less than |default_size|.
    // Its value will also be checked in AdvancedPartitionFrame.
    static const MemInfo mem_info = GetMemInfo();
    qint64 swapeSpace =  mem_info.mem_total * 2;
    if (mem_info.mem_total > 2 *  kGibiByte) {
        swapeSpace = mem_info.mem_total + 2 *  kGibiByte;
    }
    const int root_required = GetSettingsInt(kPartitionRootMiniSpace);
    const int efi_recommended = GetSettingsInt(kPartitionDefaultEFISpace);
    qint64 sumSapce = root_required * kGibiByte + swapeSpace;

    PartitionTableType table = delegate_->findDevice(partition_->device_path)->table;

    if (table == PartitionTableType::GPT && !partition_->is_lvm) {
        sumSapce += efi_recommended * kMebiByte;
    }
    sumSapce = sumSapce / kGibiByte;
    sumSapce = sumSapce  * kGibiByte;
    const qint64 default_size = sumSapce + kGibiByte;
    QString msg = "";
    if (default_size > size) {
        msg = ::QObject::tr("Unable to mount automatically, as it requires at least %1 GB")
              .arg(QString::number(default_size / kGibiByte));
       create_button_->setEnabled(false);
    } else {
        create_button_->setEnabled(true);
    }

    m_auto_label->setText(msg);
  }
}

void NewPartitionFrame::setupCloseButton()
{
    // TODO: use titleBar implement.
    m_close_button = new DImageButton(this);
    m_close_button->setFocusPolicy(Qt::TabFocus);
    m_close_button->setFixedSize(40, 40);
    m_close_button->setNormalPic(":/images/close_normal.svg");
    m_close_button->setHoverPic(":/images/close_normal.svg");
    m_close_button->setPressPic(":/images/close_normal.svg");
}


}  // namespace installer
