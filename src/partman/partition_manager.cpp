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

#include "partition_manager.h"

#include <parted/parted.h>
#include <QDebug>
#include <QDir>
#include <QCollator>
#include <QProcess>

#include "base/command.h"
#include "partman/libparted_util.h"
#include "partman/os_prober.h"
#include "partman/partition_usage.h"
#include "sysinfo/dev_disk.h"
#include "sysinfo/proc_mounts.h"
#include "ui/delegates/partition_util.h"

namespace installer {

namespace {

// Absolute path to hook_manager.sh
const char kHookManagerFile[] = BUILTIN_HOOKS_DIR "/hook_manager.sh";

#define DEVICE_IS_READ_ONLY  (1)

// Get flags of |lp_partition|.
PartitionFlags GetPartitionFlags(PedPartition* lp_partition) {
  Q_ASSERT(lp_partition);
  PartitionFlags flags;
  for (PedPartitionFlag lp_flag =
         ped_partition_flag_next(static_cast<PedPartitionFlag>(NULL));
       lp_flag;
       lp_flag = ped_partition_flag_next(lp_flag)) {
    if (ped_partition_is_flag_available(lp_partition, lp_flag) &&
        ped_partition_get_flag(lp_partition, lp_flag)) {
      flags.append(static_cast<PartitionFlag>(lp_flag));
    }
  }
  return flags;
}

// Read all partitions of |lp_disk|.
PartitionList ReadPartitions(PedDisk* lp_disk) {
  Q_ASSERT(lp_disk);
  PartitionList partitions;
  for (PedPartition* lp_partition = ped_disk_next_partition(lp_disk, nullptr);
      lp_partition != nullptr;
      lp_partition = ped_disk_next_partition(lp_disk, lp_partition)) {

    Partition::Ptr partition (new Partition);
    if (lp_partition->type == PED_PARTITION_NORMAL) {
      partition->type = PartitionType::Normal;
    } else if (lp_partition->type == PED_PARTITION_EXTENDED) {
      partition->type = PartitionType::Extended;
    } else if (lp_partition->type ==
        (PED_PARTITION_FREESPACE | PED_PARTITION_LOGICAL)) {
      partition->type = PartitionType::Unallocated;
    } else if (lp_partition->type == PED_PARTITION_LOGICAL) {
      partition->type = PartitionType::Logical;
    } else if (lp_partition->type == PED_PARTITION_FREESPACE) {
      partition->type = PartitionType::Unallocated;
    } else {
      // Ignore other types
      continue;
    }

    // Get partition flags when it is active.
    if (ped_partition_is_active(lp_partition)) {
      partition->flags = GetPartitionFlags(lp_partition);
    }

    if (lp_partition->fs_type) {
      partition->fs = GetFsTypeByName(lp_partition->fs_type->name);
      // If ESP/Boot flag is set on fat16/fat32 partition,
      // it shall be an EFI partition->
      if ((partition->fs == FsType::Fat32 || partition->fs == FsType::Fat16) &&
          partition->flags.contains(PartitionFlag::ESP)) {
        partition->fs = FsType::EFI;
      }
    } else {
      partition->fs = FsType::Unknown;
    }
    partition->start_sector = lp_partition->geom.start;
    partition->end_sector = lp_partition->geom.end;

    partition->partition_number = lp_partition->num;
    partition->path = GetPartitionPath(lp_partition);

    // Avoid reading additional filesystem information if there is no path.
    if (!partition->path.isEmpty() &&
        partition->type != PartitionType::Unallocated &&
        partition->type != PartitionType::Extended) {

      // Read label based on filesystem type
      ReadUsage(partition->path, partition->fs, partition->freespace,
                partition->length);
      // If LinuxSwap partition is not mount, it is totally free.
      if (partition->fs == FsType::LinuxSwap && partition->length <= 0) {
        partition->length = partition->getByteLength();
        partition->freespace = partition->length;
      }

      // Get partition name.
      partition->name = ped_partition_get_name(lp_partition);
    }
    partitions.append(partition);
  }
  return partitions;
}

// Unmount devices and swap partitions.
bool UnmountDevices() {
  // Swap off partitions and files.
  bool ok;
  QString out, err;
  ok = SpawnCmd("swapoff", {"--all"}, out, err);
  if (!ok) {
    qWarning() << "swapoff failed!" << out << err;
  }
  const char kTargetDir[] = "/target";
  if (QDir(kTargetDir).exists()) {
    if (!SpawnCmd("umount", {"-R", kTargetDir}, out, err)) {
      ok = false;
      qWarning() << "umount /target failed" << out << err;
    }
  }

  // TODO(xushaohua): Handles lvm.

  // umount --all almost always returns false.
  //SpawnCmd("umount", {"--all"}, out, err);

  return ok;
}

}  // namespace

PartitionManager::PartitionManager(QObject* parent)
    : QObject(parent),
      enable_os_prober_(true) {
  this->setObjectName("partition_manager");

  // Register meta types used in signals.
  qRegisterMetaType<DeviceList>("DeviceList");
  qRegisterMetaType<OperationList>("OperationList");
  qRegisterMetaType<PartitionTableType>("PartitionTableType");
  this->initConnections();
}

PartitionManager::~PartitionManager() {
  // No need to release objects in operation list.
  // It is released in PartitionDelegate.
}

void PartitionManager::initConnections() {
  connect(this, &PartitionManager::createPartitionTable,
          this, &PartitionManager::doCreatePartitionTable);
  connect(this, &PartitionManager::refreshDevices,
          this, &PartitionManager::doRefreshDevices);
  connect(this, &PartitionManager::autoPart,
          this, &PartitionManager::doAutoPart);
  connect(this, &PartitionManager::manualPart,
          this, &PartitionManager::doManualPart);
}

void PartitionManager::doCreatePartitionTable(const QString& device_path,
                                              PartitionTableType table) {
  if (!CreatePartitionTable(device_path, table)) {
    qCritical() << "PartitionManager failed to create partition table at"
                << device_path;
  }
  const DeviceList devices = ScanDevices(enable_os_prober_);
  emit this->devicesRefreshed(devices);
}

void PartitionManager::doRefreshDevices(bool umount, bool enable_os_prober) {
  // Umount devices first.
  if (umount) {
    UnmountDevices();
  }

  enable_os_prober_ = enable_os_prober;
  const DeviceList devices = ScanDevices(enable_os_prober);
  emit this->devicesRefreshed(devices);
}

void PartitionManager::doAutoPart(const QString& script_path) {
  if (!QFile::exists(script_path)) {
    qCritical() << "partition script file not found!" << script_path;
    emit this->autoPartDone(false);
    return;
  }
  const bool ok = RunScriptFile({kHookManagerFile, script_path});
  emit this->autoPartDone(ok);
}

void PartitionManager::doManualPart(const OperationList& operations) {
  qDebug() << Q_FUNC_INFO << "\n" << "operations:" << operations;
  bool ok = true;
  bool isLvm2Pv = false;
  bool islvm = false;
  // Copy operation list, as partition path will be updated in applyToDisk().
  OperationList real_operations(operations);
  PartitionList lvmpv_PartitionList;

  for (int i = 0; ok && i < real_operations.length(); ++i) {
      Operation::Ptr operation = real_operations[i];
      if (!operation->new_partition) continue;
      if (operation->new_partition->fs == FsType::LVM2PV
              && (operation->type == OperationType::Create || operation->type ==OperationType::Format)) {
            isLvm2Pv = true;
            lvmpv_PartitionList.append(operation->new_partition);
      } else if (operation->orig_partition->fs == FsType::LVM2PV && operation->type == OperationType::Delete) {
          for(Partition::Ptr pvPartitiom : lvmpv_PartitionList) {
              if (pvPartitiom->path == operation->orig_partition->path) {
                  int delret = lvmpv_PartitionList.indexOf(pvPartitiom);
                  lvmpv_PartitionList.removeAt(delret);
                  if (lvmpv_PartitionList.size() == 0) {
                      isLvm2Pv = false;
                  }
              }
          }
      }
      if (operation->new_partition->is_lvm) {
          islvm = true;
          //注意一定要isLvm2Pv = false;
          isLvm2Pv = false;
      }
  }
  qDebug() << Q_FUNC_INFO << "\n" << "real operations:" << real_operations;

  DeviceList devices;
  if (!islvm && isLvm2Pv) {
      devices = ScanVgDevices(lvmpv_PartitionList);
      emit this->manualPartDone(ok, devices);
      return ;
  }

  for (int i = 0; ok && i < real_operations.length(); ++i) {
    Operation::Ptr operation = real_operations[i];
    if (operation->type == OperationType::NewPartTable) {
        DeviceList fulldisk_devices = ScanDevices(false);
        for (Device::Ptr device : fulldisk_devices) {
            if(operation->device->path == device->path) {
                operation->umount(device);
            }
        }
    }
    ok = operation->applyToDisk();
  }

  if (ok) {
    devices = ScanDevices(false);
    if (islvm && !isLvm2Pv) {
        devices.append(real_operations.back()->device);
        VgDevice::p_installer_VgDevice->enableVG(true);
    }
    // Update mount point of real partitions.
    std::map<QString, Partition::Ptr> mountMap;
    for (const Operation::Ptr operation : real_operations) {
        if (!operation->new_partition || !operation->orig_partition) continue;
        if ((operation->type == OperationType::Create) ||
            (operation->type == OperationType::Format) ||
            (operation->type == OperationType::MountPoint)) {
            mountMap[operation->new_partition->mount_point] = operation->new_partition;
        }

        if (operation->type == OperationType::Delete) {
            mountMap.erase(operation->orig_partition->mount_point);
        }
    }

    DeviceList ok_devices;
    for (Device::Ptr device : devices) {
        if (!device) continue;
        ok_devices.append(device);
        for (Partition::Ptr partition : device->partitions) {
            auto it = std::find_if(
                mountMap.cbegin(), mountMap.cend(),
                [=](std::pair<QString, Partition::Ptr> pair) {
                    return (pair.second->device_path == partition->device_path) &&
                           (partition->start_sector == pair.second->start_sector) &&
                           (partition->end_sector == pair.second->end_sector);
                });

            if (it != mountMap.cend()) {
                const auto [mountPoint, p] = *it;
                partition->mount_point     = p->mount_point;
            }
        }
    }
    devices = ok_devices;
  }

  emit this->manualPartDone(ok, devices);
}

void EnableVG(bool enable) {
   const QString cmd { "vgchange" };
   const QStringList args { "-a", enable ? "y" : "n" };
   QString output { "" };
   QString error  { "" };
   if (!SpawnCmd(cmd, args, output, error)) {
       qWarning() << QString("EnableVG:Failed to enable vg(%1)").arg(enable);
       if (!error.isEmpty()) {
            qWarning() << QString("EnableVG:{%1}").arg(error);
       }
   }
   if (!output.isEmpty()) {
       qInfo() << QString("EnableVG:{%1}").arg(output);
   }
}

DeviceList ScanDevices(bool enable_os_prober) {
  // 0. Disable VG device
  // 1. List Devices
  // 1.1. Retrieve metadata of each device->
  // 2. List partitions of each device->
  // 3. Retrieve partition metadata.

   EnableVG(false);
   // 通过 pvdisplay -s 扫描出 LVM2PV 的path
   QProcess process;
   QStringList devicePathList;
   process.start("pvdisplay -s");
   process.waitForFinished();
   QString str = process.readLine();
   while (str.size() > 0) {
       str.replace("\n","");
       auto lst = str.split("\"");
       if (lst.size() == 3) {
           devicePathList.append(lst[1]);
       }
       str = process.readLine();
   }

  // Let libparted detect all devices and construct device list.
  ped_device_probe_all();

#ifdef QT_DEBUG_sadhu
  // NOTE(justforlxz): The loop device is not loaded by
  // default and needs to be processed manually.
  QDir dir("/dev/");
  dir.setFilter(QDir::System);
  QFileInfoList list = dir.entryInfoList();
  for (const QFileInfo& info : list) {
      if (info.filePath().startsWith("/dev/loop")) {
          qDebug() << "load Loop device: " << info.filePath();
          ped_device_get(info.filePath().toUtf8().data());
      }
  }
#endif // !QT_DEBUG

  DeviceList devices;
  const LabelItems label_items = ParseLabelDir();
  const MountItemList mount_items = ParseMountItems();

  OsProberItems os_prober_items;
  if (enable_os_prober) {
    RefreshOsProberItems();
    os_prober_items = GetOsProberItems();
  }

  // Walk through all devices.
  for (PedDevice* lp_device = ped_device_get_next(nullptr);
      lp_device != nullptr;
      lp_device = ped_device_get_next(lp_device)) {
    PedDiskType* disk_type = ped_disk_probe(lp_device);

    qInfo() << QString("DEVICE#:path:{%1} model:{%2} type:{%3}:{%4} sec:{%5}:{%6} len:{%7}: ro:{%8}")
                .arg(lp_device->path)
                .arg(lp_device->model)
                .arg(lp_device->type)
                .arg(disk_type != nullptr ? disk_type->name : "nullptr")
                .arg(lp_device->sector_size)
                .arg(lp_device->phys_sector_size)
                .arg(lp_device->length)
                .arg(lp_device->read_only);

    if (DEVICE_IS_READ_ONLY == lp_device->read_only) {
        qInfo() << QString("IGNORED:by readonly:{%1}, path:{%2}")
                   .arg(lp_device->read_only)
                   .arg(lp_device->path);
        continue;
    }

#ifndef QT_DEBUG
    if (PED_DEVICE_LOOP == lp_device->type) {
        qInfo() << QString("IGNORED:by type:{%1} path:{%2}")
                   .arg(lp_device->type)
                   .arg(lp_device->path);
        continue;
    }
#endif

    const bool is_valid_dev = [&](PedDeviceType type) -> bool {
        std::list<PedDeviceType> blackList {
#ifndef QT_DEBUG
            PedDeviceType::PED_DEVICE_LOOP,
#endif
            PedDeviceType::PED_DEVICE_UNKNOWN,
        };

        for (PedDeviceType _type : blackList) {
            if (_type == type) {
                return false;
            }
        }

        return true;
    }(lp_device->type);

    if (!is_valid_dev) {
        qDebug() << "device type: " << lp_device->type;
        continue;
    }

    Device::Ptr device(new Device);
    if (disk_type == nullptr) {
      // Current device has no partition table.
      device->table = PartitionTableType::Empty;
    } else {
      const QString disk_type_name(disk_type->name);
      if (disk_type_name == kPartitionTableGPT) {
        device->table = PartitionTableType::GPT;
      } else if (disk_type_name == kPartitionTableMsDos) {
        device->table = PartitionTableType::MsDos;
#ifdef QT_DEBUG
      } else if (disk_type_name == kPartitionLoop) {
        device->table = PartitionTableType::Others;
        qDebug() << "add device: " << disk_type_name << lp_device->path;
      }
      //The following if statement is just for debug only on my X86_X64 PC.
      //There is an unknown reason why type name is "mac" on my X86_X64 PC.
      else if (disk_type_name == kPartitionCDRomDebug) {
          device->table = PartitionTableType::Others;
          qDebug() << "add device: " << disk_type_name << lp_device->path;
#endif
      } else {
          device->table = PartitionTableType::Empty;
          qWarning() << "other type of device:" << lp_device->path
                             << disk_type->name << lp_device->type;
      }
    }

    device->path = lp_device->path;
    device->model = lp_device->model;
    device->length = lp_device->length;
    device->sector_size = lp_device->sector_size;
    device->heads = lp_device->bios_geom.heads;
    device->sectors = lp_device->bios_geom.sectors;
    device->cylinders = lp_device->bios_geom.cylinders;

    if (device->table == PartitionTableType::Empty) {
      Partition::Ptr free_partition(new Partition);
      free_partition->device_path = device->path;
      free_partition->path = "";
      free_partition->partition_number = -1;
      free_partition->start_sector = 1;
      free_partition->end_sector = device->length;
      free_partition->sector_size = device->sector_size;
      free_partition->type = PartitionType::Unallocated;
      device->partitions.append(free_partition);

    } else if (device->table == PartitionTableType::MsDos ||
        device->table == PartitionTableType::GPT || device->table == PartitionTableType::Others) {
      PedDisk* lp_disk = nullptr;
      lp_disk = ped_disk_new(lp_device);

      if (lp_disk) {
        device->max_prims = ped_disk_get_max_primary_partition_count(lp_disk);

        // If partition table is known, scan partitions in this device->
        device->partitions = ReadPartitions(lp_disk);
        // Add additional info to partitions.
        for (Partition::Ptr partition : device->partitions) {
          partition->device_path = device->path;
          partition->sector_size = device->sector_size;

          // 判断是否是LVM2PV
          for (QString partitionPath : devicePathList) {
              if (partition->path == partitionPath) {
                  partition->fs = FsType::LVM2PV;
              }
          }

          if (!partition->path.isEmpty() &&
              partition->type != PartitionType::Unallocated) {
            // Read partition label and os.
            const QString label_str = label_items.value(partition->path, GetPartitionLabel(partition));
            if (!label_str.isEmpty()) {
                partition->label = label_str;
            }
            for (const OsProberItem& item : os_prober_items) {
              if (item.path == partition->path) {
                partition->os = item.type;
                break;
              }
            }

            // Mark busy flag of this partition when it is mounted in system.
            for (const MountItem& mount_item : mount_items) {
              if (mount_item.path == partition->path) {
                partition->busy = true;
                break;
              }
            }
          }
        }
        ped_disk_destroy(lp_disk);

      } else {
        qCritical() << "Failed to get disk object:" << device->path;
      }
    }

    devices.append(device);
  }

  // Add simulated disks in debug mode for debugging the partition frame.
  #ifdef QT_DEBUG
      int deviceNum = 1;

      // add a MBR disk which has not any partitions.
      devices << constructDevice1(deviceNum);

      // add a MBR disk which has two partitions.
      ++deviceNum;
      devices << constructDevice2(deviceNum);

      // add a GPT disk which has four partitions and one of the partitions is working.
      ++deviceNum;
      devices << constructDevice3(deviceNum);
  #endif // QT_DEBUG

  QCollator collator;
  collator.setCaseSensitivity(Qt::CaseInsensitive);
  std::sort(devices.begin(), devices.end(), [=](Device::Ptr a, Device::Ptr b) {
    return collator.compare(a->path, b->path) < 0;
  });

  return devices;
}

DeviceList ScanVgDevices(PartitionList & partitionList)
{
    VgDevice* device = VgDevice::installer_VgDevice(partitionList);
    device->model = "VG device";
    device->path = QString("/dev/%1").arg(device->m_vgName);
    device->length = device->getSize() / 512;
    device->sectors = 63;
    device->sector_size = 512;
    device->max_prims = kGPTPartitionNums;
    device->read_only = false;
    device->table = PartitionTableType::GPT;

    int partitionNo = 1;
    LvPartition* partition1 = new LvPartition();
    partition1->device_path = device->path;
    partition1->path = QString("%1/lv1").arg(device->path);
    partition1->partition_number = -1;
    partition1->type = PartitionType::Unallocated;
    partition1->status = PartitionStatus::Real;
    partition1->fs = FsType::Unknown;
    partition1->busy = false;
    partition1->start_sector = 0;
    partition1->end_sector = device->length;
    partition1->sector_size = 512;

    PartitionList partitions;
    partitions << Partition::Ptr(partition1);
    device->partitions = partitions;
    DeviceList devices;
    devices.append(Device::Ptr(device));
    return devices;
}

Device::Ptr constructDevice1(int deviceNum)
{
    Device* device = new Device();
    device->model = "debug device";
    device->path = QString("/dev/debugDevice%1").arg(deviceNum);
    device->length = 3097152000000;
    device->sectors = 63;
    device->sector_size = 512;
    device->max_prims = 4;
    device->read_only = true;
    device->table = PartitionTableType::MsDos;

    int partitionNo = 1;
    Partition* partition1 = new Partition();
    partition1->device_path = device->path;
    partition1->path = QString("%1p-1").arg(device->path);
    partition1->partition_number = -1;
    partition1->type = PartitionType::Normal;
    partition1->status = PartitionStatus::Real;
    partition1->fs = FsType::LinuxSwap;
    partition1->busy = false;
    partition1->start_sector = 0;
    partition1->end_sector = 209715200;
    partition1->sector_size = 512;
    partition1->length = 3097152000000;
    partition1->freespace = partition1->length / 2;
    partition1->mount_point = "/";

    PartitionList partitions;
    partitions << Partition::Ptr(partition1);
    device->partitions = partitions;

    return Device::Ptr(device);
}

Device::Ptr constructDevice2(int deviceNum)
{
    Device* device = new Device();
    device->model = "debug device";
    device->path = QString("/dev/debugDevice%1").arg(deviceNum);
    device->length = 409715200;
    device->sectors = 63;
    device->sector_size = 512;
    device->max_prims = 4;
    device->read_only = true;
    device->table = PartitionTableType::MsDos;

    int partitionNo = 1;
    Partition* partition1 = new Partition();
    partition1->device_path = device->path;
    partition1->path = QString("%1p%2").arg(device->path).arg(partitionNo);
    partition1->partition_number = partitionNo;
    partition1->type = PartitionType::Normal;
    partition1->status = PartitionStatus::Real;
    partition1->fs = FsType::Ext4;
    partition1->busy = false;
    partition1->label = QString("debugDevice%1p%2").arg(deviceNum).arg(partitionNo);
    partition1->start_sector = 2048;
    partition1->end_sector = 209715219;
    partition1->sector_size = 512;
    partition1->length = 409715200 / 2;
    partition1->freespace = partition1->length / 4;
    partition1->mount_point = "/home/zhangdd";

    ++partitionNo;
    Partition* partition2 = new Partition();
    partition2->device_path = device->path;
    partition2->path = QString("%1p%2").arg(device->path).arg(partitionNo);
    partition2->partition_number = partitionNo;
    partition2->type = PartitionType::Normal;
    partition2->status = PartitionStatus::Real;
    partition2->fs = FsType::Ext4;
    partition2->busy = false;
    partition2->label = QString("debugDevice%1p%2").arg(deviceNum).arg(partitionNo);
    partition2->start_sector = 209715200;
    partition2->end_sector = 409715200;
    partition2->sector_size = 512;

    PartitionList partitions;
    partitions << Partition::Ptr(partition1) << Partition::Ptr(partition2);
    device->partitions = partitions;

    return Device::Ptr(device);
}

Device::Ptr constructDevice3(int deviceNum)
{
    Device* device = new Device();
    device->model = "debug device";
    device->path = QString("/dev/debug%1").arg(deviceNum);
    device->length = 701651888;
    device->sectors = 63;
    device->sector_size = 512;
    device->max_prims = 128;
    device->read_only = false;
    device->table = PartitionTableType::GPT;

    int partitionNo = 1;
    Partition* partition1 = new Partition();
    partition1->device_path = device->path;
    partition1->path = QString("%1p%2").arg(device->path).arg(partitionNo);
    partition1->partition_number = partitionNo;
    partition1->type = PartitionType::Normal;
    partition1->status = PartitionStatus::Real;
    partition1->fs = FsType::Ext4;
    partition1->busy = true;
    partition1->label = QString("debugDevice%1p%2").arg(deviceNum).arg(partitionNo);
    partition1->start_sector = 2048;
    partition1->end_sector = 251656191;
    partition1->sector_size = 512;

    ++partitionNo;
    Partition* partition2 = new Partition();
    partition2->device_path = device->path;
    partition2->path = QString("%1p%2").arg(device->path).arg(partitionNo);
    partition2->partition_number = partitionNo;
    partition2->type = PartitionType::Normal;
    partition2->status = PartitionStatus::Real;
    partition2->fs = FsType::Ext4;
    partition2->busy = false;
    partition2->label = QString("debugDevice%1p%2").arg(deviceNum).arg(partitionNo);
    partition2->start_sector = 251656192;
    partition2->end_sector = 351651839;
    partition2->sector_size = 512;

    ++partitionNo;
    Partition* partition3 = new Partition();
    partition3->device_path = device->path;
    partition3->path = QString("%1p%2").arg(device->path).arg(partitionNo);
    partition3->partition_number = partitionNo;
    partition3->type = PartitionType::Normal;
    partition3->status = PartitionStatus::Real;
    partition3->fs = FsType::Ext4;
    partition3->busy = false;
    partition3->label = QString("debugDevice%1p%2").arg(deviceNum).arg(partitionNo);
    partition3->start_sector = 351651840;
    partition3->end_sector = 501651839;
    partition3->sector_size = 512;

    ++partitionNo;
    Partition* partition4 = new Partition();
    partition4->device_path = device->path;
    partition4->path = QString("%1p-1").arg(device->path);
    partition4->partition_number = -1;
    partition4->type = PartitionType::Unallocated;
    partition4->status = PartitionStatus::Real;
    partition4->fs = FsType::Unknown;
    partition4->busy = false;
    partition4->start_sector = 501651840;
    partition4->end_sector = 701651839;
    partition4->sector_size = 512;

    PartitionList partitions;
    partitions << Partition::Ptr(partition1) << Partition::Ptr(partition2)
               << Partition::Ptr(partition3) << Partition::Ptr(partition4);
    device->partitions = partitions;

    return Device::Ptr(device);
}

}  // namespace installer
