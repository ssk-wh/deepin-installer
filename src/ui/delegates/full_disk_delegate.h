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

#include "partition_delegate.h"

namespace installer {

struct FullDiskPolicy {
        FsType        filesystem;
        QString       mountPoint;
        QString       label;
        QString       usage;
        QString       device;
        bool          alignStart;
        bool          isDataPartition = false;
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

struct SizeRange {
   qint64 min_size_bytes;
   qint64 max_size_bytes;
};

struct PartitionAdjustOption {
   int root_size_count; // policy count of usage=root-size
   int percent100_count; // policy count of usage=100%
   qint64 start_offset_sector; //start sector of first policy
   qint64 free_bytes; // free space left (bytes) by policy configuration
   SizeRange root_size_range; // size range of root policy
};

// Partition delegate used in FullDiskFrame.
class FullDiskDelegate : public partition::Delegate{
  Q_OBJECT
 public:
  // Note that this objects of this class does not take ownership of model.
  // Nor does it handle life time of model.
  explicit FullDiskDelegate(QObject* parent = nullptr);

  // add System disk
  void addSystemDisk(const QString & device_path);

  // add Data disk
  void addDataDisk(const QString & device_path);

  const QStringList & selectedDisks();

  void removeAllSelectedDisks();

  // format all disks
  bool formatWholeDeviceMultipleDisk();

  void getFinalDiskResolution(FinalFullDiskResolution& resolution);

  void setAutoInstall(bool autoinstall);

public Q_SLOTS:
  void onDeviceRefreshed(const DeviceList& devices) override;

Q_SIGNALS:
  void requestAutoInstallFinished(bool finished) const;

private:
  // get root partition size range from settings
  const SizeRange getRootPartitionSizeRange();

  // build new partition data from sorted full disk policy
  void adjustFullDiskPolicy(const Device::Ptr& device, FullDiskOption& option, const PartitionAdjustOption& adjust);

  // New version of formatWholeDevice with the support of multiple disks.
  bool formatWholeDeviceV2(const Device::Ptr& device, FullDiskOption& option);

 public slots:

  // Create and append operations to whole device at |devices_path|:
  bool formatWholeDevice(const QString& device_path, PartitionTableType type);

  // Write partitioning settings to file.
  void onManualPartDone(const DeviceList& devices) override;

  // Update swap partition size
  void saveSwapSize();

private:
  // Get auto swap size
  uint getSwapSize() const;

private:
  bool m_autoInstall;
};

}  // namespace installer

#endif  // INSTALLER_UI_DELEGATES_FULL_DISK_DELEGATE_H
