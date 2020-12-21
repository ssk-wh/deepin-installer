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

#ifndef MULTIPLE_DISK_INSTALLATION_WIDGET_H
#define MULTIPLE_DISK_INSTALLATION_WIDGET_H

#include "partman/device.h"

#include <DFrame>
#include <DListView>

DWIDGET_USE_NAMESPACE

class QStackedLayout;
class QStringListModel;

namespace installer {

class DiskInstallationDetailView;
class DiskInstallationDetailModel;

enum class DiskModelType : int {
     SystemDisk = 0
   , DataDisk
   , DiskModelCount
};

static const int kDiskModelMaxCount = static_cast<int>(DiskModelType::DiskModelCount);

class MultipleDiskInstallationWidget : public DFrame
{
    Q_OBJECT
public:
    explicit MultipleDiskInstallationWidget(QWidget *parent = nullptr);    

    bool focusSwitch();
    bool doSpace();
    bool doSelect();
    bool directionKey(int keyvalue);
    bool isLeftRightWidgetHasFocus();

signals:
    void currentDeviceChanged(int type, const Device::Ptr device);

public slots:
    void onDeviceListChanged(const DeviceList& devices);

private slots:
    void onInstallationSelectedChanged(const QModelIndex &index);
    void onInstallationDetailSelectedChanged(int index);

private:
    void initConnections();
    void initUI();
    void changeEvent(QEvent* event) override;
    const QStringList getDiskTypes();

private:
    DListView* m_left_view = nullptr;
    QStringListModel* m_left_model = nullptr;
    DiskInstallationDetailView* m_right_view = nullptr;
    DiskInstallationDetailModel* m_right_model[kDiskModelMaxCount] = {nullptr, nullptr};
    int m_current_left_index = -1;
    DeviceList  m_devices;
};

}

#endif // MULTIPLE_DISK_INSTALLATION_WIDGET_H
