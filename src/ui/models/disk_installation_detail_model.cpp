/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
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

#include "ui/models/disk_installation_detail_model.h"

Q_DECLARE_METATYPE(installer::Device::Ptr)

namespace installer {

DiskInstallationDetailModel::DiskInstallationDetailModel(QObject* parent)
 :   QAbstractListModel(parent)
   , m_index(-1)
{

}

QVariant DiskInstallationDetailModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (!index.isValid()) {
        return QVariant();
    }
    return QVariant::fromValue(m_disks.at(index.row()));
}

int DiskInstallationDetailModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)    
    return m_disks.length();
}

void DiskInstallationDetailModel::setDevices(DeviceList & devices)
{    
    m_all_disks = devices;
    m_disks = m_all_disks;
}

DeviceList & DiskInstallationDetailModel::devices()
{
    return m_all_disks;
}

Device::Ptr DiskInstallationDetailModel::getDevice(int index) const
{
    if (index < 0 || index >= m_disks.count()) {
        return nullptr;
    }

    return m_disks.at(index);
}

void DiskInstallationDetailModel::setSelectedIndex(int index)
{
    m_index = index;
}

int  DiskInstallationDetailModel::selectedIndex()
{
    return m_index;
}

int DiskInstallationDetailModel::getIndex(Device::Ptr device) const
{
    return m_disks.indexOf(device);
}

void DiskInstallationDetailModel::disableIndex(const DiskInstallationTypes::ItemIndexs & indexes)
{
    if (indexes.isEmpty()) {
        return;
    }

    m_disks = m_all_disks;
    for (int i = m_disks.length() - 1; i >= 0; i--) {
        if (indexes.contains(i)) {
            m_disks.removeAt(i);
        }
    }

    m_index = -1;
}

DiskInstallationTypes::ItemIndexs::ItemIndexs(int start, int end)
{
  for (int i = start; i < end; i++) {
      insert(i);
  }
}

}
