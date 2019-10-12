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

#include <QStackedLayout>
#include <QSizePolicy>
#include <QStringListModel>
#include <vector>
#include <initializer_list>

#include "ui/widgets/multiple_disk_installation_widget.h"
#include "ui/views/disk_installation_view.h"
#include "ui/views/disk_installation_detail_view.h"
#include "ui/models/disk_installation_detail_model.h"
#include "ui/delegates/disk_installation_delegate.h"
#include "ui/delegates/disk_installation_detail_delegate.h"

namespace installer {

MultipleDiskInstallationWidget::MultipleDiskInstallationWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnections();
}

void MultipleDiskInstallationWidget::initConnections()
{
    connect(m_left_view, &DiskInstallationView::currentSelectedChange,
            this, &MultipleDiskInstallationWidget::onInstallationSelectedChanged);
    for (int i = 0; i < kDiskModelMaxCount; i++) {
      connect(m_right_view[i], &DiskInstallationDetailView::currentSelectedChange,
              this, &MultipleDiskInstallationWidget::onInstallationDetailSelectedChanged);
      connect(m_right_model[i], &DiskInstallationDetailModel::diskListChanged,
              m_right_view[i], &DiskInstallationDetailView::onDiskListChanged);
    }
}

void MultipleDiskInstallationWidget::initUI()
{
    QSizePolicy size_policy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setSizePolicy(size_policy);

    QHBoxLayout * hboxlayout = new QHBoxLayout();
    QVBoxLayout * leftlayout = new QVBoxLayout();
    QVBoxLayout * rightlayout = new QVBoxLayout();

    m_left_model = new QStringListModel(getDiskTypes());
    m_left_view = new DiskInstallationView();
    DiskInstallationItemDelegate* delegate = new DiskInstallationItemDelegate(m_left_view);
    m_left_view->setItemDelegate(delegate);
    m_left_view->setModel(m_left_model);
    m_right_layout = new QStackedLayout();
    for (int i = 0; i < kDiskModelMaxCount; i++) {
        m_right_model[i] = new DiskInstallationDetailModel();
        m_right_view[i] = new DiskInstallationDetailView();
        DiskInstallationDetailDelegate* delegate = new DiskInstallationDetailDelegate(m_right_view[i]);
        m_right_view[i]->setItemDelegate(delegate);
        m_right_view[i]->setModel(m_right_model[i]);
        m_right_layout->addWidget(m_right_view[i]);
    }
    m_right_layout->setCurrentIndex(0);
    m_right_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    leftlayout->setMargin(0);
    leftlayout->setSpacing(0);
    leftlayout->setContentsMargins(0, 0, 0, 0);
    leftlayout->addWidget(m_left_view, 0, Qt::AlignLeft);
    leftlayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    rightlayout->setMargin(0);
    rightlayout->setSpacing(0);
    rightlayout->setContentsMargins(0, 0, 0, 0);
    rightlayout->addLayout(m_right_layout);
    hboxlayout->addLayout(leftlayout);
    hboxlayout->addSpacing(1);
    hboxlayout->addLayout(rightlayout, 1);
    hboxlayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    main_layout->addLayout(hboxlayout, 1);
    setLayout(main_layout);
    main_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
}

void MultipleDiskInstallationWidget::onDeviceListChanged(const DeviceList& devices)
{
   m_devices.clear();
   for (const Device::Ptr& device : devices) {
      bool partition_busy = false;
      for (Partition::Ptr partition : device->partitions) {
        if (partition->busy) {
          partition_busy = true;
          break;
        }
      }
      if (partition_busy) {
        continue;
      }
      m_devices.append(device);
   }

   for (int i = 0; i < kDiskModelMaxCount; i++) {
        m_right_model[i]->setDevices(m_devices);
        m_right_view[i]->setCurrentIndex(m_right_model[i]->index(-1, 0));
        m_right_model[i]->setSelectedIndex(-1);
   }

   m_right_model[static_cast<int>(DiskModelType::DataDisk)]->disableIndex(DiskInstallationTypes::ItemIndexs{ -1 });
   m_left_view->setCurrentIndex(m_left_model->index(0, 0));
}

void MultipleDiskInstallationWidget::onInstallationSelectedChanged(int index)
{
    if (index < 0) {
        return;
    }

    int current_detail_index;
    m_current_left_index = index;
    current_detail_index = m_right_model[index]->selectedIndex();

    // all disks are unavailable for data disk before system disk is selected
    // you can select any other disk for data disk, except the one which is selected as system disk
    if (static_cast<int>(DiskModelType::DataDisk) == m_current_left_index) {
        int sys_index = m_right_model[static_cast<int>(DiskModelType::SystemDisk)]->selectedIndex();
        if (-1 == sys_index) {
            m_right_model[index]->disableIndex(DiskInstallationTypes::ItemIndexs(0, m_right_model[index]->devices().length()));
        }
        else {
            m_right_model[index]->disableIndex(DiskInstallationTypes::ItemIndexs { sys_index });
        }
    }

    m_right_view[index]->setCurrentIndex(m_right_model[index]->index(current_detail_index, 0));
    m_right_layout->setCurrentIndex(index);
}

void MultipleDiskInstallationWidget::onInstallationDetailSelectedChanged(int index)
{
   m_right_model[m_current_left_index]->setSelectedIndex(index);
   if (-1 == index) {
       return;
   }

   if (static_cast<int>(DiskModelType::SystemDisk) == m_current_left_index) {
        m_right_model[static_cast<int>(DiskModelType::DataDisk)]->disableIndex(DiskInstallationTypes::ItemIndexs{ index });
   }

   Device::Ptr device = m_right_model[m_current_left_index]->virtualDevices().at(index);
   emit currentDeviceChanged(m_current_left_index, device);
}

bool MultipleDiskInstallationWidget::validate() const
{
    return m_right_model[static_cast<int>(DiskModelType::SystemDisk)]->selectedIndex() != -1;
}

void MultipleDiskInstallationWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        // just keep left and right selection unchanged
        int index = m_left_view->currentIndex().row();
        m_left_model->setStringList(getDiskTypes());
        m_left_view->setCurrentIndex(m_left_model->index(index, 0));
    }
    else {
        QWidget::changeEvent(event);
    }
}

const QStringList MultipleDiskInstallationWidget::getDiskTypes()
{
    return  QStringList { tr("System Disk"), tr("Data Disk") };
}

}
