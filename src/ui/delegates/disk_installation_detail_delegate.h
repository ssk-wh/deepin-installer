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

#ifndef DISK_INSTALLATION_DETAIL_DELEGATE_H
#define DISK_INSTALLATION_DETAIL_DELEGATE_H

#include "partman/device.h"

#include <QStyledItemDelegate>

class QPainter;

namespace installer {

class DiskInstallationDetailDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:

    struct DeviceSize {
        qint64 length;
        qint64 freespace;
    };

    explicit DiskInstallationDetailDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    static const QString    humanReadableDeviceName(const Device::Ptr & device);
    static const DeviceSize humanReadableDeviceSize(const Device::Ptr & device);
    static const QString    humanReadableDeviceSizeString(const DeviceSize & size);
};

}

#endif // DISK_INSTALLATION_DETAIL_DELEGATE_H
