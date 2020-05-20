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

#include "ui/models_cli/partition_type_model.h"

namespace installer {

PartitionTypeModel::PartitionTypeModel(QObject* parent)
    : QObject(parent),
      is_logical_visible_(true),
      is_primary_visible_(true) {
  this->setObjectName("partition_type_model");
}

int PartitionTypeModel::getLogicalIndex() const {
  // Check whether logical partition is visible.
  if (is_logical_visible_) {
    return is_primary_visible_ ? 1 : 0;
  }
  return -1;
}

int PartitionTypeModel::getPrimaryIndex() const {
  // Check whether primary partition is visible.
  return is_primary_visible_ ? 0 : -1;
}

bool PartitionTypeModel::isLogical(int index) const {
  // First make sure that logical partition is visible.
  if (is_logical_visible_) {
    return is_primary_visible_ ? (index == 1) : (index == 0);
  }
  return false;
}

bool PartitionTypeModel::isPrimary(int index) const {
  // First make sure that primary partition is visible.
  return is_primary_visible_ ? (index == 0) : false;
}

void PartitionTypeModel::reset() { 
  is_logical_visible_ = true;
  is_primary_visible_ = true;

}

void PartitionTypeModel::setLogicalVisible(bool visible) { 
  is_logical_visible_ = visible; 
}

void PartitionTypeModel::setPrimaryVisible(bool visible) {
   is_primary_visible_ = visible;
}

}  // namespace installer
