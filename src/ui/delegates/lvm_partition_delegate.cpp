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

#include "ui/delegates/lvm_partition_delegate.h"
#include "ui/delegates/partition_util.h"

namespace installer {

LvmPartitionDelegate::LvmPartitionDelegate(QObject* parent)
    : AdvancedPartitionDelegate (parent) {
    m_islvm = true;
}

void LvmPartitionDelegate::onLvmPartDone(bool ok,const DeviceList& devices) {
    if (ok) {
        install_Lvm_Status = Install_Lvm_Status::Lvm_Install;
    }
    onDeviceRefreshed(devices);
}

Partition* LvmPartitionDelegate::newPartition() {
    return new LvPartition;
}

Partition* LvmPartitionDelegate::newPartition(const Partition &partition) {
    return new LvPartition(*(LvPartition *)(&partition));
}

Operation* LvmPartitionDelegate::newOperation(const Device::Ptr device) {
    return new LvmOperation(device);
}

Operation* LvmPartitionDelegate::newOperation(OperationType type,
                                const Partition::Ptr orig_partition,
                                const Partition::Ptr new_partition) {
    return new LvmOperation(type, orig_partition, new_partition);
}

QStringList LvmPartitionDelegate::getOptDescriptions() const {
    QStringList descriptions;
    for (const Operation::Ptr operation : m_oldOperationList) {
        descriptions.append(operation->description());
    }

    for (const Operation::Ptr operation : operations_) {
        descriptions.append(operation->description());
    }

    return descriptions;
}

}  // namespace installer
