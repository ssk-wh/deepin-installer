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

#ifndef INSTALLER_PARTMAN_LVM_OPERATION_H
#define INSTALLER_PARTMAN_LVM_OPERATION_H

#include <QDebug>
#include <QList>

#include "partman/operation.h"

namespace installer {
// Abstract class for operations.
class LvmOperation : public Operation {
 public:
  // Create a new operation, with type OperationType::NewPartTable.
  explicit LvmOperation(const Device::Ptr device);

  LvmOperation(OperationType type,
            const Partition::Ptr orig_partition,
            const Partition::Ptr new_partition);
  ~LvmOperation();
  float getDiskCap();
  QSharedPointer<LvPartition> getLvPatition();
  bool CreatePartition();
  bool updatePartition();
  bool deletePartition();
  bool lvinit();
  bool lvresize(float size_gbaty);
  bool lvmkfs();
  bool lvdestroy();
  bool origin_lvdestroy();
  virtual bool applyToDisk();

};

}  // namespace installer

#endif  // INSTALLER_PARTMAN_LVM_OPERATION_H
