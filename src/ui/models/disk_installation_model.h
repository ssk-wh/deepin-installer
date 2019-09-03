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

#ifndef DISKINSTALLATIONMODEL_H
#define DISKINSTALLATIONMODEL_H

#include <QAbstractListModel>
#include <QStringList>

namespace installer {

class DiskInstallationModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit DiskInstallationModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent) const override;

signals:

public slots:

private:
    QStringList          m_disks;
};

}

#endif // DISKINSTALLATIONMODEL_H
