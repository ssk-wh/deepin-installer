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

#ifndef INSTALLER_UI_DELEGATES_FULL_DISK_DELEGATE_H
#define INSTALLER_UI_DELEGATES_FULL_DISK_DELEGATE_H

#include <QObject>
#include <partman/operation.h>

#include "partman/device.h"
#include "service/settings_manager.h"

namespace installer {

enum class FullDiskValidateState {
  Ok,
  MaxPrimPartErr,  // All primary partition numbers are used.
  RootMissing,
  RootTooSmall,
};

struct FullDiskPolicy {
        FsType        filesystem;
        QString       mountPoint;
        QString       label;
        QString       usage;
        QString       device;
        bool          alignStart;
        qint64        startSector;
        qint64        endSector;
        qint64        sectors;
        PartitionType partitionType;
};

typedef QList<FullDiskPolicy>  FullDiskPolicyList;

struct FullDiskOption {
    FullDiskPolicyList  policy_list;
    bool                is_system_disk;
};

// Partition delegate used in FullDiskFrame.
class FullDiskDelegate : public QObject {
  Q_OBJECT
 public:
  // Note that this objects of this class does not take ownership of model.
  // Nor does it handle life time of model.
  explicit FullDiskDelegate(QObject* parent = nullptr);

  // Get physical device list.
  const DeviceList& real_devices() const { return real_devices_; }

  // Get virtual device list.
  const DeviceList& virtual_devices() const { return virtual_devices_; }

  // return fake device
  Device::Ptr fullInstallScheme(Device::Ptr device) const;

  // Get alternative partition type. Used while creating a new partition.
  // |partition| is an unallocated partition.
  bool canAddLogical(const Partition::Ptr partition) const;
  bool canAddPrimary(const Partition::Ptr partition) const;

  // Get human readable operation descriptions.
  QStringList getOptDescriptions() const;

  // Returns true if current boot mode is mbr or any system is found on disks
  // with msdos partition table.
  bool isMBRPreferred() const;

  // Check whether device at |device_path| is appropriate for current system.
  bool isPartitionTableMatch(const QString& device_path) const;

  // Get current operation list.
  const OperationList& operations() const { return operations_; }

  // Select |partition| as root partition.
  // It may be separated into two partitions if needed.
  // Call validate() to check whether this |partition| is appropriate before
  // using it.
  void selectPartition(const Partition::Ptr partition);

  // Set boot flag of root partition. Call this before operations() and after
  // validate().
  // Returns false if no appropriate partition can be set as bootable.
  bool setBootFlag();

  // Validate whether selected partition is appropriate.
  FullDiskValidateState validate() const;

  // add System disk
  void addSystemDisk(const QString & device_path);

  // add Data disk
  void addDataDisk(const QString & device_path);

  const QStringList & selectedDisks();

  void removeAllSelectedDisks();

  // format all disks
  bool formatWholeDeviceMultipleDisk();

  void getFinalDiskResolution(FinalFullDiskResolution& resolution);

  const DiskPartitionSetting& settings() const;

private:
  // New version of formatWholeDevice with the support of multiple disks.
  bool formatWholeDeviceV2(const Device::Ptr& device, FullDiskOption& option);

 signals:
  void deviceRefreshed(const DeviceList& devices);

 public slots:
  bool createPartition(const Partition::Ptr partition,
                       PartitionType partition_type,
                       bool align_start,
                       FsType fs_type,
                       const QString& mount_point,
                       qint64 total_sectors,
                       const QString& label=QString(""));
  bool createLogicalPartition(const Partition::Ptr partition,
                              bool align_start,
                              FsType fs_type,
                              const QString& mount_point,
                              qint64 total_sectors,
                              const QString& label = QString(""));
  bool createPrimaryPartition(const Partition::Ptr partition,
                              PartitionType partition_type,
                              bool align_start,
                              FsType fs_type,
                              const QString& mount_point,
                              qint64 total_sectors,
                              const QString& label = QString(""));
  Partition::Ptr deletePartition(const Partition::Ptr partition);
  void formatPartition(const Partition::Ptr partition,
                       FsType fs_type,
                       const QString& mount_point);

  // Create and append operations to whole device at |devices_path|:
  bool formatWholeDevice(const QString& device_path, PartitionTableType type);

  // Save real device list when it is refreshed.
  void onDeviceRefreshed(const DeviceList& devices);

  // Write partitioning settings to file.
  void onManualPartDone(const DeviceList& devices);

  // Clear operation list.
  void resetOperations();

  // Update bootloader settings to |path|.
  void setBootloaderPath(const QString& path);

  // Update swap partition size
  void saveSwapSize();

private:
  // Get auto swap size
  uint getSwapSize() const;

 private:
  DeviceList real_devices_;
  DeviceList virtual_devices_;
  QString bootloader_path_;
  OperationList operations_;
  Partition::Ptr selected_partition_;
  int primaryPartitionLength;

  // device_path_list[0]:SystemDisk, [1]:DataDisk.
  QStringList  selected_disks;
  DeviceList   selected_devices;
  DiskPartitionSetting settings_;
};

}  // namespace installer

#endif  // INSTALLER_UI_DELEGATES_FULL_DISK_DELEGATE_H
