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

#include "ui/frames_cli/inner/new_partition_frame.h"



#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/frames/consts.h"
#include "ui/delegates/advanced_partition_delegate.h"
#include "ui/delegates/partition_util.h"
#include "ui/models_cli/partition_type_model.h"

namespace installer {

namespace {
// Minimum size of new partition is 100 Mib.
const qint64 kMinimumPartitionSizeCli = 100 * kMebiByte;
}  // namespace

NewPartitionFrame::NewPartitionFrame(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, AdvancedPartitionDelegate* delegate)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      delegate_(delegate),
      partition_(),
      last_slider_value_(0) {
  this->setObjectName("new_partition_frame");
  m_isNew = false;
}

void NewPartitionFrame::setPartition(const Partition::Ptr partition) {
  // Update partition information.
  partition_ = partition;
  const QString device_path = partition->device_path;
  this->initUI();
  this->initConnections();

  FsTypeList fsList = delegate_->getFsTypeList();
  if (GetSettingsBool(kEnableRecoveryPartition)) {
      if (fsList.indexOf(FsType::Recovery) == -1) {
        fsList.append(FsType::Recovery);
      }
  }

  QStringList fsStringList;
  for (FsType fs : fsList) {
      fsStringList.append(GetFsTypeName(fs));
  }
  fs_box_->setList(fsStringList);

  QStringList mountPointStringList(delegate_->getMountPoints());
  if (mountPointStringList.at(0) == "") {
      mountPointStringList.removeFirst();
      mountPointStringList.insert(0, "unused");
  }
  mount_point_box_->setList(mountPointStringList);

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

  QStringList typeList;
  if (primary_ok) {
    typeList.append(::QObject::tr("Primary partition"));
  }

  if (logical_ok) {
      typeList.append(::QObject::tr("Logical partition"));
  }

  type_box_->setList(typeList);

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
  QStringList alignmentList;
  alignmentList.append(::QObject::tr("Start"));
  alignmentList.append(::QObject::tr("End"));
  alignment_box_->setList(alignmentList);
  alignment_box_->setCurrentIndex(0);

  // Select default fs type.
  const QString default_fs = GetFsTypeName(GetDefaultFsType());
  const int default_fs_index = fs_box_->getList().indexOf(default_fs);
  fs_box_->setCurrentIndex(default_fs_index);

  // Select empty mount-point.
  const int mount_point_index = mount_point_box_->getList().indexOf("unused");
  mount_point_box_->setCurrentIndex(mount_point_index);

  // Set value range of size_slider_
  last_slider_value_ = partition->getByteLength();  
  size_slider_->setText(QString::number(last_slider_value_ / kMebiByte));
}

void NewPartitionFrame::show()
{
    if(!m_isshow){
        NCursesWindowBase::show();
        m_isshow = true;
    }
}

void NewPartitionFrame::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}


void NewPartitionFrame::initConnections() {

  connect(fs_box_,
          static_cast<void(NcursesListView::*)(int)>(&NcursesListView::selectChanged),
          this, &NewPartitionFrame::onFsChanged);
  connect(mount_point_box_,
          static_cast<void(NcursesListView::*)(int)>(&NcursesListView::selectChanged),
          this, &NewPartitionFrame::onMountPointChanged);
  connect(cancel_button_, &NcursesButton::clicked,
          this, &NewPartitionFrame::finished);
  connect(create_button_, &NcursesButton::clicked,
          this, &NewPartitionFrame::onCreateButtonClicked);
}

void NewPartitionFrame::initUI() {
  QString strCreate = ::QObject::tr("Create");
  QString strCancel = ::QObject::tr("Back");
  int buttonHeight = 3;
  int buttonWidth = std::max(strCancel.length(), strCreate.length()) + 4;
  cancel_button_ = new NcursesButton(this, strCancel, buttonHeight, 14, begy() + height() - buttonHeight - 2, begx() + 5);

  title_label_ = new NcursesLabel(this, 1, 1, begy(), begx());
  title_label_->setFocusEnabled(false);
  title_label_->setText(::QObject::tr("Create New Partition"));

  type_label_ = new NcursesLabel(this, 1, 20, begy(), begx());
  type_label_->setFocusEnabled(false);
  type_label_->setText(::QObject::tr("Type"));
  type_box_ = new NcursesListView(this, height() - 10, 20, begy(), begx());
  type_model_ = new PartitionTypeModel(type_box_);

  alignment_label_ = new NcursesLabel(this, 1, 20, begy(), begx());
  alignment_label_->setFocusEnabled(false);
  alignment_label_->setText(::QObject::tr("Location"));
  alignment_box_ = new NcursesListView(this, height() - 10, 20, begy(), begx());

  fs_label_ = new NcursesLabel(this, 1, 2, begy(), begx());
  fs_label_->setFocusEnabled(false);
  fs_label_->setText(::QObject::tr("File system"));
  fs_box_ = new NcursesListView(this, height() - 10, 20, begy(), begx());
  fs_box_->setFocus(true);

  mount_point_label_ = new NcursesLabel(this, 1, 20, begy(), begx());
  mount_point_label_->setFocusEnabled(false);
  mount_point_label_->setText(::QObject::tr("Mount point"));
  mount_point_box_ = new NcursesListView(this, height() - 10, 20, begy(), begx());

  size_label_ = new NcursesLabel(this, 1, 20, begy(), begx());
  size_label_->setFocusEnabled(false);
  size_label_->setText(::QObject::tr("Size"));
  size_slider_ = new NCursesLineEdit(this, 1, 20, begy(), begx());
  size_slider_->setIsNumber(true);

  create_button_ = new NcursesButton(this, strCreate, buttonHeight, 14, begy() + height() - buttonHeight - 2, begx() + width() - buttonWidth - 13);

  cancel_button_->drawShadow(true);
  create_button_->drawShadow(true);

  cancel_button_->box();
  create_button_->box();


  this->setFocus(true);

  m_showChild.push_back(type_box_);
  m_showChild.push_back(alignment_box_);
  m_showChild.push_back(fs_box_);
  m_showChild.push_back(mount_point_box_);
  m_showChild.push_back(size_slider_);
  m_showChild.push_back(cancel_button_);
  m_showChild.push_back(create_button_);

}

void NewPartitionFrame::updateTs()
{
    type_label_->setText(::QObject::tr("Type"));
    alignment_label_->setText(::QObject::tr("Location"));
    fs_label_->setText(::QObject::tr("File system"));
    mount_point_label_->setText(::QObject::tr("Mount point"));
    size_label_->setText(::QObject::tr("Size") + "(MB)");
    cancel_button_->setText(::QObject::tr("Back"));
    create_button_->setText(::QObject::tr("Create"));
    layout();
}

void NewPartitionFrame::layout()
{
    title_label_->adjustSizeByContext();
    title_label_->mvwin(begy(), begx() + (width() - title_label_->width()) / 2);

    type_label_->adjustSizeByContext();
    type_label_->mvwin(begy() + 1, begx() + 1);
    type_box_->adjustSizeByContext();
    type_box_->mvwin(begy() + type_label_->height() + 1, begx() + 1);

    int testwidth = 20;

    alignment_label_->adjustSizeByContext();
    alignment_label_->mvwin(begy() + 1, begx() + testwidth + 1);
    alignment_box_->adjustSizeByContext();
    alignment_box_->mvwin(begy() + type_label_->height() + 1, begx() + testwidth + 1);

    testwidth += 20;
    fs_label_->adjustSizeByContext();
    fs_label_->mvwin(begy() + 1, begx() + testwidth + 1);
    fs_box_->adjustSizeByContext();
    fs_box_->mvwin(begy() + type_label_->height() + 1, begx() + testwidth + 1);

    testwidth += 20;

    mount_point_label_->adjustSizeByContext();
    mount_point_label_->mvwin(begy() + 1, begx() + testwidth + 1);
    mount_point_box_->adjustSizeByContext();
    mount_point_box_->mvwin(begy() + type_label_->height() + 1, begx() + testwidth + 1);


    testwidth += 20;
    size_label_->adjustSizeByContext();
    size_label_->mvwin(begy() + 1, begx() + testwidth + 1);
    size_slider_->adjustSizeByContext();
    size_slider_->mvwin(begy() + type_label_->height() + 1, begx() + testwidth + 1);

//    cancel_button_->adjustSizeByContext();
//    cancel_button_->mvwin(begy() + height() -3 , begx() + (width() - title_label_->width()) / 2 - 20);

//    create_button_->adjustSizeByContext();
//    create_button_->mvwin(begy() + height() -3 , begx() + (width() - title_label_->width()) / 2 + 20);

    NCursesWindowBase::show();
}

void NewPartitionFrame::onKeyPress(int keycode)
{
    switch (keycode) {
        case KEY_TAB: break;
        case KEY_RIGHT:
        case KEY_LEFT:
        QVector<NCursesWindowBase* > showChild;
        for(NCursesWindowBase* child : m_showChild) {
            if(!child->hidden()) {
                showChild.append(child);
            }
        }

        for (NCursesWindowBase* child : showChild) {
            if(child->isOnFoucs()) {
                int size = showChild.size();
                int index = showChild.indexOf(child);

                int offset = 1;
                if (keycode == KEY_LEFT) {
                    if (index == 0) {
                        offset = size -1;
                    } else {
                        offset = -1;
                    }
                }

                int nextIndex = ( index + offset) % size;
                NCursesWindowBase* nextchild = showChild.at(nextIndex);

                if(nextchild != cancel_button_) {
                    cancel_button_->setFocus(false);
                }

                if(nextchild != create_button_) {
                    create_button_->setFocus(false);
                }

                if(nextchild != size_slider_) {
                    size_slider_->setFocus(false);
                }

                nextchild->setFocus(true);
                return ;
            }
        }
        break;
    }
}

void NewPartitionFrame::keyPresseEvent(int keycode)
{
   //if (KEY_TAB == keycode) return ;
   if(!m_isshow) {
        if(m_currentchoicetype != -1){
            emit keyEventTrigerSignal(keycode);
        }
        return;
    } else {
        if(m_currentchoicetype != -1){
            emit keyEventTrigerSignal(keycode);
        } else {
            FrameInterfacePrivate::keyEventTriger(keycode);
            emit keyEventTrigerSignal(keycode);
        }
    }
}

void NewPartitionFrame::updateSlideSize() {
  const int fs_index = fs_box_->getCurrentIndex();
  const FsType fs_type = GetFsTypeByName(fs_box_->getCurrenItem());
  const int mp_index = mount_point_box_->getCurrentIndex();
  const QString mount_point = mount_point_box_->getCurrenItem();

  // If fs_type is special, no need to display mount-point box.
  const bool visible = IsMountPointSupported(fs_type);
  if (visible) {
      mount_point_label_->show();
      mount_point_box_->adjustSizeByContext();
      mount_point_box_->show();
  } else {
      mount_point_label_->hide();
      mount_point_box_->hide();
  }

  if (fs_type == FsType::EFI) {
    // Set default size of EFI partition.
    // NOTE(xushaohua): partition size might be less than |default_size|.
    // Its value will also be checked in AdvancedPartitionFrame.
    const qint64 default_size = GetSettingsInt(kPartitionDefaultEFISpace) *
                                kMebiByte;
    const qint64 real_size = qMin(default_size, partition_->getByteLength());
    size_slider_->setText(QString::number(real_size / kMebiByte));
    size_slider_->blockSignals(false);   
  } else if (mount_point == kMountPointBoot) {
    // Set default size for /boot.
    // NOTE(xushaohua): partition size might be less than |default_size|.
    // Its value will also be checked in AdvancedPartitionFrame.
    const qint64 default_size = GetSettingsInt(kPartitionDefaultBootSpace) *
                                kMebiByte;
    const qint64 real_size = qMin(default_size, partition_->getByteLength());  
    size_slider_->setText(QString::number(real_size / kMebiByte));
    size_slider_->blockSignals(false);
  }
  else if (fs_type == FsType::Recovery) {
      const qint64 default_size = GetSettingsInt(kRecoveryDefaultSize) * kGibiByte;
      const qint64 real_size    = qMin(default_size, partition_->getByteLength());
      size_slider_->setText(QString::number(real_size / kMebiByte));

      // Block size_slider_ from emitting signals.
      size_slider_->blockSignals(true);
      size_slider_->setText(QString::number(real_size / kMebiByte));
      size_slider_->blockSignals(false);
  }
  else {
    // Reset minimum value of size_slider_.    
     size_slider_->setText(QString::number(kMinimumPartitionSizeCli));
    // And set current value to last value specified by user.
     size_slider_->setText(QString::number(last_slider_value_ / kMebiByte));
  }
}

void NewPartitionFrame::onCreateButtonClicked() {
  const bool is_primary = type_model_->isPrimary(type_box_->getCurrentIndex());
  const bool is_logical = type_model_->isLogical(type_box_->getCurrentIndex());
  if (!is_primary && !is_logical) {
    // We shall never reach here.
    qCritical() << "Both primary and logical partition are not available!";
    emit this->finished();
    return;
  }
  const PartitionType partition_type = is_primary ? PartitionType::Normal :
                                                    PartitionType::Logical;
  const bool align_start = (alignment_box_->getCurrentIndex() == 0);
  const FsType fs_type = GetFsTypeByName(fs_box_->getCurrenItem());
  QString mount_point;
  if (IsMountPointSupported(fs_type)) {
    // Set mount_point only if mount_point_box_ is visible.
    mount_point = mount_point_box_->getCurrenItem();
    if (mount_point == "unused") {
        mount_point = "";
    }
  }
  // TODO(xushaohua): Calculate exact sectors
  const qint64 total_sectors = size_slider_->text().toUInt() * kMebiByte / partition_->sector_size;

  delegate_->createPartition(partition_, partition_type, align_start, fs_type,
                             mount_point, total_sectors);
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
}

}  // namespace installer
