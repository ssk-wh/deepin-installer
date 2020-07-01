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

#include "partman/vg_device.h"

#include <QDateTime>
#include <QDir>

#include "base/command.h"

namespace installer {

VgDevice::VgDevice(QString name, QStringList pvPath, uint64_t vgsize) {
    m_vgName = name;
    m_pvPaths = pvPath;
    m_vgSize = vgsize;
    device_type = DeviceType::VGDevice;
    m_initOk = false;
    m_peSize = 0;
}

bool VgDevice::addPv(QString path) {
    QStringList args(m_vgName);
    args.append(path);

    return SpawnCmd("vgextend", args);
}

bool VgDevice::removePv(QString path) {
    QStringList args(m_vgName);
    args.append(path);

    return SpawnCmd("vgreduce", args);
}

void VgDevice::update() {
    if (m_initOk) return ;

    QStringList args(m_vgName);
    args.append(m_pvPaths);
    SpawnCmd("vgcreate", args);

    args.clear();
    args.append("-v");
    args.append(m_vgName);
    QString output;
    SpawnCmd("vgdisplay", args, output);
    QStringList msglinelist = output.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    QStringList flaglist({"VG UUID", "Free  PE / Size"});
    for (QString msg : msglinelist) {
        for (QString flag : flaglist) {
            int ret_index= msg.indexOf(flag);
            if (ret_index > -1) {
                msg.replace(flag,"");
                msg.replace("\n","");
                msg.replace(" ","");
                if (flag == "VG UUID") {
                    m_uuid = msg;
                } else if (flag == "Free  PE / Size") {
                    m_peSize = msg.split("/").at(0).toULong();
                }
            }
        }
    }

    m_initOk = true;

}

void VgDevice::enableVG(bool enable) {


    //判断pv是否已经分配给其他vg
    bool ret = false;

    while (!ret) {
        const QString cmd { "vgchange" };
        const QStringList args { "-a", enable ? "y" : "n", m_vgName };
        QString output { "" };
        QString error  { "" };
        if (!SpawnCmd(cmd, args, output, error)) {
            qWarning() << QString("EnableVG:Failed to enable vg(%1)").arg(enable);
            if (!error.isEmpty()) {
                 qWarning() << QString("EnableVG:{%1}").arg(error);
                 enableVG(enable);
            }
        }

        QDir dir( "/dev/mapper/");
        QFileInfoList fileInfoList = dir.entryInfoList();
        for (QFileInfo fileInfo : fileInfoList) {
           if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")  continue;
               QString vglvName = fileInfo.absoluteFilePath();
               if (vglvName.indexOf(m_vgName) > -1) {
                   ret = true;
                   break;
               }
        }

        if (!output.isEmpty()) {
            qInfo() << QString("EnableVG:{%1}").arg(output);
        }
    }

}

uint64_t VgDevice::getSize() {
    return m_vgSize;
}

uint64_t VgDevice::getPeSize() {
    return m_peSize;
}

VgDevice * VgDevice::p_installer_VgDevice = nullptr;

VgDevice * VgDevice::installer_VgDevice(PartitionList & partitionList) {
    if (!p_installer_VgDevice) {
        QStringList pvPathList_ok;
        uint64_t vgsize(0);

        for (Partition::Ptr partition : partitionList) {
            pvPathList_ok.append(partition->path);
            vgsize += partition->getByteLength();
        }
        //vg name = "vg"+"时间戳"+"随机数"
        QDateTime timedt = QDateTime::currentDateTime();
        QString timeStr = timedt.toString("yyyyMMddhhmmsszzz");
        QString vgName = "vg" + timeStr + QString::number(rand()%100);
        p_installer_VgDevice = new VgDevice(vgName, pvPathList_ok, vgsize);
    }

    return p_installer_VgDevice;
}

}  // namespace installer
