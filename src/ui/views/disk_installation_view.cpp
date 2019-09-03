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

#include "ui/views/disk_installation_view.h"
#include "base/file_util.h"
#include "ui/delegates/disk_installation_delegate.h"

namespace installer {

const int kDiskInstallationTypeWidth = 200;

DiskInstallationView::DiskInstallationView(QWidget* parent)
 :QListView(parent)
{
    DiskInstallationItemDelegate* delegate = new DiskInstallationItemDelegate(this);
    setItemDelegate(delegate);
    setObjectName("disk_installation_view");
    setStyleSheet(ReadFile(":/styles/disk_installation_view.css"));
    QSizePolicy list_policy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    list_policy.setHorizontalStretch(0);
    list_policy.setVerticalStretch(0);
    setSizePolicy(list_policy);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedWidth(kDiskInstallationTypeWidth);
    setSelectionRectVisible(false);
    setUniformItemSizes(true);
}

void DiskInstallationView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    if (selected.indexes().size()>0) {
        emit currentSelectedChange(selected.indexes().first().row());
    }
}

}
