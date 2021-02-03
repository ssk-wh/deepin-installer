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

#include "partman/lv_partition.h"

#include <QRegularExpression>

#include "base/command.h"

namespace installer {

LvPartition::LvPartition() : m_initOk(false) {
    is_lvm = true;
}

LvPartition::LvPartition(const LvPartition &partition)
    : Partition(partition),
    m_initOk(partition.m_initOk),
    m_vgName(partition.m_vgName) {
    is_lvm = true;
}

LvPartition::~LvPartition(){

}

Partition* LvPartition::newPartition() {
    //TODO: 目前有此方法异议，日后在想更完美的解决办法
    return new LvPartition(*this);
}

void LvPartition::changeNumber(int partition_number) {
    this->partition_number = partition_number;
    this->path = QString("%1/lv%2").arg(this->device_path).arg(partition_number);
    this->name = QString("lv%1").arg(partition_number);
}

}  // namespace installer
