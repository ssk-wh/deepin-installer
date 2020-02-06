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

#ifndef DISK_INSTALLATION_DETAIL_VIEW_H
#define DISK_INSTALLATION_DETAIL_VIEW_H

#include <DListView>

DWIDGET_USE_NAMESPACE

namespace installer {

class DiskInstallationDetailView : public DListView
{
Q_OBJECT
public:
    explicit DiskInstallationDetailView(QWidget* parent = nullptr);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

    int currentIndex() const;

signals:
     void currentSelectedChange(int index);

public slots:
    void onDiskListChanged();

private:
    int m_current_index;
};

}

#endif // DISK_INSTALLATION_DETAIL_VIEW_H
