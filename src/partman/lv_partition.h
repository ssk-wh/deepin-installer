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

#ifndef INSTALLER_PARTMAN_LV_PARTITION_H
#define INSTALLER_PARTMAN_LV_PARTITION_H

#include <QDebug>
#include <QList>
#include <QStringList>
#include <QSharedPointer>

#include "partman/fs.h"
#include "partman/structs.h"
#include "partman/partition.h"

namespace installer {

class LvPartition : public Partition {
 public:
  LvPartition();
  LvPartition(const LvPartition &partition); 
  virtual ~LvPartition();
  virtual Partition* newPartition();
  virtual void changeNumber(int partition_number);
  bool m_initOk;  
  QString m_vgName;
};

}  // namespace installer

#endif  // INSTALLER_PARTMAN_LV_PARTITION_H
