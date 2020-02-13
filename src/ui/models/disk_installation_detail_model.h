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

#ifndef DISK_INSTALLATION_DETAIL_MODEL_H
#define DISK_INSTALLATION_DETAIL_MODEL_H

#include <QAbstractListModel>
#include <QSet>

#include "partman/device.h"

namespace installer {

class DiskInstallationTypes
{
public:

    typedef QSet<int> IntegerSet;

    class ItemIndexs : public IntegerSet
    {
    public:
        using IntegerSet::IntegerSet;

        // fill integers: [start, end)
        ItemIndexs(int start, int end);
    };

enum DeviceConflictPolicy
{
      Ignore    = 0x0U
    , Disabled
    , Invisible
    , Default  = Disabled
};

};

class DiskInstallationDetailModel : public QAbstractListModel
{
Q_OBJECT


public:
    explicit DiskInstallationDetailModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent) const override;
    void setDevices(DeviceList & devices);
    DeviceList & devices();
    DeviceList & virtualDevices();
    void setSelectedIndex(int index);
    int  selectedIndex();
    void disableIndex(const DiskInstallationTypes::ItemIndexs & indexes);

signals:
    void diskListChanged();

private:
    DeviceList     m_all_disks;
    DeviceList     m_disks;
    int            m_index = -1;
};

}

#endif // DISK_INSTALLATION_DETAIL_MODEL_H
