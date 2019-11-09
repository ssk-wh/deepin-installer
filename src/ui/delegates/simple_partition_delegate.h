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

#ifndef INSTALLER_UI_DELEGATES_SIMPLE_PARTITION_DELEGATE_H
#define INSTALLER_UI_DELEGATES_SIMPLE_PARTITION_DELEGATE_H

#include "ui/delegates/partition_delegate.h"

namespace installer {

enum class SimpleValidateState {
  Ok,
  MaxPrimPartErr,  // All primary partition numbers are used.
  RootMissing,
  RootTooSmall,
};

// Partition delegate used in SimplePartitionFrame.
class SimplePartitionDelegate : public partition::Delegate {
  Q_OBJECT
 public:
  // Note that this objects of this class does not take ownership of model.
  // Nor does it handle life time of model.
    explicit SimplePartitionDelegate(QObject* parent = nullptr);
    ValidateStates validate() const override;
    bool formatWholeDevice(const QString &device_path, PartitionTableType type);
    void onManualPartDone(const DeviceList &devices) override;
};

}  // namespace installer

#endif  // INSTALLER_UI_DELEGATES_SIMPLE_PARTITION_DELEGATE_H
