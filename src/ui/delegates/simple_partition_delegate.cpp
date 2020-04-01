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

#include "ui/delegates/simple_partition_delegate.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/partition_util.h"

namespace installer {

SimplePartitionDelegate::SimplePartitionDelegate(QObject* parent)
    : partition::Delegate(parent) {

}

ValidateStates SimplePartitionDelegate::validate() const {
  // Policy:
  // * Returns ok if partition table of selected partition is empty.
  // * Check / partition is set.
  // * Check / partition is large enough.

    ValidateStates states;

  if (selected_partition_.isNull()) {
       states << ValidateState::RootMissing;
  }

  const Partition::Ptr root_partition = selected_partition_;

  // Check partition table is empty or not.
  const int device_index = DeviceIndex(virtual_devices_,
                                       root_partition->device_path);
  if (device_index == -1) {
    qCritical() << "validate() device index out of range:"
                << root_partition->device_path;
    states << ValidateState::RootMissing;
  }
  const Device::Ptr device = virtual_devices_.at(device_index);
  if (device->table == PartitionTableType::Empty) {
    states << ValidateState::Ok;
  }

  if (root_partition->device_path.isEmpty()) {
    states << ValidateState::RootMissing;
  }

   // If currently selected device reaches its maximum primary partitions and
  // root partition is a Freespace, it is impossible to created a new
  // primary partition->
  if ((root_partition->type == PartitionType::Unallocated) &&
      !this->canAddPrimary(root_partition) &&
      !this->canAddLogical(root_partition)) {
    states << ValidateState::MaxPrimPartErr;
  }

  const int root_required = GetSettingsInt(kPartitionRootMiniSpace);
  const qint64 root_minimum_bytes = root_required * kGibiByte;
  const qint64 root_real_bytes = root_partition->getByteLength() + kMebiByte;
  if (root_real_bytes < root_minimum_bytes) {
    states << ValidateState::RootTooSmall;
  }

   return states.isEmpty() ? ValidateStates() << ValidateState::Ok : states;
}

bool SimplePartitionDelegate::formatWholeDevice(const QString& device_path,
                                                PartitionTableType type) {

  // Policy:
  //  * Create new partition table (msdos);
  //  * Create /boot with ext4, 500MiB;
  //  * Create swap partition with 4GiB;
  //  * Create / with ext4, remaining space (at most 300GiB);
  qDebug() << "formatWholeDevice()" << device_path;

  const int device_index = DeviceIndex(virtual_devices_, device_path);
  if (device_index == -1) {
    qCritical() << "formatWholeDevice() device index out of range:"
                << device_path;
    return false;
  }
  Device::Ptr device = virtual_devices_[device_index];
  qDebug() << "device selected to format:" << device;

  // Add NewPartTable operation.
  Device::Ptr new_device(new Device(*device));
  new_device->partitions.clear();
  new_device->table = type;
  Operation::Ptr operation(newOperation(new_device));
  operations_.append(operation);
  // Update virtual device property at the same time.
  operation->applyToVisual(device);

  if (device->partitions.length() == 0) {
    qCritical() << "partition is empty" << device;
    return false;
  }

  Partition::Ptr unallocated = device->partitions.last();

  // Create /boot partition->
  const int boot_space = GetSettingsInt(kPartitionDefaultBootSpace);
  const qint64 boot_sectors = boot_space * kMebiByte / unallocated->sector_size;
  bool ok = createPrimaryPartition(unallocated,
                                   PartitionType::Normal,
                                   true,
                                   FsType::Ext4,
                                   kMountPointBoot,
                                   boot_sectors);
  if (!ok) {
    qCritical() << "Failed to create /boot partition on" << unallocated;
    return false;
  }
  Operation::Ptr boot_operation = operations_.last();
  boot_operation->applyToVisual(device);

  unallocated = device->partitions.last();
  // Create swap partition->
  const int swap_space = GetSettingsInt(kPartitionSwapPartitionSize);
  const qint64 swap_sectors = swap_space * kMebiByte / unallocated->sector_size;
  ok = createPrimaryPartition(unallocated,
                              PartitionType::Normal,
                              true,
                              FsType::LinuxSwap,
                              "",
                              swap_sectors);
  if (!ok) {
    qCritical() << "Failed to created swap partition:" << device;
    return false;
  }
  Operation::Ptr swap_operation = operations_.last();
  swap_operation->applyToVisual(device);

  const qint64 kRootMaximumSize = 300 * kGibiByte;
  const qint64 device_size = device->getByteLength();
  const qint64 root_size = (device_size > kRootMaximumSize) ?
                           kRootMaximumSize :
                           device_size;
  const qint64 root_sectors = root_size / device->sector_size;

  unallocated = device->partitions.last();
  ok = createPrimaryPartition(unallocated,
                              PartitionType::Normal,
                              true,
                              FsType::Ext4,
                              kMountPointRoot,
                              root_sectors);
  if (!ok) {
    qCritical() << "Failed to create / partition:" << device;
    return false;
  }

  qDebug() << "operations for simple disk mode:" << operations_;

  return true;
}

void SimplePartitionDelegate::onManualPartDone(const DeviceList& devices) {
  qDebug() << "SimplePartitionDelegate::onManualPartDone()" << devices;
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

      // Check linux-swap.
      if (partition->fs == FsType::LinuxSwap) {
        found_swap = true;

        // Add swap area to mount_point list.
        // NOTE(xushaohua): Multiple swap partitions may be set.
        const QString record(QString("%1=swap").arg(partition->path));
        mount_points.append(record);
      } else if (partition->fs == FsType::EFI && esp_path.isEmpty()) {
        // Only use the first EFI partition->
        esp_path = partition->path;
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

  if (!IsMBRPreferred(real_devices_)) {
    // Enable EFI mode. First check newly created EFI partition-> If not found,
    // check existing EFI partition->
    WriteUEFI(true);

    // There shall be only one EFI partition->
    if (esp_path.isEmpty()) {
      qCritical() << "esp path is empty!";
    }
    WritePartitionInfo(root_disk, root_path, esp_path, mount_points.join(';'));
  } else {
    WriteUEFI(false);
    // In legacy mode.
    WritePartitionInfo(root_disk, root_path, bootloader_path_,
                       mount_points.join(';'));
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
  WriteRequiringSwapFile(use_swap_file);
}

}  // namespace installer
