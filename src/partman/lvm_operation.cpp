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

#include "partman/lvm_operation.h"

#include <QDebug>
#include <QDateTime>

#include <memory>

#include "partman/libparted_util.h"
#include "partman/partition_format.h"
#include "ui/delegates/partition_util.h"
#include "base/command.h"

namespace installer {
LvmOperation::LvmOperation(const Device::Ptr device)
    : Operation(device) {
}

LvmOperation::LvmOperation(OperationType type,
                     const Partition::Ptr orig_partition,
                     const Partition::Ptr new_partition)
    : Operation(type, orig_partition, new_partition) {
}

LvmOperation::~LvmOperation() {
}

float LvmOperation::getDiskCap() {
   return static_cast<float>(new_partition->getByteLength()) / static_cast<float>(kGibiByte);
}

QSharedPointer<LvPartition> LvmOperation::getLvPatition() {
    return new_partition.staticCast<LvPartition>();
}

bool LvmOperation::CreatePartition() {
    QDateTime timedt = QDateTime::currentDateTime();
    QString timeStr = timedt.toString("yyyyMMddhhmmsszzz");   
    VgDevice::p_installer_VgDevice->update();
    lvinit();

    return lvmkfs();
}

bool LvmOperation::updatePartition() {
    lvresize(getDiskCap());

    return lvmkfs();
}

bool LvmOperation::deletePartition() {
    return origin_lvdestroy();
}

bool LvmOperation::lvinit() {
    QSharedPointer<LvPartition>  lvpartition = getLvPatition();
    QStringList pathlist = device->path.split("/");
    QString vgname = pathlist.back();    
    if (!lvpartition->m_initOk) {
        QStringList args("-l");
        float per = static_cast<float>(lvpartition->getByteLength()) / static_cast<float>(VgDevice::p_installer_VgDevice->getSize());
        int pe_size = per * VgDevice::p_installer_VgDevice->getPeSize();
        args.append(QString::number(pe_size));
        args.append("-n");
        args.append(lvpartition->path);
        args.append(vgname);
        args.append("-y");
        QString output;
        SpawnCmd("lvcreate", args, output);
        lvpartition->m_vgName = vgname;       
        lvpartition->m_initOk = true;
    }

    return lvpartition->m_initOk;
}


bool LvmOperation::lvresize(float size_gbaty) {
    QSharedPointer<LvPartition>  lvpartition = getLvPatition();
    if (!lvpartition->m_initOk)  return false;

    QStringList args("-L");
    args.append(QString::number(size_gbaty)+"G");
    args.append(lvpartition->path);
    QString output;
    SpawnCmd("lvresize", args, output);
    QStringList msglinelist = output.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    QString ok_flag = "successfully";
    bool isok = false;
    for (QString msg : msglinelist) {
       if (msg.indexOf(ok_flag) > -1) {
           isok = true;
           break;
       }
    }

    return isok;
}

bool LvmOperation::lvmkfs() {
    QSharedPointer<LvPartition>  lvpartition = getLvPatition();
    if (!lvpartition->m_initOk) return false;

    if (!Mkfs(lvpartition)) {
      qCritical() << "Lvm Mkfs() failed:" << lvpartition;
      return false;
    }

    return true;
}

bool LvmOperation::lvdestroy() {
    QSharedPointer<LvPartition>  lvpartition = getLvPatition();
    if (!lvpartition->m_initOk)
        return false;

    QStringList args(lvpartition->path);
    args.append("-y");
    QString output;
    SpawnCmd("lvremove", args, output);
    QStringList msglinelist = output.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    QString ok_flag = "successfully";
    bool isok = false;
    for (QString msg : msglinelist) {
       if (msg.indexOf(ok_flag) > -1) {
           isok = true;
           break;
       }
    }

    return isok;
}

bool LvmOperation::origin_lvdestroy() {
    QSharedPointer<LvPartition>  lvpartition = orig_partition.staticCast<LvPartition>();

    QStringList args(lvpartition->path);
    args.append("-y");
    QString output;
    SpawnCmd("lvremove", args, output);
    QStringList msglinelist = output.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    QString ok_flag = "successfully";
    bool isok = false;
    for (QString msg : msglinelist) {
       if (msg.indexOf(ok_flag) > -1) {
           isok = true;
           break;
       }
    }

    return isok;
}

bool LvmOperation::applyToDisk() {
    switch (type) {
      case OperationType::Create: {
        // Filters filesystem type.
        if (new_partition->fs == FsType::Unknown) {
          qCritical() << "OperationCreate unknown fs" << new_partition;
          return false;
        }
        return CreatePartition();
      }

      case OperationType::Delete: {
        return deletePartition();
      }

      case OperationType::Format: {
        // Filters filesystem type.
        if (new_partition->fs == FsType::Unknown) {
          qCritical() << "OperationFormat unknown fs" << new_partition;
          return false;
        }
        return updatePartition();
      }

      case OperationType::Invalid: {
        qCritical() << "Invalid operation!";
        return false;
      }

      case OperationType::MountPoint: {
        return true;
      }

      case OperationType::NewPartTable: {
        return true;
      }

      case OperationType::Resize: {
        return updatePartition();
      }

      default: {
        qCritical() << "Unknown type of operation:" << type;
        return false;
      }
    }
}

}  // namespace installer
