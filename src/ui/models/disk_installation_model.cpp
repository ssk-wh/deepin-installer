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

#include "ui/models/disk_installation_model.h"

namespace installer {

DiskInstallationModel::DiskInstallationModel(QObject *parent)
 :QAbstractListModel (parent)
{
    m_disks.append(QString(tr("SystemDisk")));
    m_disks.append(QString(tr("DataDisk")));
}

QVariant DiskInstallationModel::data(const QModelIndex &index, int role ) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (!index.isValid()) {
        return QVariant();
    }
    return m_disks.at(index.row());
}

int DiskInstallationModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_disks.length();
}

}
