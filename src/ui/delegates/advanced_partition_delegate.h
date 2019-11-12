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

#ifndef INSTALLER_UI_DELEGATES_ADVANCED_PARTITION_DELEGATE_H
#define INSTALLER_UI_DELEGATES_ADVANCED_PARTITION_DELEGATE_H

#include <QObject>
#include "partition_delegate.h"

namespace installer {

enum class PartitionAction {
    CreateLogicalPartition,
    RemoveLogicalPartition,
};

// Partition delegate used in AdvancedPartitionFrame and other sub frame pages.
class AdvancedPartitionDelegate : public partition::Delegate {
  Q_OBJECT
 public:
  explicit AdvancedPartitionDelegate(QObject* parent = nullptr);

  // Get all supported filesystems.
  FsTypeList getFsTypeList() const;

  // Get all supported filesystems for /boot folder.
  FsTypeList getBootFsTypeList() const;

  // Get all available mount points, defined in settings file.
  QStringList getMountPoints() const;

  QList<Device::Ptr> getAllUsedDevice() const;

  // Write partitioning settings to file.
  void onManualPartDone(const DeviceList& devices) override;

  bool unFormatPartition(const Partition::Ptr partition);

  // make sure that extended partition exists here now before call it.
  // return true: success, start_sector & end_sector is new extended partition boundary.
  // return false: failed, start_sector & end_sector is unchanged.
  bool reCalculateExtPartBoundary(PartitionAction action, const Partition::Ptr& current, qint64& start_sector, qint64& end_sector);
  bool reCalculateExtPartBoundary(const PartitionList& partitions, PartitionAction action, const Partition::Ptr& current, qint64& start_sector, qint64& end_sector);
  ValidateStates validate() const override;
  bool isPartitionTableMatch(const QString &device_path) const;
};

}  // namespace installer

#endif  // INSTALLER_UI_DELEGATES_ADVANCED_PARTITION_DELEGATE_H
