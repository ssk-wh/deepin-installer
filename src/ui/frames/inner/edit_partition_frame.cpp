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

#include "ui/frames/inner/edit_partition_frame.h"

#include <QCheckBox>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <DHorizontalLine>
#include <QPainterPath>

#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/frames/consts.h"
#include "ui/delegates/advanced_partition_delegate.h"
#include "ui/delegates/partition_util.h"
#include "ui/models/fs_model.h"
#include "ui/models/mount_point_model.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/rounded_progress_bar.h"
#include "ui/widgets/table_combo_box.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/select_button.h"

DWIDGET_USE_NAMESPACE

namespace installer {

namespace {

const int kMainFrameWidth = 560;
const int kMainFrameHeight = 500;

const int kProgressBarWidth = 240;
const int KLableWidth = 86;
const int kComboxWidth = 310;

const int kButtonWidth = 240;
const int kButtonHeight = 36;

// Check whether partition with |mount_point| should be formatted
// compulsively.
bool IsInFormattedMountPointList(const QString& mount_point) {
  const QStringList list(GetSettingsStringList(kPartitionFormattedMountPoints));
  return list.contains(mount_point);
}

}  // namespace

EditPartitionFrame::EditPartitionFrame(FrameProxyInterface* frameProxyInterface, AdvancedPartitionDelegate* delegate,
                                       QWidget* parent)
    : ChildFrameInterface(frameProxyInterface, parent)
    , delegate_(delegate)
    , partition_()
{
  this->setObjectName("edit_partition_frame");

  this->initUI();
  this->initConnections();
}

void EditPartitionFrame::setPartition(const Partition::Ptr partition) {
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

  QStringList mountList = delegate_->getMountPoints();
  if (mountList.back() == kMountPointAuto) {
      mountList.pop_back();
  }
  mount_point_model_ = new MountPointModel(mountList, this);
  mount_point_box_->setModel(mount_point_model_);

  os_label_->setPixmap(QPixmap(GetOsTypeLargeIcon(partition->os)));
  name_label_->setText(GetPartitionLabelAndPath(partition));
  usage_label_->setText(GetPartitionUsage(partition));
  usage_bar_->setValue(GetPartitionUsageValue(partition));

  // Update fs list.
  fs_model_->setShowUnknown(partition->fs == FsType::Unknown);
  fs_model_->setShowRecovery(GetSettingsBool(kEnableRecoveryPartition));

  // Reset fs index.
  int fs_index = fs_model_->indexOf(partition->fs);
  if (fs_index == -1) {
    // Get index of unused filesystem in filesystem list.
    fs_index = fs_model_->indexOf(FsType::Empty);
  }
  // If partition fs type not in current fs_list, select nothing.
  fs_box_->setCurrentIndex(fs_index);

  // Reset mount point box. mount_point might be empty.
  const int mp_index = mount_point_model_->indexOf(partition->mount_point);
  mount_point_box_->setCurrentIndex(mp_index);

  updateFormatBoxState();
}

void EditPartitionFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(::QObject::tr("Edit Disk"));
    fs_label_->setText(::QObject::tr("File system"));
    mount_point_label_->setText(::QObject::tr("Mount point"));
    format_check_box_->setText(::QObject::tr("Format the partition"));
    cancel_button_->setText(::QObject::tr("Cancel"));
    ok_button_->setText(::QObject::tr("Confirm"));

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

void EditPartitionFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 25, 25);
    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);

    return QWidget::paintEvent(event);
}

void EditPartitionFrame::forceFormat(bool force) {
  format_check_box_->setChecked(force);
  format_check_box_->setEnabled(!force);
}

void EditPartitionFrame::updateFormatBoxState() {
  const FsType fs_type = fs_model_->getFs(fs_box_->currentIndex());
  const Partition::Ptr real_partition(delegate_->getRealPartition(partition_));
  const int mp_index = mount_point_box_->currentIndex();
  QString mount_point;

  // Read mount point only if current filesystem supports mount-point.
  if (IsMountPointSupported(fs_type)) {
    mount_point = mount_point_model_->getMountPoint(mp_index);
  }

  // If current partition type is PartitionType::Unallocated,
  // real_partition is nullptr, need format.
  // But if status is PartitionStatus::New, will always format
  bool format = (real_partition.isNull() || partition_->status == PartitionStatus::New);


  // NOTE: maybe need check partition->path.isEmpty()
  if (!format) {
      // Format this partition compulsively if its fs type changed.
      format = (fs_type != real_partition->fs);
  }

  // Format this partition if mount-point is in formatted-mount-point list.
  format |= IsInFormattedMountPointList(mount_point);
  this->forceFormat(format);

  // If it is linux-swap, empty or known, hide format box.
  bool invisible = (fs_type == FsType::LinuxSwap ||
      fs_type == FsType::Empty ||
      fs_type == FsType::Unknown);
  format_check_box_->setVisible(!invisible);
}

void EditPartitionFrame::initConnections() {
  connect(fs_box_,
          static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &EditPartitionFrame::onFsChanged);
  connect(mount_point_box_,
          static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &EditPartitionFrame::onMountPointChanged);

  // Does nothing when cancel-button is clicked.
  connect(cancel_button_, &QPushButton::clicked,
          this, &EditPartitionFrame::finished);
  connect(ok_button_, &QPushButton::clicked,
          this, &EditPartitionFrame::onOkButtonClicked);

  connect(m_close_button, &DImageButton::clicked, [this]{Q_EMIT cancel_button_->click();});
#ifdef QT_DEBUG
  connect(m_close_button, &DImageButton::clicked, [this]{qDebug() << "close button!";});
#endif // QT_DEBUG
}

void EditPartitionFrame::initUI() {
  title_label_ = new TitleLabel(::QObject::tr("Edit Disk"));
  os_label_ = new QLabel();
  os_label_->setObjectName("os_label");
  name_label_ = new QLabel();
  name_label_->setObjectName("name_label");
  usage_label_ = new QLabel();
  usage_label_->setObjectName("usage_label");

  QHBoxLayout* name_layout = new QHBoxLayout();
  name_layout->setContentsMargins(0, 0, 0, 0);
  name_layout->setSpacing(0);
  name_layout->addWidget(name_label_);
  name_layout->addStretch();
  name_layout->addWidget(usage_label_);

  QFrame* name_frame = new QFrame();
  name_frame->setObjectName("name_frame");
  name_frame->setContentsMargins(0, 0, 0, 0);
  name_frame->setLayout(name_layout);
  name_frame->setFixedWidth(kProgressBarWidth);

  usage_bar_ = new RoundedProgressBar();
  usage_bar_->setFixedSize(kProgressBarWidth, 8);

  QVBoxLayout* bar_layout = new QVBoxLayout;
  bar_layout->addWidget(name_frame, 0, Qt::AlignLeft);
  bar_layout->addSpacing(5);
  bar_layout->addWidget(usage_bar_, 0, Qt::AlignLeft);

  QFrame* bar_frame = new QFrame;
  bar_frame->setLayout(bar_layout);

  QLabel* separator_label = new QLabel();
  separator_label->setObjectName("separator_label");

  DHorizontalLine* dHerticalLine = new DHorizontalLine;
  QHBoxLayout* line_layout = new QHBoxLayout;
  line_layout->addSpacing(30);
  line_layout->addWidget(dHerticalLine);
  line_layout->addSpacing(30);

  mount_point_label_ = new QLabel(::QObject::tr("Mount point"));
  mount_point_label_->setObjectName("mount_point_label");
  mount_point_label_->setFixedWidth(KLableWidth);

  fs_label_ = new QLabel(::QObject::tr("File system"));
  fs_label_->setObjectName("fs_label");
  fs_label_->setFixedWidth(KLableWidth);
  fs_model_ = new FsModel(delegate_->getFsTypeList(), this);
  fs_box_ = new TableComboBox();
  fs_box_->setObjectName("fs_box");
  fs_box_->setModel(fs_model_);
  fs_box_->setFixedWidth(kComboxWidth);

  mount_point_box_ = new TableComboBox();
  mount_point_box_->setObjectName("mount_point_box");
  mount_point_model_ = new MountPointModel(delegate_->getMountPoints(), this);
  mount_point_box_->setModel(mount_point_model_);

  format_check_box_ = new QCheckBox();
  format_check_box_->setObjectName("format_check_box");
  format_check_box_->setText(::QObject::tr("Format the partition"));
  format_check_box_->setFocusPolicy(Qt::TabFocus);

  QHBoxLayout* fs_layout =  new QHBoxLayout;
  fs_layout->addStretch();
  fs_layout->addWidget(fs_label_, 0, Qt::AlignLeft);
  fs_layout->addSpacing(15);
  fs_layout->addWidget(fs_box_);
  fs_layout->addStretch();

  QHBoxLayout* mount_layout = new QHBoxLayout;
  mount_layout->addStretch();
  mount_layout->addWidget(mount_point_label_, 0, Qt::AlignLeft);
  mount_layout->addSpacing(15);
  mount_layout->addWidget(mount_point_box_);
  mount_layout->addStretch();

  cancel_button_ = new SelectButton();
  cancel_button_->setFixedSize(QSize(kButtonWidth, kButtonHeight));
  cancel_button_->setFocusPolicy(Qt::TabFocus);
  ok_button_ = new DSuggestButton();
  ok_button_->setFixedSize(QSize(kButtonWidth, kButtonHeight));
  ok_button_->setFocusPolicy(Qt::TabFocus);

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(0);
  buttonLayout->addWidget(cancel_button_, 0, Qt::AlignHCenter | Qt::AlignLeft);
  buttonLayout->addSpacing(10);
  buttonLayout->addWidget(ok_button_, 0, Qt::AlignHCenter | Qt::AlignRight);
  QWidget *buttonWrapWidget = new QWidget;
  buttonWrapWidget->setContentsMargins(0, 0, 0, 0);
  buttonWrapWidget->setLayout(buttonLayout);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addSpacing(kMainLayoutSpacing);
  layout->addWidget(title_label_, 0, Qt::AlignHCenter);
  layout->addWidget(os_label_, 0, Qt::AlignHCenter);
  layout->addSpacing(20);
  layout->addWidget(bar_frame, 0, Qt::AlignHCenter);
  layout->addSpacing(10);
  layout->addLayout(line_layout);
  layout->addSpacing(10);
  layout->addLayout(fs_layout);
  layout->addSpacing(kMainLayoutSpacing);
  layout->addLayout(mount_layout);
  layout->addStretch();
  layout->addWidget(format_check_box_, 0, Qt::AlignCenter);
  layout->addSpacing(kMainLayoutSpacing);
  layout->addWidget(buttonWrapWidget, 0, Qt::AlignCenter);
  layout->addSpacing(10);

  setFixedSize(QSize(kMainFrameWidth, kMainFrameHeight));
  setLayout(layout);
  setContentsMargins(0, 0, 0, 0);

  setupCloseButton();
}

void EditPartitionFrame::onFsChanged(int index) {
  // Hide mount point box if current selected filesystem does not need
  // mount point.
  const FsType fs_type = fs_model_->getFs(index);
  const bool visible = IsMountPointSupported(fs_type);
  mount_point_label_->setVisible(visible);
  mount_point_box_->setVisible(visible);

  this->updateFormatBoxState();
}

void EditPartitionFrame::onMountPointChanged(int index) {
  const QString mount_point = mount_point_model_->getMountPoint(index);
  const FsType fs_type = fs_model_->getFs(fs_box_->currentIndex());
  const FsType default_fs = GetDefaultFsType();
  const int default_fs_index = fs_model_->indexOf(default_fs);

  // If mount point is not empty, partition fs shall be not empty and can not
  // be linux-swap or efi.
  if (!mount_point.isEmpty()) {
    // If fs is empty, select default fs.
    if (fs_type == FsType::Empty ||
        fs_type == FsType::LinuxSwap ||
        fs_type == FsType::EFI) {
      fs_box_->setCurrentIndex(default_fs_index);
    }
  }

  this->updateFormatBoxState();
}

void EditPartitionFrame::onOkButtonClicked() {
  const FsType fs_type = fs_model_->getFs(fs_box_->currentIndex());
  QString mount_point;

  // Read mount point only if current filesystem supports mount-point.
  if (IsMountPointSupported(fs_type)) {
    const int index = mount_point_box_->currentIndex();
    mount_point = mount_point_model_->getMountPoint(index);
  }

  if (format_check_box_->isChecked()) {
    // Create an OperationFormat object.
    delegate_->formatPartition(partition_, fs_type, mount_point);
  } else {
    // If FormatOperation is reverted.
    if (partition_->status == PartitionStatus::Format) {
      delegate_->unFormatPartition(partition_);
      // Reset partition status.
      partition_->status = PartitionStatus::Real;
    }

    // Create an OperationMountPoint object.
    if (partition_->mount_point != mount_point) {
      delegate_->updateMountPoint(partition_, mount_point);
    }
  }
  delegate_->refreshVisual();

  removeOsProberDataByPath(partition_->path);

  emit this->finished();
}

void EditPartitionFrame::setupCloseButton()
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
