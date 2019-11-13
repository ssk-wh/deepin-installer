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

#include "ui/delegates/advanced_partition_delegate.h"

#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/partition_util.h"
#include "base/command.h"

namespace installer {

namespace {

// "unused" mount point.
const char kMountPointUnused[] = "unused";

const QMap<FsType, QString> FsFormatCmdMap{
    { FsType::Btrfs, QString("mkfs.btrfs") },
    { FsType::EFI, QString("mkfs.vfat") },
    { FsType::Ext2, QString("mkfs.ext2") },
    { FsType::Ext3, QString("mkfs.ext3") },
    { FsType::Ext4, QString("mkfs.ext4") },
    { FsType::F2fs, QString("mkfs.f2fs") },
    { FsType::Fat16, QString("mkfs.fat") },
    { FsType::Fat32, QString("mkfs.fat") },
    { FsType::Hfs, QString("/usr/bin/hformat") },
    { FsType::HfsPlus, QString("/usr/bin/hpfsck") },
    { FsType::Jfs, QString("mkfs.jfs") },
    { FsType::LinuxSwap, QString("mkswap") },
    { FsType::LVM2PV, QString("lvm") },
    { FsType::Nilfs2, QString("mkfs.nilfs2") },
    { FsType::NTFS, QString("mkfs.ntfs") },
    { FsType::Reiser4, QString("mkfs.reiser4") },
    { FsType::Reiserfs, QString("mkfs.reiserfs") },
    { FsType::Xfs, QString("mkfs.xfs") }
};

}  // namespace

AdvancedPartitionDelegate::AdvancedPartitionDelegate(QObject* parent)
    : partition::Delegate(parent)
    {
  this->setObjectName("advanced_partition_delegate");
}

FsTypeList AdvancedPartitionDelegate::getFsTypeList() const {
  FsTypeList fs_types;
  if (fs_types.isEmpty()) {
    const QString name = GetSettingsString(kPartitionSupportedFs);
    Q_ASSERT(!name.isEmpty());
    const QStringList fs_names = name.split(';');
    for (const QString& fs_name : fs_names) {
      // check fs can be used
      const QString cmd = FsFormatCmdMap[GetFsTypeByName(fs_name)];
      if (cmd.isEmpty()) continue;
#ifndef QT_DEBUG
      if (!SpawnCmd("which", QStringList() << cmd)) continue;
#endif
      FsType type = GetFsTypeByName(fs_name);
      fs_types.append(type);
    }
  }
  return fs_types;
}

FsTypeList AdvancedPartitionDelegate::getBootFsTypeList() const {
  FsTypeList fs_types;
  if (fs_types.isEmpty()) {
    const QString name = GetSettingsString(kPartitionBootPartitionFs);
    Q_ASSERT(!name.isEmpty());
    const QStringList fs_names = name.split(';');
    for (const QString& fs_name : fs_names) {
      FsType type = GetFsTypeByName(fs_name);
      fs_types.append(type);
    }
  }
  return fs_types;}

QStringList AdvancedPartitionDelegate::getMountPoints() const {
  QStringList mount_points;
  if (mount_points.isEmpty()) {
    // Read available mount points.
    mount_points = GetSettingsStringList(kPartitionMountPoints);

    // Replace "unused" mount point with ""
    for (QString& mount_point : mount_points) {
      if (mount_point == kMountPointUnused) {
        mount_point = "";
      }
    }
  }
  return mount_points;
}

QList<Device::Ptr> AdvancedPartitionDelegate::getAllUsedDevice() const
{
    QList<Device::Ptr> list;

    auto operations_ { operations() };
    auto real_devices_ { realDevices() };

    for (const Operation& operation : operations_) {
        if (operation.type != OperationType::NewPartTable) {
            for (Device::Ptr device : real_devices_) {
                if (list.contains(device)) {
                    continue;
                }
                if (device->path == operation.orig_partition->device_path) {
                    list << device;
                    break;
                }
            }
        }
    }

    return list;
}

bool AdvancedPartitionDelegate::isPartitionTableMatch(
    const QString& device_path) const {
  return IsPartitionTableMatch(realDevices(), device_path);
}

ValidateStates AdvancedPartitionDelegate::validate() const {
  ValidateStates states;
  bool found_efi = false;
  bool efi_large_enough = false;
  bool found_root = false;
  bool root_large_enough = false;
  bool found_boot = false;
  bool boot_large_enough = false;

  // Filesystem of /boot and /.
  FsType boot_fs = FsType::Empty;
  FsType root_fs = FsType::Empty;

  // Partition number of /boot and /.
  int boot_part_number = -1;
  int root_part_number = -1;

  const int root_required = GetSettingsInt(kPartitionRootMiniSpace);
  const int boot_recommended = GetSettingsInt(kPartitionDefaultBootSpace);
  const int efi_recommended = GetSettingsInt(kPartitionDefaultEFISpace);
  const int efi_minimum = GetSettingsInt(kPartitionEFIMinimumSpace);
  const int partition_min_size_by_gb = GetSettingsInt(kPartitionOthersMinimumSize);
  const qint64 partition_min_size_bytes = partition_min_size_by_gb * kGibiByte;

  Device::Ptr root_device;
  for (const Device::Ptr device : virtualDevices()) {
    for (const Partition::Ptr partition : device->partitions) {
      if (partition->mount_point == kMountPointRoot) {
        // Check / partition->
        root_device = device;
        found_root = true;
        root_fs = partition->fs;
        root_part_number = partition->partition_number;
        const qint64 root_real_bytes = partition->getByteLength() + kMebiByte;
        const qint64 root_minimum_bytes = root_required * kGibiByte;
        root_large_enough = (root_real_bytes >= root_minimum_bytes);

      } else if (partition->mount_point == kMountPointBoot) {
        // Check /boot partition->
        found_boot = true;
        boot_fs = partition->fs;
        boot_part_number = partition->partition_number;
        const qint64 boot_recommend_bytes = boot_recommended * kMebiByte;
        // Add 1Mib to partition size.
        const qint64 boot_real_bytes = partition->getByteLength() + kMebiByte;
        boot_large_enough = (boot_real_bytes >= boot_recommend_bytes);

      }
    }
  }

  if (!root_device.isNull()) {
      // Find esp partition on this device
      for (Partition::Ptr partition : root_device->partitions) {
         if (partition->fs == FsType::EFI) {
             found_efi = true;
             if (partition->status == PartitionStatus::Real) {
               // For existing EFI partition->
               const qint64 efi_minimum_bytes = efi_minimum * kMebiByte;
               const qint64 efi_real_bytes = partition->getByteLength() + kMebiByte;
               efi_large_enough = (efi_real_bytes >= efi_minimum_bytes);
             } else {
               // For newly created EFI partition->
               const qint64 efi_recommended_bytes = efi_recommended * kMebiByte;
               const qint64 efi_real_bytes = partition->getByteLength() + kMebiByte;
               efi_large_enough = (efi_real_bytes >= efi_recommended_bytes);
             }
             break;
          }
      }
  }

  if (found_root) {
    // Check root size only if root is set.
    if (!root_large_enough) {
      states.append(ValidateState::RootTooSmall);
    }
  } else {
    states.append(ValidateState::RootMissing);
  }

  // Check whether efi filesystem exists.
  if (!this->isMBRPreferred()) {
    // program looks for root dir, if root dir exists, then it looks for efi dir
    // so, if found_efi is true, then found_root must be true
    if (found_efi) {
      if (!efi_large_enough) {
        states.append(ValidateState::EfiTooSmall);
      }
    }
    else {
      if (found_root){
        states.append(ValidateState::EfiMissing);
      }
    }
  }

  if (found_boot && !boot_large_enough) {
    states.append(ValidateState::BootTooSmall);
  }

  // Check filesystem type is suitable for /boot folder.
  if (found_boot || found_root) {
    const FsType boot_root_fs = found_boot ? boot_fs : root_fs;
    const FsTypeList boot_fs_list = this->getBootFsTypeList();
    if (!boot_fs_list.contains(boot_root_fs)) {
      states.append(ValidateState::BootFsInvalid);
    }
  }

  // If /boot folder is required to be the first partition, validate it.
  if (GetSettingsBool(kPartitionBootOnFirstPartition)) {
    const int boot_root_part_num = found_boot ?
                                   boot_part_number :
                                   root_part_number;
    // If /boot or / is set, validate its partition number.
    if ((boot_root_part_num != -1) && (boot_root_part_num != 1)) {
      states.append(ValidateState::BootPartNumberInvalid);
    }
  }

  const QStringList known_mounts { kMountPointRoot, kMountPointBoot };
  for (const Device::Ptr& device : virtualDevices()) {
    for (const Partition::Ptr& partition : device->partitions) {
      if (partition->fs == FsType::EFI) {
            continue;
      }
      if (partition->mount_point.isEmpty()) {
          continue;
      }
      if (known_mounts.contains(partition->mount_point)) {
          continue;
      }
      if (partition->getByteLength() < partition_min_size_bytes) {
           states.append(ValidateState(ValidateState::PartitionTooSmall, partition));
      }
    }
  }

  return states;
}

void AdvancedPartitionDelegate::onManualPartDone(const DeviceList& devices) {
  qDebug() << "AdvancedPartitionDelegate::onManualPartDone()" << devices;

    QString        root_disk;
    QString        root_path;
    QStringList    mount_points;
    bool           found_swap = false;
    QString        esp_path;
    Device::Ptr    root_device;
    Partition::Ptr efi_partition;

    // Check use-specified partitions with mount point.
    for (const Device::Ptr device : devices) {
        for (const Partition::Ptr partition : device->partitions) {
            if (!partition->mount_point.isEmpty()) {
                // Add used partitions to mount_point list.
                const QString record(QString("%1=%2").arg(partition->path).arg(partition->mount_point));
                mount_points.append(record);
                if (partition->mount_point == kMountPointRoot) {
                    root_disk   = partition->device_path;
                    root_path   = partition->path;
                    root_device = device;
                }
            }
        }
    }

    // Find this device efi partition
    for (Partition::Ptr partition : root_device->partitions) {
        if (partition->fs == FsType::EFI && esp_path != partition->path) {
            // NOTE(lxz): maybe we shoud check efi freespcae
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

  if (!IsMBRPreferred(realDevices())) {
    // Enable EFI mode. First check newly created EFI partition-> If not found,
    // check existing EFI partition->
    settings_.uefi_required = true;

    if (esp_path.isEmpty()) {
      // We shall never reach here.
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
  } else if (!GetSettingsBool(kPartitionEnableSwapFileInAdvancedPage)) {
    use_swap_file = false;
  } else {
    use_swap_file = IsSwapAreaNeeded();
  }
  settings_.swap_file_required = use_swap_file;
  WriteDiskPartitionSetting(settings_);
}

bool AdvancedPartitionDelegate::unFormatPartition(const Partition::Ptr partition) {
  Q_ASSERT(partition->status == PartitionStatus::Format);
  if (partition->status == PartitionStatus::Format) {
    OperationList operations_ = operations();
    for (int index = operations_.length() - 1; index >= 0; --index) {
      const Operation& operation = operations_.at(index);
      // Remove the last FormatOperation if its new_partition range is the
      // same with |partition|.
      if (operation.type == OperationType::Format &&
          operation.new_partition == partition) {
        operations_.removeAt(index);
        return true;
      }
    }
    qCritical() << "No appropriate FormatPartition found:" << partition;
  } else {
    qCritical() << "Invalid partition status:" << partition;
  }
  return false;
}

}  // namespace installer
