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

#ifndef INSTALLER_PARTMAN_VG_DEVICE_H
#define INSTALLER_PARTMAN_VG_DEVICE_H

#include <QDebug>
#include <QList>
#include <QSharedPointer>

#include "partman/device.h"
#include "partman/lv_partition.h"

namespace installer {

class VgDevice: public Device {
 public:
  VgDevice(QString name, QStringList pvPath, uint64_t vgsize);
  bool addPv(QString path);
  bool removePv(QString path);
  void update();
  void enableVG(bool enable);
  uint64_t getSize();
  uint64_t getPeSize();
  static VgDevice * p_installer_VgDevice;  
  static VgDevice * installer_VgDevice(PartitionList & partitionList);

  bool m_initOk;
  QString m_vgName;
  QString m_uuid;
  uint64_t m_vgSize;
  uint64_t m_peSize;
  QStringList m_pvPaths;  
};

}  // namespace installer

#endif  // INSTALLER_PARTMAN_VG_DEVICE_H
