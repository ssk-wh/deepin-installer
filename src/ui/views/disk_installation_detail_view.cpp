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

#include "ui/views/disk_installation_detail_view.h"
#include "base/file_util.h"

namespace installer {

static const int kDiskInstallationDetailWidth = 758;

DiskInstallationDetailView::DiskInstallationDetailView(QWidget* parent)
 :  QListView(parent)
  , m_current_index(0)
{
    setObjectName("disk_installation_view");
    setStyleSheet(ReadFile(":/styles/disk_installation_detail_view.css"));
    QSizePolicy list_policy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    list_policy.setHorizontalStretch(0);
    list_policy.setVerticalStretch(0);
    setSizePolicy(list_policy);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setFixedWidth(kDiskInstallationDetailWidth);
    setSelectionRectVisible(false);
    setUniformItemSizes(true);
}

void DiskInstallationDetailView::onDiskListChanged()
{
    this->reset();
}

void DiskInstallationDetailView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    if (selected.indexes().size()>0) {
        m_current_index = selected.indexes().first().row();
    }
    else {
        m_current_index = -1;
    }
    emit currentSelectedChange(m_current_index);
}

int DiskInstallationDetailView::currentIndex() const
{
    return m_current_index;
}

}