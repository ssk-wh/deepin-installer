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

#ifndef INSTALLER_UI_DELEGATES_LVM_PARTITION_DELEGATE_H
#define INSTALLER_UI_DELEGATES_LVM_PARTITION_DELEGATE_H

#include <QObject>
#include "advanced_partition_delegate.h"

namespace installer {

// Partition delegate used in AdvancedPartitionFrame and other sub frame pages.
class LvmPartitionDelegate : public AdvancedPartitionDelegate {
  Q_OBJECT
 public:
  explicit LvmPartitionDelegate(QObject* parent = nullptr);
  void onLvmPartDone(bool ok,const DeviceList& devices);
  virtual Partition* newPartition();
  virtual Partition* newPartition(const Partition &partition);
  virtual Operation* newOperation(const Device::Ptr device);
  virtual Operation* newOperation(OperationType type,
                                  const Partition::Ptr orig_partition,
                                  const Partition::Ptr new_partition);
  OperationList m_oldOperationList;
};

}  // namespace installer

#endif  // INSTALLER_UI_DELEGATES_LVM_PARTITION_DELEGATE_H
