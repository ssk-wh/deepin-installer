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

}  // namespace

Install_Lvm_Status AdvancedPartitionDelegate::install_Lvm_Status = Install_Lvm_Status::Lvm_No_Need;
QStringList AdvancedPartitionDelegate::mountPoints_AdvancedPartition;
bool AdvancedPartitionDelegate::swapOk = false;

AdvancedPartitionDelegate::AdvancedPartitionDelegate(QObject* parent)
    : partition::Delegate(parent),
      m_islvm(false)
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
      if (install_Lvm_Status == Install_Lvm_Status::Lvm_Install) {
          if(type == FsType::LVM2PV || type == FsType::EFI)
              continue;
      }
      fs_types.append(type);
    }
  }

  if (GetSettingsBool(kEnableRecoveryPartition)) {
      if (fs_types.indexOf(FsType::Recovery) == -1) {
          fs_types.append(FsType::Recovery);
      }
  } else {
      if (fs_types.indexOf(FsType::Recovery) != -1) {
          fs_types.removeAll(FsType::Recovery);
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
  QStringList new_mount_points;
  if (mount_points.isEmpty()) {
    // Read available mount points.
    mount_points = GetSettingsStringList(kPartitionMountPoints);

    // Replace "unused" mount point with ""
    for (QString& mount_point : mount_points) {
      if (mount_point == kMountPointUnused) {
        mount_point = "";
      }
      new_mount_points.append(mount_point);
      if (install_Lvm_Status == Install_Lvm_Status::Lvm_Install) {
          for (QString mountpoint : mountPoints_AdvancedPartition) {
              if(mountpoint == mount_point && mountpoint != "") {
                  new_mount_points.pop_back();
                  break;
              }
          }
      }
    }
  }
  return new_mount_points;
}

QList<Device::Ptr> AdvancedPartitionDelegate::getAllUsedDevice() const
{
    QList<Device::Ptr> list;

    auto operations_ { operations() };
    auto real_devices_ { realDevices() };

    for (const Operation::Ptr operation : operations_) {
        if (operation->type != OperationType::NewPartTable) {
            for (Device::Ptr device : real_devices_) {
                if (list.contains(device)) {
                    continue;
                }
                if (device->path == operation->orig_partition->device_path) {
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
      swapOk = false;
      ValidateStates states;
      bool found_lvm = false;
      bool found_efi = false;
      bool efi_large_enough = false;
      Partition::Ptr rootPartition;
      bool root_large_enough = false;
      Partition::Ptr bootPartition;
      bool boot_large_enough = false;
      Partition::Ptr efiPartition;

      // Filesystem of /boot and /.
      FsType boot_fs = FsType::Empty;
      FsType root_fs = FsType::Empty;

      const int root_required = GetSettingsInt(kPartitionRootMiniSpace);
      const int boot_recommended = GetSettingsInt(kPartitionDefaultBootSpace);
      const int efi_recommended = GetSettingsInt(kPartitionDefaultEFISpace);
      const int efi_minimum = GetSettingsInt(kPartitionEFIMinimumSpace);
      const int partition_min_size_by_gb = GetSettingsInt(kPartitionOthersMinimumSize);
      const qint64 partition_min_size_bytes = partition_min_size_by_gb * kGibiByte;

      Device::Ptr root_device;
      Device::Ptr boot_device;
      DeviceList pv_devices;

      qint64 offset_logical = 0;

      if (install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
          mountPoints_AdvancedPartition.clear();
          install_Lvm_Status = Install_Lvm_Status::Lvm_No_Need;
      }

      for (const Device::Ptr device : virtualDevices()) {
        for (const Partition::Ptr partition : device->partitions) {

          if (install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
            mountPoints_AdvancedPartition.append(partition->mount_point);
          }          
          if (partition->type == PartitionType::Logical) {
               offset_logical = kMebiByte;
          }
          if (partition->mount_point == kMountPointRoot) {
            // Check / partition->
            root_device = device;
            rootPartition = partition;
            root_fs = partition->fs;
            const qint64 root_real_bytes = partition->getByteLength();
            const qint64 root_minimum_bytes = root_required * kGibiByte - offset_logical;
            root_large_enough = (root_real_bytes >= root_minimum_bytes);

          } else if (partition->mount_point == kMountPointBoot) {
            // Check /boot partition->
            bootPartition = partition;
            boot_fs = partition->fs;
            const qint64 boot_recommend_bytes = boot_recommended * kMebiByte - offset_logical;
            // Add 1Mib to partition size.
            const qint64 boot_real_bytes = partition->getByteLength();
            boot_large_enough = (boot_real_bytes >= boot_recommend_bytes);
            boot_device = device;
          }
          else if (partition->mount_point == kMountPointEFI) {
              efiPartition = partition;
          }

          if (partition->fs == FsType::LVM2PV && partition->status != PartitionStatus::Real ) {
              found_lvm = true;
              if (pv_devices.indexOf(device) < 0) {
                  pv_devices.append(device);
              }

              if (install_Lvm_Status == Install_Lvm_Status::Lvm_No_Need) {
                  install_Lvm_Status = Install_Lvm_Status::Lvm_Format_Pv;
              }
          } else if (partition->fs == FsType::LinuxSwap) {
              swapOk = true;
          }

        }
      }

      if (!root_device.isNull()) {
          // Find esp partition on this device
          for (Partition::Ptr partition : root_device->partitions) {
             if (partition->fs == FsType::EFI) {
                 found_efi = true;
                 efiPartition = partition;
                 if (partition->status == PartitionStatus::Real) {
                     if (partition->type == PartitionType::Logical) {
                         offset_logical = kMebiByte;
                     }
                   // For existing EFI partition->
                   const qint64 efi_minimum_bytes = efi_minimum * kMebiByte - offset_logical;
                   const qint64 efi_real_bytes = partition->getByteLength();
                   efi_large_enough = (efi_real_bytes >= efi_minimum_bytes);
                 } else {
                   // For newly created EFI partition->
                   const qint64 efi_recommended_bytes = efi_recommended * kMebiByte;
                   const qint64 efi_real_bytes = partition->getByteLength();
                   efi_large_enough = (efi_real_bytes >= efi_recommended_bytes);
                 }
                 break;
              }
          }
      } else if(install_Lvm_Status == Install_Lvm_Status::Lvm_Format_Pv) {
          for (const Device::Ptr device : pv_devices) {
            for (const Partition::Ptr partition : device->partitions) {
                 if (partition->fs == FsType::EFI) {
                     found_efi = true;
                     efiPartition = partition;
                     if (partition->status == PartitionStatus::Real) {
                       // For existing EFI partition->
                       const qint64 efi_minimum_bytes = efi_minimum * kMebiByte;
                       const qint64 efi_real_bytes = partition->getByteLength();
                       efi_large_enough = (efi_real_bytes >= efi_minimum_bytes);
                     } else {
                       // For newly created EFI partition->
                       const qint64 efi_recommended_bytes = efi_recommended * kMebiByte;
                       const qint64 efi_real_bytes = partition->getByteLength();
                       efi_large_enough = (efi_real_bytes >= efi_recommended_bytes);
                     }
                     break;
                  }
              }
          }
      }

      if (!rootPartition.isNull()) {
        // Check root size only if root is set.
        if (!root_large_enough) {
          states.append(ValidateState::RootTooSmall);
        }
      } else if (install_Lvm_Status != Install_Lvm_Status::Lvm_Format_Pv) {
          states.append(ValidateState::RootMissing);
          for (QString mountPoint : mountPoints_AdvancedPartition) {
              if (mountPoint == kMountPointRoot) {
                   states.pop_back();
                   break;
              }
          }
      }

      if (install_Lvm_Status == Install_Lvm_Status::Lvm_Install) {
          const DeviceList devices = virtualDevices();

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

              if (partition->type == PartitionType::Logical) {
                  offset_logical = kMebiByte;
              }

              if (partition->getByteLength() + offset_logical < partition_min_size_bytes) {
                   states.append(ValidateState(ValidateState::PartitionTooSmall, partition));
              }
            }
          }

          if (devices.size() != 1) {
              return states;
          }

          if (devices.at(0)->partitions.size() != 1) {
              return states;
          }

          if (devices.at(0)->partitions.at(0)->partition_number != -1) {
              return states;
          }

          states.append(ValidateState::LvmPartNumberInvalid);

          return states;

      }

      // Check whether efi filesystem exists.
      if ((install_Lvm_Status == Install_Lvm_Status::Lvm_Format_Pv && !bootPartition.isNull() && boot_device->table == PartitionTableType::GPT)
              ||(!root_device.isNull() && root_device->device_type == DeviceType::NormalDevice && root_device->table == PartitionTableType::GPT)
              ||!this->isMBRPreferred()) {
        // program looks for root dir, if root dir exists, then it looks for efi dir
        // so, if found_efi is true, then found_root must be true
        if (found_efi) {
          if (!efi_large_enough) {
            states.append(ValidateState::EfiTooSmall);
          }
        }
        else {
          if (!rootPartition.isNull() || install_Lvm_Status == Install_Lvm_Status::Lvm_Format_Pv){
            states.append(ValidateState::EfiMissing);
          }
        }
      }

      if (!bootPartition.isNull() && !boot_large_enough) {
        states.append(ValidateState::BootTooSmall);
      } else if(bootPartition.isNull() && found_lvm) {
        states.append(ValidateState::BootBeforeLvm);
        return states;
      }

      // Check filesystem type is suitable for /boot folder.
      if (!bootPartition.isNull() || !rootPartition.isNull()) {
        const FsType boot_root_fs = !bootPartition.isNull() ? boot_fs : root_fs;
        const FsTypeList boot_fs_list = this->getBootFsTypeList();
        if (!boot_fs_list.contains(boot_root_fs)) {
          states.append(ValidateState::BootFsInvalid);
        }
      }

      // If /boot folder is required to be the first partition,
      // validate whether /boot or / partition is the first partition.
      if (GetSettingsBool(kPartitionBootOnFirstPartition)) {
          for (const Device::Ptr device : virtualDevices()) {
              PartitionList list;
              for (const Partition::Ptr partition : device->partitions) {
                  if (partition->type != PartitionType::Unallocated){
                      list << partition;
                  }
              }

              if (list.isEmpty()){
                  continue;
              }

              if (device->table == PartitionTableType::GPT) {
                  if (efiPartition.isNull()) {
                      continue;
                  }
                  int index = list.indexOf(efiPartition);
                  if ((index != -1 && index != 0) || efiPartition->partition_number != 1) {
                      states.clear();
                      states << ValidateState::EfiPartNumberinvalid;
                      return states;
                  }
                  continue;
              }

              if (!bootPartition.isNull()) {
                  if (bootPartition->device_path == device->path) {
                      int index = list.indexOf(bootPartition);
                      // boot partition exists, but is not the first partition.
                      if ((index != -1 && index != 0) || bootPartition->partition_number != 1) {
                          states.clear();
                          states << ValidateState::BootPartNumberInvalid;
                          return states;
                      }
                  }
              }

              if (rootPartition.isNull()) {
                  continue;
              }

              if (rootPartition->device_path != device->path) {
                  continue;
              }

              int index = list.indexOf(rootPartition);
              // boot partition does not exist, root partition exists, but is not the first partition.
              if (bootPartition.isNull() && ((index != -1 && index != 0) || rootPartition->partition_number != 1)) {
                  states.clear();
                  states << ValidateState::BootPartNumberInvalid;
                  return states;
              }
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
          if (partition->type == PartitionType::Logical) {
              offset_logical = kMebiByte;
          }
          if (partition->getByteLength() + offset_logical < partition_min_size_bytes) {
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
    Device::Ptr    boot_device;
    Partition::Ptr efi_partition;
    // Check use-specified partitions with mount point.
    for (const Device::Ptr &device : devices) {
        for (const Partition::Ptr &partition : device->partitions) {
            if (!partition->mount_point.isEmpty()) {
                // Add used partitions to mount_point list.
                const QString record(QString("%1=%2").arg(partition->path).arg(partition->mount_point));
                mount_points.append(record);
                if (partition->mount_point == kMountPointRoot) {
                    root_disk   = partition->device_path;
                    root_path   = partition->path;
                    root_device = device;
                }

                if (partition->mount_point == kMountPointBoot && bootloader_path_ == "") {
                    bootloader_path_ = partition->device_path;
                    boot_device = device;
                }
            }

            if (partition->fs == FsType::EFI && esp_path != partition->path) {
                // NOTE(lxz): maybe we shoud check efi freespcae
                esp_path = partition->path;                
            }
        }
    }

    // Find this device efi partition
    for (Partition::Ptr partition : root_device->partitions) {
        if (!root_device) continue;
        if (!partition->is_lvm && partition->fs == FsType::EFI && esp_path != partition->path) {
            // NOTE(lxz): maybe we shoud check efi freespcae
            esp_path = partition->path;
            break;
        }
    }

    // Check linux-swap.
   for (Partition::Ptr partition : root_device->partitions) {
        if (!root_device) continue;
        if (partition->fs == FsType::LinuxSwap) {
            found_swap = true;
            const QString record(QString("%1=swap").arg(partition->path));
            mount_points.append(record);
        }
    }

  if ((!boot_device.isNull() && boot_device->device_type == DeviceType::NormalDevice && boot_device->table == PartitionTableType::GPT)
          || (!root_device.isNull() && root_device->device_type == DeviceType::NormalDevice && root_device->table == PartitionTableType::GPT)
          || !IsMBRPreferred(realDevices())) {
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

    if (bootloader_path_.isEmpty()) {
        qCritical() << "bootloader_path_ path is empty!";
        if (boot_device.isNull() && !root_disk.isEmpty()) {
            bootloader_path_ = root_disk;
        }
    }

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
      const Operation::Ptr operation = operations_.at(index);
      // Remove the last FormatOperation if its new_partition range is the
      // same with |partition|.
      if (operation->type == OperationType::Format &&
          operation->new_partition == partition) {
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
