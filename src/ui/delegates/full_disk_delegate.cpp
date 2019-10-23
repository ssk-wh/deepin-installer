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

#include "ui/delegates/full_disk_delegate.h"

#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/partition_util.h"

#include <sys/sysinfo.h>
#include <math.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace installer {

namespace {

const char kLinuxSwapMountPoint[] = "linux-swap";
const char kUEFIPartitionLabel[] = "efi";
const char kFullDiskPolicyRootb[] = "rootb";
const char kFullDiskPolicyUsageRootSize[] = "root-size";
const char kPartitionFullDiskRootPartitionUsage[] = "partition_full_disk_root_partition_usage";

}  // namespace

FullDiskDelegate::FullDiskDelegate(QObject* parent)
    : QObject(parent),
      real_devices_(),
      virtual_devices_(),
      bootloader_path_(),
      operations_(),
      selected_partition_(),
      primaryPartitionLength(0) {
    this->setObjectName("full_disk_delegate");
}

Device::Ptr FullDiskDelegate::fullInstallScheme(Device::Ptr device) const
{
    Device::Ptr fake(new Device());

    for (auto dev : selected_devices) {
        if (dev->path != device->path) {
            continue;
        }

        fake = Device::Ptr(new Device(*dev));
        fake->partitions.clear();

        for (auto partition : dev->partitions) {
            if (partition->end_sector < 0
             || partition->start_sector < 0
             || partition->type == PartitionType::Extended) {
                continue;
            }
            Partition::Ptr p(new Partition(*partition));
            p->type = PartitionType::Unallocated;
            p->length = p->end_sector - p->start_sector + 1;
            fake->partitions.append(p);
        }
        std::sort(fake->partitions.begin(), fake->partitions.end(), [=] (Partition::Ptr partition1, Partition::Ptr partition2) {
            return partition1->start_sector < partition2->start_sector;
        });
        return fake;
    }
    return fake;
}

bool FullDiskDelegate::canAddLogical(const Partition::Ptr partition) const {
  const int index = DeviceIndex(virtual_devices_, partition->device_path);
  if (index == -1) {
    qCritical() << "getSupportedPartitionType() no device found at:"
                << partition->device_path;
    return false;
  }
  const Device::Ptr device = virtual_devices_.at(index);

  // If partition table is empty, always returns false.
  // Thus, at least one primary partition shall be created.
  if (device->table == PartitionTableType::Empty) {
    return false;
  }

  // Ignores gpt table.
  if (device->table != PartitionTableType::MsDos) {
    return false;
  }

  bool logical_ok = true;
  const int ext_index = ExtendedPartitionIndex(device->partitions);
  if (ext_index == -1) {
    // No extended partition found, so check a new primary partition is
    // available or not.
    if (GetPrimaryPartitions(device->partitions).length() >= device->max_prims) {
      logical_ok = false;
    }
  } else {
    // Check whether there is primary partition between |partition| and
    // extended partition->
    const Partition::Ptr ext_partition = device->partitions.at(ext_index);
    const PartitionList prim_partitions = GetPrimaryPartitions(
        device->partitions);
    if (partition->end_sector < ext_partition->start_sector) {
      for (const Partition::Ptr prim_partition : prim_partitions) {
        if (prim_partition->end_sector > partition->start_sector &&
            prim_partition->start_sector < ext_partition->start_sector) {
          logical_ok = false;
        }
      }
    } else if (partition->start_sector > ext_partition->end_sector) {
      for (const Partition::Ptr prim_partition : prim_partitions) {
        if (prim_partition->end_sector < partition->start_sector &&
            prim_partition->start_sector > ext_partition->end_sector) {
          logical_ok =false;
        }
      }
    }
  }
  return logical_ok;
}

bool FullDiskDelegate::canAddPrimary(const Partition::Ptr partition) const {
  const int index = DeviceIndex(virtual_devices_, partition->device_path);
  if (index == -1) {
    qCritical() << "getSupportedPartitionType() no device found at:"
                << partition->device_path;
    return false;
  }
  const Device::Ptr device = virtual_devices_.at(index);

  // If partition table is empty, always returns true.
  if (device->table == PartitionTableType::Empty) {
    return true;
  }

  const PartitionList prim_partitions = GetPrimaryPartitions(device->partitions);
  const PartitionList logical_partitions =
      GetLogicalPartitions(device->partitions);

  // Check primary type
  bool primary_ok = true;
  if (prim_partitions.length() >= device->max_prims) {
    primary_ok = false;
  } else {
    // Check whether |partition| is between two logical partitions.
    bool has_logical_before = false;
    bool has_logical_after = false;
    for (const Partition::Ptr logical_partition : logical_partitions) {
      if (logical_partition->start_sector < partition->start_sector) {
        has_logical_before = true;
      }
      if (logical_partition->end_sector > partition->end_sector) {
        has_logical_after = true;
      }
    }
    if (has_logical_after && has_logical_before) {
      primary_ok = false;
    }
  }

  return primary_ok;
}

QStringList FullDiskDelegate::getOptDescriptions() const {
  QStringList descriptions;
  for (const Operation& operation : operations_) {
    descriptions.append(operation.description());
  }

  return descriptions;
}

bool FullDiskDelegate::isMBRPreferred() const {
  return IsMBRPreferred(real_devices_);
}

bool FullDiskDelegate::isPartitionTableMatch(
    const QString& device_path) const {
  return IsPartitionTableMatch(real_devices_, device_path);
}

void FullDiskDelegate::selectPartition(const Partition::Ptr partition) {
  selected_partition_ = partition;
}

bool FullDiskDelegate::setBootFlag() {
  bool found_boot = false;

  // First check new EFI partition->
  for (Operation& operation : operations_) {
      if (operation.type == OperationType::NewPartTable) continue;
      if (operation.new_partition->fs == FsType::EFI) {
          operation.new_partition->flags.append(PartitionFlag::Boot);
          operation.new_partition->flags.append(PartitionFlag::ESP);
          found_boot = true;
      }
  }

  // Check existing EFI partition->
  for (const Device::Ptr device : virtual_devices_) {
    for (const Partition::Ptr partition : device->partitions) {
      if (partition->fs == FsType::EFI) {
        return true;
      }
    }
  }

  // Check /boot partition->
  if (!found_boot) {
      for (Operation& operation : operations_) {
          if (operation.type == OperationType::NewPartTable) continue;
          if (operation.new_partition->mount_point == kMountPointBoot) {
              operation.new_partition->flags.append(PartitionFlag::Boot);
              found_boot = true;
          }
      }
  }

  // At last, check / partition->
  if (!found_boot) {
      for (Operation& operation : operations_) {
          if (operation.type == OperationType::NewPartTable) continue;
          if (operation.new_partition->mount_point == kMountPointRoot) {
              operation.new_partition->flags.append(PartitionFlag::Boot);
              found_boot = true;
          }
      }
  }

  return found_boot;
}

FullDiskValidateState FullDiskDelegate::validate() const {
  // Policy:
  // * Returns ok if partition table of selected partition is empty.
  // * Check / partition is set.
  // * Check / partition is large enough.

  const Partition::Ptr root_partition = selected_partition_;

  // Check partition table is empty or not.
  const int device_index = DeviceIndex(virtual_devices_,
                                       root_partition->device_path);
  if (device_index == -1) {
    qCritical() << "validate() device index out of range:"
                << root_partition->device_path;
    return FullDiskValidateState::RootMissing;
  }
  const Device::Ptr device = virtual_devices_.at(device_index);
  if (device->table == PartitionTableType::Empty) {
    return FullDiskValidateState::Ok;
  }

  if (root_partition->device_path.isEmpty()) {
    return FullDiskValidateState::RootMissing;
  }

   // If currently selected device reaches its maximum primary partitions and
  // root partition is a Freespace, it is impossible to created a new
  // primary partition->
  if ((root_partition->type == PartitionType::Unallocated) &&
      !this->canAddPrimary(root_partition) &&
      !this->canAddLogical(root_partition)) {
    return FullDiskValidateState::MaxPrimPartErr;
  }

  const int root_required = GetSettingsInt(kPartitionMinimumDiskSpaceRequired);
  const qint64 root_minimum_bytes = root_required * kGibiByte;
  const qint64 root_real_bytes = root_partition->getByteLength() + kMebiByte;
  if (root_real_bytes < root_minimum_bytes) {
    return FullDiskValidateState::RootTooSmall;
  }

  return FullDiskValidateState::Ok;
}

void FullDiskDelegate::resetOperations() {
    operations_.clear();

    virtual_devices_ = FilterInstallerDevice(real_devices_);

    primaryPartitionLength = 0;
}

bool FullDiskDelegate::createPartition(const Partition::Ptr partition,
                                       PartitionType partition_type,
                                       bool align_start,
                                       FsType fs_type,
                                       const QString& mount_point,
                                       qint64 total_sectors,
                                       const QString& label) {
  // Policy:
  // * If partition table is empty, create a new one.
  const int device_index = DeviceIndex(virtual_devices_, partition->device_path);
  if (device_index == -1) {
    qCritical() << "createPartition() device index out of range:"
                << partition->device_path;
    return false;
  }

  Device::Ptr device = virtual_devices_[device_index];

  if (device->table == PartitionTableType::Empty) {
    // Add NewPartTable operation.
    Device::Ptr new_device(new Device(*device));
    new_device->partitions.clear();
    new_device->table = IsEfiEnabled() ?
                       PartitionTableType::GPT :
                       PartitionTableType::MsDos;

    //NOTE: GPT table need 33 sectors in the end.
    if (new_device->table == PartitionTableType::GPT) {
        partition->length -= 33;
        partition->end_sector -= 33;
    }

    const Operation operation(new_device);
    operations_.append(operation);
    // Update virtual device property at the same time.
    operation.applyToVisual(device);
  }

  if (fs_type == FsType::Recovery) {
      settings_.recovery_path = partition->path;
  }

  if (partition_type == PartitionType::Normal) {
    return createPrimaryPartition(partition,
                                  partition_type,
                                  align_start,
                                  fs_type,
                                  mount_point,
                                  total_sectors,
                                  label);
  } else if (partition_type == PartitionType::Logical) {
    return createLogicalPartition(partition,
                                  align_start,
                                  fs_type,
                                  mount_point,
                                  total_sectors,
                                  label);
  } else {
    qCritical() << "not supported partition type:" << partition_type;
    return false;
  }
}

bool FullDiskDelegate::createLogicalPartition(const Partition::Ptr partition,
                                              bool align_start,
                                              FsType fs_type,
                                              const QString& mount_point,
                                              qint64 total_sectors,
                                              const QString& label) {
  // Policy:
  // * Create extended partition if not found;
  // * If new logical partition is not contained in or is intersected with
  //   extended partition, enlarge extended partition->

  const int device_index = DeviceIndex(virtual_devices_, partition->device_path);
  if (device_index == -1) {
    qCritical() << "createLogicalPartition() device index out of range:"
                << partition->device_path;
    return false;
  }
  const Device::Ptr device = virtual_devices_.at(device_index);

  const int ext_index = ExtendedPartitionIndex(device->partitions);
  Partition::Ptr ext_partition (new Partition);
  if (ext_index == -1) {
    // TODO(xushaohua): Support extended partition in simple mode.
    qCritical() << "Cannot create extended partition in simple mode";
//    return false;

    // No extended partition found, create one.
    if (!createPrimaryPartition(partition,
                                PartitionType::Extended,
                                align_start,
                                FsType::Empty,
                                "",
                                total_sectors)) {
      qCritical() << "Failed to create extended partition";
      return false;
    }

    operations_.last().applyToVisual(device);
    ext_partition = operations_.last().new_partition;
  } else {
    // No need to add extended partition or enlarge it.
    ext_partition = device->partitions.at(ext_index);

    // Enlarge extended partition if needed.
    if (ext_partition->start_sector > partition->start_sector ||
        ext_partition->end_sector < partition->end_sector) {
      Partition::Ptr new_ext_partition(new Partition(*ext_partition));
      new_ext_partition->start_sector = qMin(ext_partition->start_sector,
                                            partition->start_sector);
      new_ext_partition->end_sector = qMax(ext_partition->end_sector,
                                          partition->end_sector);

      AlignPartition(new_ext_partition);

      Operation resize_ext_operation(OperationType::Resize,
                                           ext_partition,
                                           new_ext_partition);
      resize_ext_operation.device = device;
      operations_.append(resize_ext_operation);
      ext_partition = new_ext_partition;
    }
  }

  Partition::Ptr new_partition (new Partition);
  new_partition->device_path = partition->device_path;
  new_partition->path = partition->path;
  new_partition->sector_size = partition->sector_size;
  new_partition->status = PartitionStatus::New;
  new_partition->type = PartitionType::Logical;
  new_partition->fs = fs_type;
  new_partition->mount_point = mount_point;
  new_partition->label = label;

  for (const Operation& operation : operations_) {
      //###multidisk:maybe NOT the same disk?
      if (operation.type == OperationType::Create &&
          operation.device->path != device->path) {
          continue;
      }
    if ((operation.type == OperationType::NewPartTable &&
         operation.device->path == device->path) ||
        (operation.orig_partition != nullptr &&
         operation.orig_partition->device_path == device->path)) {
      operation.applyToVisual(device);
    }
  }

  const int partition_number = AllocLogicalPartitionNumber(device);
  if (partition_number < 0) {
    qCritical() << "Failed to allocate logical part number!";
    return false;
  }
  new_partition->changeNumber(partition_number);

  if (fs_type == FsType::Recovery) {
      // Hide recovery partition
      new_partition->flags << PartitionFlag::Hidden;
      settings_.recovery_path = new_partition->path;
  }

  // space is required for the Extended Boot Record.
  // Generally an additional track or MebiByte is required so for
  // our purposes reserve a MebiByte in front of the partition->
  const qint64 oneMebiByteSector = 1 * kMebiByte / partition->sector_size;
  if (align_start) {
    // Align from start of |partition|.
    // Add space for Extended Boot Record.
    const qint64 start_sector = qMax(partition->start_sector,
                                     ext_partition->start_sector);
    new_partition->start_sector = start_sector + oneMebiByteSector + 1;

    const qint64 end_sector = qMin(partition->end_sector,
                                   ext_partition->end_sector);
    new_partition->end_sector = qMin(end_sector,
                                    total_sectors + new_partition->start_sector - 1);
  } else {
    new_partition->end_sector = qMin(partition->end_sector,
                                    ext_partition->end_sector) - oneMebiByteSector;
    const qint64 start_sector = qMax(partition->start_sector,
                                     ext_partition->start_sector);
    new_partition->start_sector = qMax(start_sector + oneMebiByteSector - 1,
                                      partition->end_sector - total_sectors + 1);
  }

  // Align to nearest MebiBytes.
  AlignPartition(new_partition);

  // Check partition sector range.
  // Also check whether partition size is less than 1MiB or not.
  if (new_partition->start_sector < partition->start_sector ||
      new_partition->start_sector >= partition->end_sector ||
      new_partition->getByteLength() < kMebiByte ||
      new_partition->end_sector > partition->end_sector) {
    qCritical() << "Invalid partition sector range";
    return false;
  }

  Operation operation(OperationType::Create, partition, new_partition);
  //###multidisk
  operation.device = device;
  operations_.append(operation);

  return true;
}

bool FullDiskDelegate::createPrimaryPartition(const Partition::Ptr partition,
                                              PartitionType partition_type,
                                              bool align_start,
                                              FsType fs_type,
                                              const QString& mount_point,
                                              qint64 total_sectors,
                                              const QString& label) {
  // Policy:
  // * If new primary partition is contained in or intersected with
  //   extended partition, shrink extended partition or delete it if no other
  //   logical partitions.

  if (partition_type != PartitionType::Normal &&
      partition_type != PartitionType::Extended) {
    qCritical() << "createPrimaryPartition() invalid part type:"
                << partition_type;
    return false;
  }

  const int device_index = DeviceIndex(virtual_devices_, partition->device_path);
  if (device_index == -1) {
    qCritical() << "createPrimaryPartition() device index out of range:"
                << partition->device_path;
    return false;
  }
  Device::Ptr device = virtual_devices_[device_index];

  const qint64 oneMebiByteSector = 1 * kMebiByte / partition->sector_size;

  // Shrink extended partition if needed.
  const int ext_index = ExtendedPartitionIndex(device->partitions);
  if (partition_type == PartitionType::Normal && ext_index > -1) {
    const Partition::Ptr ext_partition = device->partitions.at(ext_index);
    const PartitionList logical_parts = GetLogicalPartitions(device->partitions);
    if (logical_parts.isEmpty()) {
      // Remove extended partition if no logical partitions.
      Partition::Ptr unallocated_partition (new Partition);
      unallocated_partition->device_path = ext_partition->device_path;
      // Extended partition does not contain any sectors.
      // This new allocated partition will be merged to other unallocated
      // partitions.
      unallocated_partition->start_sector = ext_partition->start_sector;
      unallocated_partition->end_sector = ext_partition->end_sector;
      unallocated_partition->sector_size = ext_partition->sector_size;
      unallocated_partition->type = PartitionType::Unallocated;
      const Operation operation(OperationType::Delete,
                                ext_partition,
                                unallocated_partition);
      operations_.append(operation);

      // Remove extended partition from partition list explicitly.
      device->partitions.removeAt(ext_index);

    } else if (IsPartitionsJoint(ext_partition, partition)) {
      // Shrink extended partition to fit logical partitions.
      Partition::Ptr new_ext_part(new Partition(*ext_partition));
      new_ext_part->start_sector = logical_parts.first()->start_sector -
                                  oneMebiByteSector;
      new_ext_part->end_sector = logical_parts.last()->end_sector;

      if (IsPartitionsJoint(new_ext_part, partition)) {
        qCritical() << "Failed to shrink extended partition!";
        return false;
      }

      const Operation operation(OperationType::Resize,
                                ext_partition,
                                new_ext_part);
      operations_.append(operation);
    }
  }

  Partition::Ptr new_partition (new Partition);
  new_partition->device_path = partition->device_path;
  new_partition->path = partition->path;
  new_partition->sector_size = partition->sector_size;
  new_partition->status = PartitionStatus::New;
  new_partition->type = partition_type;
  new_partition->fs = fs_type;
  new_partition->mount_point = mount_point;
  new_partition->label = label;

  int partition_number;
  // In simple mode, operations has never been applied to partition list.
  // So do it temporarily.
  for (const Operation& operation : operations_) {
      //###multidisk:maybe NOT the same disk?
      if (operation.type == OperationType::Create &&
          operation.device->path != device->path) {
          continue;
      }
    if ((operation.type == OperationType::NewPartTable &&
         operation.device->path == device->path) ||
        (operation.orig_partition != nullptr &&
         operation.orig_partition->device_path == device->path)) {
      operation.applyToVisual(device);
    }
  }

  partition_number = AllocPrimaryPartitionNumber(device);
  if (partition_number < 0) {
    qCritical() << "Failed to allocate primary partition number!";
    return false;
  }
  new_partition->changeNumber(partition_number);

  if (fs_type == FsType::Recovery) {
      // Hide recovery partition
      new_partition->flags << PartitionFlag::Hidden;
      settings_.recovery_path = new_partition->path;
  }

  // Check whether space is required for the Master Boot Record.
  // Generally an additional track or MebiByte is required so for
  // our purposes reserve a MebiByte in front of the partition->
  const bool need_mbr = (partition->start_sector <= oneMebiByteSector);
  if (align_start) {
    // Align from start of |partition|.
    if (need_mbr) {
      new_partition->start_sector = oneMebiByteSector;
    } else {
      new_partition->start_sector = partition->start_sector;
    }
    new_partition->end_sector = qMin(partition->end_sector,
                                    total_sectors + new_partition->start_sector - 1);
  } else {
    new_partition->end_sector = partition->end_sector;
    if (need_mbr) {
      new_partition->start_sector = qMax(oneMebiByteSector,
                                        partition->end_sector - total_sectors + 1);
    } else {
      new_partition->start_sector = qMax(partition->start_sector,
                                        partition->end_sector - total_sectors + 1);
    }
  }

  // Align to nearest MebiBytes.
  AlignPartition(new_partition);

  // Check partition sector range.
  // Also check whether partition size is less than 1MiB or not.
  if (new_partition->start_sector < partition->start_sector ||
      new_partition->start_sector >= partition->end_sector ||
      new_partition->getByteLength() < kMebiByte ||
      new_partition->end_sector > partition->end_sector) {
    qCritical() << "Invalid partition sector range"
                << ", new_partition:" << new_partition
                << ", partition:" << partition;
    return false;
  }

  Operation operation(OperationType::Create, partition, new_partition);
  //###multidisk
  operation.device = device;
  operations_.append(operation);

  primaryPartitionLength++;

  return true;
}

Partition::Ptr FullDiskDelegate::deletePartition(const Partition::Ptr partition) {
  Partition::Ptr new_partition (new Partition);
  new_partition->sector_size = partition->sector_size;
  new_partition->start_sector = partition->start_sector;
  new_partition->end_sector = partition->end_sector;
  new_partition->device_path = partition->device_path;
  new_partition->type = PartitionType::Unallocated;
  new_partition->freespace = new_partition->length;
  new_partition->fs = FsType::Empty;
  new_partition->status = PartitionStatus::Delete;

  // No need to merge operations.
  // No need to delete extended partition->

  Operation operation(OperationType::Delete, partition, new_partition);
  operations_.append(operation);

  return new_partition;
}

void FullDiskDelegate::formatPartition(const Partition::Ptr partition,
                                       FsType fs_type,
                                       const QString& mount_point) {
  qDebug() << "formatSimplePartition()" << partition << fs_type << mount_point;

  Partition::Ptr new_partition (new Partition);
  new_partition->sector_size = partition->sector_size;
  new_partition->start_sector = partition->start_sector;
  new_partition->end_sector = partition->end_sector;
  new_partition->path = partition->path;
  new_partition->device_path = partition->device_path;
  new_partition->fs = fs_type;
  new_partition->type = partition->type;
  new_partition->mount_point = mount_point;
  if (partition->status == PartitionStatus::Real) {
    new_partition->status = PartitionStatus::Format;
  } else if (partition->status == PartitionStatus::New ||
             partition->status == PartitionStatus::Format) {
    new_partition->status = partition->status;
  }

  Operation operation(OperationType::Format,partition, new_partition);
  operations_.append(operation);
}

bool FullDiskDelegate::formatWholeDevice(const QString& device_path,
                                         PartitionTableType type) {
  // * First clear any existing operations
  // * Then parse policy based on settings, partition table type and disk.
  // * At last create operations based on policy.
  qDebug() << "formatWholeDevice()" << device_path;

  const int device_index = DeviceIndex(virtual_devices_, device_path);
  if (device_index == -1) {
    qCritical() << "formatWholeDevice() device index out of range:"
                << device_path;
    return false;
  }
  Device::Ptr device = virtual_devices_[device_index];
  qDebug() << "device selected to format:" << device;

  Device::Ptr new_device(new Device(*device));
  new_device->partitions.clear();
  new_device->table = type;
  const Operation operation(new_device);
  operations_.append(operation);
  // Update virtual device property at the same time.
  operation.applyToVisual(device);

  if (device->partitions.length() == 0) {
      qCritical() << "partition is empty" << device;
      return false;
  }

  const QByteArray& policyStr{ GetFullDiskInstallPolicy() };
  if (policyStr.isEmpty()) {
      qWarning() << "Full Disk Install policy is empty!";
      return false;
  }

  const QJsonArray& policyArray = QJsonDocument::fromJson(policyStr).array();

  if (policyArray.isEmpty()) {
      qWarning() << "Full Disk Install policy is empty!";
      return false;
  }

  const uint     swapSize{ getSwapSize() };
  qint64         lastDeviceLenght{ device->length };
  Partition::Ptr unallocated = device->partitions.last();

  if (device->table == PartitionTableType::GPT) {
      // 首先创建EFI分区
      const qint64 uefiSize =
          ParsePartitionSize("300Mib", lastDeviceLenght * device->sector_size);

      bool ok;
      ok = createPrimaryPartition(unallocated, PartitionType::Normal, true, FsType::Ext4,
                                  "/boot/efi", uefiSize / device->sector_size);

      if (!ok) {
          qCritical() << "Failed to create partition on " << unallocated;
          return false;
      }

      const qint64 sectors = uefiSize / device->sector_size;
      lastDeviceLenght -= sectors;

      primaryPartitionLength++;

      Operation& last_operation = operations_.last();
      last_operation.applyToVisual(device);

      unallocated = device->partitions.last();
  }

  for (const QJsonValue& jsonValue : policyArray) {
      const QJsonObject& jsonObject  = jsonValue.toObject();
      QString            mount_point = jsonObject["mountPoint"].toString();
      const QJsonArray&  platform = jsonObject["platform"].toArray();

      if (!platform.contains(GetCurrentPlatform())) {
          continue;
      }

      const FsType fs_type{ GetFsTypeByName(jsonObject["filesystem"].toString()) };
      qint64       partitionSize{ 0 };

      const QString& use_range{ jsonObject["usage"].toString().toLower() };

      if (use_range == "swap-size") {
          partitionSize = ParsePartitionSize(QString("%1gib").arg(swapSize),
                                             lastDeviceLenght * device->sector_size);
      }

      if (partitionSize < 1) {
          partitionSize =
              ParsePartitionSize(use_range, lastDeviceLenght * device->sector_size);
      }

      if (mount_point == kLinuxSwapMountPoint) {
          mount_point = "";
      }

      const qint64 sectors = partitionSize / device->sector_size;
      lastDeviceLenght -= sectors;

      bool ok = false;

      if (device->table == PartitionTableType::GPT || primaryPartitionLength < (device->max_prims - 1)) {
          ok = createPrimaryPartition(unallocated,
                                      PartitionType::Normal,
                                      jsonObject["alignStart"].toBool(),
                                      fs_type,
                                      mount_point,
                                      sectors);
      } else {
          // create extend partition
          ok = createLogicalPartition(unallocated,
                                      jsonObject["alignStart"].toBool(),
                                      fs_type,
                                      mount_point,
                                      sectors);
      }

      if (!ok) {
          qCritical() << "Failed to create partition on " << unallocated;
          return false;
      }

      Operation& last_operation = operations_.last();
      last_operation.applyToVisual(device);

      // NOTE(justforlxz): 找到中间的空闲分区，因为对齐存在右边
      for (Partition::Ptr p : device->partitions) {
          if (p->type == PartitionType::Unallocated) {
              unallocated = p;
              break;
          }
      }
  }

    qDebug() << "operations for full disk mode:" << operations_;

    // Update bootloader path.
    setBootloaderPath(new_device->path);

    return true;
}

void FullDiskDelegate::onDeviceRefreshed(const DeviceList& devices) {
  real_devices_ = devices;
  operations_.clear();
  virtual_devices_ = FilterInstallerDevice(real_devices_);
  emit this->deviceRefreshed(virtual_devices_);
}

void FullDiskDelegate::onManualPartDone(const DeviceList& devices) {
  qDebug() << "FullDiskDelegate::onManualPartDone()" << devices;
  QString root_disk;
  QString root_path;
  QStringList mount_points;
  bool found_swap = false;
  QString esp_path;
  Device::Ptr root_device;

  // Check use-specified partitions with mount point.
  for (const Device::Ptr device : devices) {
    for (const Partition::Ptr partition : device->partitions) {
      if (!partition->mount_point.isEmpty()) {
        // Add in-used partitions to mount_point list.
        const QString record(QString("%1=%2").arg(partition->path)
                                 .arg( partition->mount_point));
        mount_points.append(record);
        if (partition->mount_point == kMountPointRoot) {
          root_disk = partition->device_path;
          root_path = partition->path;
          root_device = device;
        }
      }
    }
  }

  // Find esp partition on this device
  for (Partition::Ptr partition : root_device->partitions) {
     if (partition->fs == FsType::EFI && esp_path != partition->path) {
          esp_path = partition->path;
          break;
      }
  }

  // Check linux-swap.
  for (Partition::Ptr partition : root_device->partitions) {
      if (partition->fs == FsType::LinuxSwap) {
          found_swap = true;
          const QString record(QString("%1=swap").arg(partition->path));
          mount_points.append(record);
      }
  }

  if (!IsMBRPreferred(real_devices_)) {
    // Enable EFI mode. First check newly created EFI partition-> If not found,
    // check existing EFI partition->
    settings_.uefi_required = true;

    // There shall be only one EFI partition->
    if (esp_path.isEmpty()) {
      qCritical() << "esp path is empty!";
    }
    settings_.root_disk = root_disk;
    settings_.root_partition = root_path;
    settings_.boot_partition = esp_path;
    settings_.mount_points = mount_points.join(';');
  } else {
    settings_.uefi_required = false;
    // In legacy mode.
    settings_.root_disk = root_disk;
    settings_.root_partition = root_path;
    settings_.boot_partition = bootloader_path_;
    settings_.mount_points = mount_points.join(';');
  }

  // Create swap file if physical memory is less than 4Gib and
  // swap partition is not found.
  bool use_swap_file;
  if (found_swap) {
    use_swap_file = false;
  } else if (!GetSettingsBool(kPartitionEnableSwapFile)) {
    use_swap_file = false;
  } else if (GetSettingsBool(kPartitionForceSwapFileInSimplePage)) {
    use_swap_file = true;
  } else {
    use_swap_file = IsSwapAreaNeeded();
  }
  settings_.swap_file_required = use_swap_file;
  WriteDiskPartitionSetting(settings_);
}

void FullDiskDelegate::setBootloaderPath(const QString& path) {
    bootloader_path_ = path;
}

void FullDiskDelegate::saveSwapSize() {
    WriteSwapPartitionSize(getSwapSize());
}

uint FullDiskDelegate::getSwapSize() const
{
    // get system memory
    struct sysinfo myinfo;
    unsigned long total_bytes;

    sysinfo(&myinfo);

    total_bytes = myinfo.mem_unit * myinfo.totalram;

    const double by = static_cast<double>(total_bytes) /
            static_cast<double>(kKibiByte) /
            static_cast<double>(kKibiByte) /
            static_cast<double>(kKibiByte);

    qDebug() << "system memory is: " << total_bytes << by;

    uint size = qRound(sqrt(by) + qRound(by));

    // Limit swap to 16G
    // see: https://tower.im/teams/9487/todos/232339/
    return std::min(static_cast<uint>(16), size);
}

bool FullDiskDelegate::formatWholeDeviceMultipleDisk()
{
    resetOperations();
    selected_devices.clear();
    if (selectedDisks().isEmpty()) {
        return false;
    }

    const QStringList& device_path_list = selected_disks;
    for (QString device_path : device_path_list) {
        int device_index = DeviceIndex(virtual_devices_, device_path);
        if (device_index == -1) {
            return false;
        }
    }

    PartitionTableType table;
    table = IsEfiEnabled() ? PartitionTableType::GPT : PartitionTableType::MsDos;

    FullDiskOption   disk_option;
    FullDiskPolicyList & policy_list = disk_option.policy_list;

    // Get All policies from json file.
    {
        const QByteArray& policyStr{ GetFullDiskInstallPolicy() };
        if (policyStr.isEmpty()) {
            return false;
        }

        const QJsonArray policyArray = QJsonDocument::fromJson(policyStr).array();
        if (policyArray.isEmpty()) {
            return false;
        }

        for (const QJsonValue& jsonValue : policyArray) {
            const QJsonObject& jsonObject  = jsonValue.toObject();
            const QJsonArray&  platform = jsonObject["platform"].toArray();
            if (!platform.contains(GetCurrentPlatform())) {
                continue;
            }

            FullDiskPolicy  policy;
            policy.filesystem = GetFsTypeByName(jsonObject["filesystem"].toString());
            policy.mountPoint = jsonObject["mountPoint"].toString();
            policy.label      = jsonObject["label"].toString().toLower();
            policy.usage      = jsonObject["usage"].toString();
            policy.alignStart = jsonObject["alignStart"].toBool();
            policy.device     = jsonObject["device"].toString();
            if (policy.mountPoint == kLinuxSwapMountPoint) {
                policy.mountPoint = "";
            }
            policy_list.push_back(policy);
        }
   }

    // Change data disk configuration.
    for (int i = 0; i < policy_list.length(); i++) {
        FullDiskPolicy & policy = policy_list[i];
        if (device_path_list.length() > 1
            && policy.mountPoint == QString("/data")) {
            policy.device = device_path_list.at(1);
        }
        else {
            policy.device = device_path_list.at(0);
        }
    }

    // Format every disks one by one.
    for (const QString& device_path : device_path_list) {
        int device_index = DeviceIndex(virtual_devices_, device_path);

        Device::Ptr device = virtual_devices_[device_index];
        device->partitions.clear();
        device->table = table;

        disk_option.is_system_disk = device->path == device_path_list.at(0);
        if (!formatWholeDeviceV2(device, disk_option)) {
            selected_devices.clear();
            return false;
        }
        selected_devices.append(device);
    }
    return true;
}

struct  FullDiskPolicyComparator {
    bool operator()(const FullDiskPolicy& a, const FullDiskPolicy& b)
    {
            return a.startSector < b.startSector;
    }
};

bool FullDiskDelegate::formatWholeDeviceV2(const Device::Ptr& device, FullDiskOption& option)
{
    Operation operation(device);
    operations_.append(operation);
    operation.applyToVisual(device);

    int            primary_count { 0 };
    const uint     swapSize{ getSwapSize() };
    qint64         startSector{ 0 };
    qint64         endSector { device->length - 1 };
    qint64         lastDeviceLenght{ device->length };
    Partition::Ptr unallocated = device->partitions.last();

    const qint64 oneMebiByteSector = 1 * kMebiByte / device->sector_size;
    startSector = oneMebiByteSector;
    lastDeviceLenght -= oneMebiByteSector;
    qint64 adjust_start_offset_sector = startSector;
    int root_size_count { 0 };
    int percent100_count { 0 };

    if (IsEfiEnabled() && option.is_system_disk) {
        const qint64 uefiSize =
            ParsePartitionSize("300Mib", lastDeviceLenght * device->sector_size);
        if (!createPrimaryPartition(unallocated, PartitionType::Normal, true, FsType::EFI,
                                    "/boot/efi", uefiSize / device->sector_size, kUEFIPartitionLabel) ) {
            qCritical() << QString("Failed to create EFI partition with label:{%1}!")
                           .arg(kUEFIPartitionLabel);
            return false;
        }
        qInfo() << QString("UEFI partition with label:{%1} created successfully!")
                       .arg(kUEFIPartitionLabel);

        const qint64 sectors = uefiSize / device->sector_size;
        lastDeviceLenght -= sectors;
        primary_count++;
        Operation& last_operation = operations_.last();
        last_operation.applyToVisual(device);
        unallocated = device->partitions.last();
        startSector += sectors;
        adjust_start_offset_sector += sectors;
    }

    // total bytes used by policy
    const qint64 policy_total_bytes = lastDeviceLenght * device->sector_size;
    SizeRange root_range;

    FullDiskPolicyList& policy_list = option.policy_list;
    FullDiskPolicyList::iterator begin = policy_list.end();
    FullDiskPolicyList::iterator end   = policy_list.end();

    FullDiskPolicyList::iterator it;
    for (it = policy_list.begin(); it != policy_list.end(); it++) {
        FullDiskPolicy& policy = *it;
        if (policy.device != device->path) {
            if (begin != policy_list.end()) {
                end = it;
            }
            continue;
        }

        if (policy_list.end() == begin) {
            begin = it;
        }

        qint64       partitionSize{ 0 };
        const QString      swap_size_str = QString("%1gib").arg(swapSize);
        qint64  length = lastDeviceLenght * device->sector_size;
        if (policy.usage == "swap-size") {
            partitionSize = ParsePartitionSize(swap_size_str,length);
        }

        if (policy.usage == "100%") {
            percent100_count++;
        }

        if (policy.mountPoint == kMountPointRoot || policy.label == kFullDiskPolicyRootb) {
            if (policy.usage == kFullDiskPolicyUsageRootSize) {
                root_size_count++;
                const QString root_size_str = GetSettingsString(kPartitionFullDiskRootPartitionUsage);
                partitionSize = ParsePartitionSize(root_size_str, device->getByteLength());
            }

            root_range = getRootPartitionSizeRange();
            partitionSize = std::max(partitionSize, root_range.min_size_bytes);
            partitionSize = std::min(partitionSize, root_range.max_size_bytes);
        }

        if (partitionSize < 1) {
            partitionSize = ParsePartitionSize(policy.usage, length);
        }

        policy.sectors = partitionSize / device->sector_size;
        if (policy.alignStart) {
            policy.startSector = startSector;
            policy.endSector = policy.startSector + policy.sectors - 1;
            startSector += policy.sectors;
        }
        else {
            policy.endSector = endSector;
            policy.startSector = policy.endSector - policy.sectors + 1;
            endSector -= policy.sectors;
        }

        lastDeviceLenght -= policy.sectors;
        if (lastDeviceLenght < 0) {
            qDebug() << QString("FULLDISK:size out of range. please check your configuration file");
            return false;
        }

        bool is_primary = (device->table == PartitionTableType::GPT
                           || primary_count < (device->max_prims - 1));
        policy.partitionType = is_primary ? PartitionType::Normal : PartitionType::Logical;
        if (is_primary) {
            primary_count++;
        }
    }

    // Adjust partition size for root partition if it is system disk
    // and there is free space left
    PartitionAdjustOption adjust;
    adjust.free_bytes = lastDeviceLenght * device->sector_size;
    adjust.root_size_count = root_size_count;
    adjust.percent100_count = percent100_count;
    adjust.start_offset_sector = adjust_start_offset_sector;
    adjust.root_size_range = root_range;
    adjustFullDiskPolicy(device, option, adjust);

    // sort all policies
    qSort(begin, end, FullDiskPolicyComparator());

    const FullDiskPolicyList& const_policy_list = option.policy_list;
    for (const FullDiskPolicy& policy : const_policy_list) {
        if (policy.device != device->path) {
            continue;
        }

        // alignStart is always true ,since policies are sorted by start_sector
        if (!createPartition(unallocated, policy.partitionType, true, policy.filesystem,
                    policy.mountPoint, policy.sectors,policy.label)) {
            return false;
        }

        Operation& last_operation = operations_.last();
        last_operation.applyToVisual(device);
        for (Partition::Ptr p : device->partitions) {
            if (p->type == PartitionType::Unallocated) {
                unallocated = p;
                break;
            }
        }
    }

    if (option.is_system_disk) {
        setBootloaderPath(device->path);
    }
    return true;
}

void FullDiskDelegate::removeAllSelectedDisks()
{
    selected_devices.clear();
    selected_disks.clear();
}

void FullDiskDelegate::addSystemDisk(const QString & device_path)
{
    selected_devices.clear();
    selected_disks.clear();
    selected_disks.append(device_path);
}

void FullDiskDelegate::addDataDisk(const QString & device_path)
{
    if (selected_disks.isEmpty()) {
        return;
    }
    for(signed int i = selected_disks.length() - 1; i > 0; i--) {
        selected_disks.removeAt(i);
    }
    selected_disks.removeAll(device_path);
    selected_disks.append(device_path);
}

const QStringList & FullDiskDelegate::selectedDisks()
{
    return selected_disks;
}

void FullDiskDelegate::getFinalDiskResolution(FinalFullDiskResolution& resolution)
{
    resolution.option_list.clear();
    FinalFullDiskOption option;
    for (Operation op : operations()) {
        if (OperationType::Create != op.type || FsType::Empty == op.new_partition->fs) {
            continue;
        }

        FinalFullDiskPolicy policy;
        policy.filesystem = GetFsTypeName(op.new_partition->fs);
        policy.mountPoint = op.new_partition->mount_point;
        policy.label = op.new_partition->label;
        policy.offset = op.new_partition->start_sector * op.new_partition->sector_size;
        policy.size = (op.new_partition->end_sector - op.new_partition->start_sector) *
                op.new_partition->sector_size;
        policy.device = op.new_partition->device_path;

        if (option.policy_list.length() > 0
            && option.policy_list.last().device != policy.device) {
            resolution.option_list.append(option);
            option.policy_list.clear();
        }
        if (option.policy_list.length() == 0) {
            option.device = policy.device;
            option.password = "";
        }
        option.policy_list.append(policy);
    }
    if (option.policy_list.length() > 0) {
        resolution.option_list.append(option);
    }
}

const DiskPartitionSetting& FullDiskDelegate::settings() const
{
    return settings_;
}

const DeviceList FullDiskDelegate::selectedDevices()
{
    return selected_devices;
}

const SizeRange FullDiskDelegate::getRootPartitionSizeRange()
{
    const QString root_range_str = GetSettingsString(kPartitionFullDiskLargeRootPartRange);
    const QStringList root_ranges = root_range_str.split(":", QString::SkipEmptyParts);

    int min_root_size_gib { 0 };
    int max_root_size_gib { 0 };

    if (root_ranges.length() > 0) {
        min_root_size_gib = root_ranges[0].toInt();
    }

    if (root_ranges.length() > 1) {
        max_root_size_gib = root_ranges[1].toInt();
    }

    if (max_root_size_gib < min_root_size_gib) {
        max_root_size_gib = min_root_size_gib;
    }

    SizeRange range;
    range.min_size_bytes = kGibiByte * std::min(min_root_size_gib, max_root_size_gib);
    range.max_size_bytes = kGibiByte * std::max(min_root_size_gib, max_root_size_gib);
    return range;
}

void FullDiskDelegate::adjustFullDiskPolicy(const Device::Ptr& device, FullDiskOption& option, const PartitionAdjustOption& adjust)
{
    // no need to adjust if any policy's usage = 100%
    // no need to adjust if no policy's usage is root-size
    if (adjust.percent100_count > 0 || adjust.root_size_count == 0) {
        return;
    }

    qint64 start_sectors = adjust.start_offset_sector;
    qint64 end_sectors = device->length - 1;
    const qint64 total_free_sectors = adjust.free_bytes / device->sector_size;
    const qint64 root_free_sectors = total_free_sectors / adjust.root_size_count;

    FullDiskPolicyList& policy_list = option.policy_list;
    FullDiskPolicyList::iterator it;
    for (it = policy_list.begin(); it != policy_list.end(); it++) {
        FullDiskPolicy& policy = *it;
        if (policy.device != device->path) {
            continue;
        }

        if (policy.usage == kFullDiskPolicyUsageRootSize &&
           (policy.mountPoint == kMountPointRoot || policy.label == kFullDiskPolicyRootb))
        { // make sure that roota & rootb use all left space
            policy.sectors += root_free_sectors;
            qint64 partitionSize = policy.sectors * device->sector_size;
            partitionSize = std::max(partitionSize, adjust.root_size_range.min_size_bytes);
            partitionSize = std::min(partitionSize, adjust.root_size_range.max_size_bytes);
            policy.sectors = partitionSize / device->sector_size;
        }

        //others boundary maybe need adjust since some root partition's size is changed
        if (policy.alignStart) {
            policy.startSector = start_sectors;
            policy.endSector = policy.startSector + policy.sectors - 1;
            start_sectors = policy.endSector + 1;
        }
        else {
            policy.endSector = end_sectors;
            policy.startSector = policy.endSector - policy.sectors + 1;
            end_sectors = policy.startSector - 1;
        }
    }
}

}  // namespace installer
