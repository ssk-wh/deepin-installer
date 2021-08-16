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

#include "base/command.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/partition_util.h"

#include <sys/sysinfo.h>
#include <math.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

namespace installer {

namespace {

const char kLinuxSwapMountPoint[] = "linux-swap";
const char kUEFIPartitionLabel[] = "EFI";
const char kFullDiskPolicyRootb[] = "Rootb";
const char kFullDiskPolicyUsageRootSize[] = "root-size";
const char kPartitionFullDiskRootPartitionUsage[] = "partition_full_disk_root_partition_usage";

}  // namespace

FullDiskDelegate::FullDiskDelegate(QObject* parent)
    : partition::Delegate(parent)
    , m_autoInstall(false)
{
    this->setObjectName("full_disk_delegate");
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
  Operation::Ptr operation(newOperation(new_device));
  operations_.append(operation);
  // Update virtual device property at the same time.
  operation->applyToVisual(device);

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

  if (device->table == PartitionTableType::GPT && GetSettingsBool(kSystemIsCreateEFI)) {
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

void FullDiskDelegate::onManualPartDone(const DeviceList& devices) {
  qDebug() << "FullDiskDelegate::onManualPartDone()" << devices;
  QString root_disk;
  QString root_path;
  QStringList mount_points;
  bool found_swap = false;
  QString esp_path;
  Device::Ptr root_device;

  // Check use-specified partitions with mount point.
  for (const Device::Ptr &device : devices) {
      //删除 lvm  逻辑卷时会出现设备为空的情况
      if (!device) continue;

    for (const Partition::Ptr &partition : device->partitions) {
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

    int size = qRound(sqrt(by) + qRound(by));

    // Limit swap to 16G
    // see: https://tower.im/teams/9487/todos/232339/
    return std::min(static_cast<uint>(16), static_cast<uint>(size));
}

bool FullDiskDelegate::formatWholeDeviceMultipleDisk()
{
    resetOperations();

    PartitionTableType table;
    table = IsEfiEnabled() ? PartitionTableType::GPT : PartitionTableType::MsDos;

    FullDiskOption disk_option;
    FullDiskPolicyList & policy_list = disk_option.policy_list;

    QStringList device_path_list;
    // Get All policies from json file.
    {
        const QByteArray& policyStr{ GetFullDiskInstallPolicy() };
        if (policyStr.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "policy is empty";
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
            policy.label      = jsonObject["label"].toString();
            policy.usage      = jsonObject["usage"].toString();
            policy.alignStart = jsonObject["alignStart"].toBool();
            policy.device     = jsonObject["device"].toString();
            policy.isDataPartition = jsonObject["isDataPartition"].toBool();
            policy.isLvm      = jsonObject["isLvm"].toBool();
            if (policy.mountPoint == kLinuxSwapMountPoint) {
                policy.mountPoint = "";
            }
            policy_list.push_back(policy);

            if (!policy.device.isEmpty() && device_path_list.indexOf(policy.device) == -1) {
                device_path_list.append(policy.device);
            }
        }
    }

    if (device_path_list.isEmpty()) {
        device_path_list = selected_disks;
    }

    if (device_path_list.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "select Disk is Empty";
        return false;
    }

    for (QString device_path : device_path_list) {
        int device_index = DeviceIndex(virtual_devices_, device_path);
        if (device_index == -1) {
            qWarning() << Q_FUNC_INFO << "not find device: " << device_path;
            return false;
        }
    }

    // Change data disk configuration.
    QString system_data_mount_point;
    for (int i = 0; i < policy_list.length(); i++) {
        FullDiskPolicy & policy = policy_list[i];

        // 计算数据分区挂载点位置，以json中配置的第一个数据分区的mountPoint为优先级最高
        // 其中判断policy.label == _dde_data 是为了向老版本兼容
        if (system_data_mount_point.isEmpty()
                && (policy.isDataPartition || policy.label == "_dde_data")) {
            system_data_mount_point = policy.mountPoint;
            WriteSystemDataMountPoint(system_data_mount_point);
        }

        // 如果json的device字段没有配置，则根据配置或者界面选择的设备设置，否则使用json中的device字段
        // 默认json的device字段为空
        if (device_path_list.length() > 1
            && policy.device.isEmpty()
            && policy.isDataPartition) {

            policy.device = device_path_list.at(1);
        }
        else if (policy.device.isEmpty()){
            policy.device = device_path_list.at(0);
        }
    }

    m_FullDiskPolicyList = policy_list;

    selected_devices.clear();
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
    Operation::Ptr operation(newOperation(device));
    operations_.append(operation);
    operation->applyToVisual(device);

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

    if (IsEfiEnabled() && option.is_system_disk && GetCurrentPlatform() != "sw" && GetSettingsBool(kSystemIsCreateEFI)) {
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

        unallocated = device->partitions.last();
        startSector += sectors;
        adjust_start_offset_sector += sectors;
    }

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

        else if (policy.usage == "backup-size") {
            //恢复分区大小=总分区大小的2%+10GB
            qint64  backSize = length * 2 / 100 / kGibiByte + GetSettingsInt(kSystemRecoveryBaseSize);
            const QString backup_size_str = QString("%1gib").arg(backSize);
            partitionSize = ParsePartitionSize(backup_size_str,length);

        } else if (policy.usage == "100%") {
            percent100_count++;

        } else if (policy.usage == kFullDiskPolicyUsageRootSize) {
            // root_siez 大小一般为磁盘的20%,并且在partition_full_disk_large_root_part_range配置的范围内
            const QString root_size_str = GetSettingsString(kPartitionFullDiskRootPartitionUsage);
            partitionSize = ParsePartitionSize(root_size_str, device->getByteLength());


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

        bool is_primary = ((device->table == PartitionTableType::GPT) ||
                           (primary_count < (device->max_prims - 1)));
        policy.partitionType =
            is_primary ? PartitionType::Normal : PartitionType::Logical;

        if (is_primary) {
            primary_count++;
        }

        if (policy.mountPoint == kMountPointRoot || policy.label == kFullDiskPolicyRootb) {
            root_size_count++;
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

        qDebug() << "zdd test: " << policy.mountPoint << policy.sectors
                 << policy.label;
        // alignStart is always true ,since policies are sorted by start_sector
        if (!createPartition(unallocated, policy.partitionType, true, policy.filesystem,
                    policy.mountPoint, policy.sectors,policy.label)) {
            return false;
        }

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

void FullDiskDelegate::addSystemDisk(const QString &device_path)
{
    selected_devices.clear();
    selected_disks.clear();
    selected_disks.append(device_path);


    qInfo() << Q_FUNC_INFO << "add system disk: " << device_path;
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

    qInfo() << Q_FUNC_INFO << "add data disk: " << device_path;
}

const QStringList & FullDiskDelegate::selectedDisks()
{
    return selected_disks;
}

void FullDiskDelegate::getFinalDiskResolution(FinalFullDiskResolution& resolution)
{
    resolution.option_list.clear();
    FinalFullDiskOption option;
    for (Operation::Ptr op : operations()) {
        if (OperationType::Create != op->type || FsType::Empty == op->new_partition->fs) {
            continue;
        }

        FinalFullDiskPolicy policy;
        policy.filesystem = GetFsTypeName(op->new_partition->fs);
        policy.mountPoint = op->new_partition->mount_point;
        policy.label = op->new_partition->label;
        policy.offset = op->new_partition->start_sector * op->new_partition->sector_size;
        policy.size = (op->new_partition->end_sector - op->new_partition->start_sector) *
                op->new_partition->sector_size;
        policy.device = op->new_partition->device_path;
        policy.isLvm = this->isLvm(op->new_partition->mount_point);

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

void FullDiskDelegate::setAutoInstall(bool autoinstall)
{
    m_autoInstall = autoinstall;
}

bool FullDiskDelegate::isLvm(const QString &mountPoint)
{
    for (int i = 0; i < m_FullDiskPolicyList.size(); i++) {
        if (m_FullDiskPolicyList.at(i).mountPoint == mountPoint) {
            return m_FullDiskPolicyList.at(i).isLvm;
        }
    }

    return false;
}

bool FullDiskDelegate::isLvm()
{
    for (int i = 0; i < m_FullDiskPolicyList.size(); i++) {
        qDebug() << "m_FullDiskPolicyList: " << m_FullDiskPolicyList.at(i).label << "  " << m_FullDiskPolicyList.at(i).isLvm;
        if (m_FullDiskPolicyList.at(i).isLvm) {
            return true;
        }
    }

    return false;
}

void FullDiskDelegate::onDeviceRefreshed(const DeviceList &devices)
{
    partition::Delegate::onDeviceRefreshed(devices);

    if (!m_autoInstall) {
        return;
    }

    if (virtual_devices_.isEmpty()) {
        qWarning() << "device list is empty!";
        return;
    }

    const qint64 root_required = GetSettingsInt(kPartitionFullDiskMiniSpace);
    const qint64 root_required_bytes = kGibiByte * root_required;

    QMap<Device::Ptr, qint64> deviceSpeedMap;
    QList<Device::Ptr> deviceList;

    //如果有指定设备，则使用指定的设备
    QStringList devicelistfromset = GetSettingsStringList("DI_FULLDISK_MULTIDISK_DEVICE");
    if(devicelistfromset.size() > 0) {
        foreach(QString testdevicepath, devicelistfromset) {
            foreach (Device::Ptr device, virtual_devices_) {
                if(!testdevicepath.compare(device->path)) {
                    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
                    deviceSpeedMap[device] = QDateTime::currentMSecsSinceEpoch() - currentTime;
                    deviceList << device;
                    qWarning() << Q_FUNC_INFO << "DI_FULLDISK_MULTIDISK_DEVICE Device:"<< device->path;
                }
            }
        }
    } else {
        for (Device::Ptr device : virtual_devices_) {
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            SpawnCmd("dd", {QString("if=%1").arg(device->path), QString("of=%1").arg(device->path), "bs=4k", "count=5120", "iflag=direct,nonblock", "oflag=direct,nonblock"});
            deviceSpeedMap[device] = QDateTime::currentMSecsSinceEpoch() - currentTime;
            deviceList << device;
            qWarning() << Q_FUNC_INFO << "Device:"<< device->path <<" time use :" << deviceSpeedMap[device] << "data size 4k * 5120";
        }

        //排序
        if (deviceList.length() > 1) {
            std::sort(deviceList.begin(), deviceList.end(), [=](Device::Ptr p1, Device::Ptr p2) {
                return deviceSpeedMap[p1] < deviceSpeedMap[p2];
            });
        }
    }

    //获取所有符合条件的硬盘
    for (auto it = deviceList.begin(); it != deviceList.end();) {
        Device::Ptr device = *it;
        if (device->getByteLength() >= root_required_bytes) {
            ++it;
        }
        else {
            it = deviceList.erase(it);
        }
    }

    if (deviceList.size() == 0) {
        qWarning() << Q_FUNC_INFO << "not found system disk, please check!";
    } else {
        addSystemDisk(deviceList.first()->path);
        qDebug()<<"add system disk path :"<< deviceList.first()->path;
        deviceList.erase(deviceList.begin());
    }

    for (auto it = deviceList.begin(); it != deviceList.end();) {
        Device::Ptr device = *it;
        addDataDisk(device->path);//每次将包含有此项的删除掉，再次添加此项
        it = deviceList.erase(it);
    }

    emit requestAutoInstallFinished(formatWholeDeviceMultipleDisk());
}

const SizeRange FullDiskDelegate::getRootPartitionSizeRange()
{
    const QString root_range_str = GetSettingsString(kPartitionFullDiskLargeRootPartRange);
    const QStringList root_ranges = root_range_str.split(":", QString::SkipEmptyParts);

    int min_root_size_gib { GetSettingsInt(kPartitionRootMiniSpace) };
    int max_root_size_gib { GetSettingsInt(kPartitionRootMiniSpace) };

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
            //            partitionSize = std::max(partitionSize, adjust.root_size_range.min_size_bytes);
            //            partitionSize = std::min(partitionSize, adjust.root_size_range.max_size_bytes);
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
