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
#include <QEvent>
#include <DFrame>
#include <DVerticalLine>
#include <QListView>

#include "ui/widgets/multiple_disk_installation_widget.h"
#include "ui/views/disk_installation_detail_view.h"
#include "ui/models/disk_installation_detail_model.h"
#include "ui/delegates/disk_installation_detail_delegate.h"

DWIDGET_USE_NAMESPACE

namespace {
    const int kLeftListViewWidth = 130;
    const int kRightListViewWidth = 390;
}

namespace installer {

MultipleDiskInstallationWidget::MultipleDiskInstallationWidget(QWidget *parent)
    : DFrame(parent)
    , m_current_left_index(0)
{
    initUI();
    initConnections();
}

bool MultipleDiskInstallationWidget::focusSwitch()
{
    QWidget *leftWrapWidget = this->findChild<QWidget *>("leftWrapWidget");
    if (leftWrapWidget->hasFocus()) {
        QModelIndex testindex = m_left_view->currentIndex();
        if ((testindex.row() + 1) < m_left_view->count()) {
            m_left_view->setCurrentIndex(testindex.siblingAtRow(testindex.row() + 1));
            return false;
        }
    }
    return true;
}

bool MultipleDiskInstallationWidget::doSpace()
{
    return true;
}

bool MultipleDiskInstallationWidget::doSelect()
{
    return true;
}

bool MultipleDiskInstallationWidget::directionKey(int keyvalue)
{
    QWidget *leftWrapWidget = this->findChild<QWidget *>("leftWrapWidget");
    QWidget *rightWrapWidget = this->findChild<QWidget *>("rightWrapWidget");

    switch (keyvalue) {
    case Qt::Key_Up: {
            if (leftWrapWidget->hasFocus()) {
                QModelIndex testindex = m_left_view->currentIndex();
                if ((testindex.row() - 1) >= 0) {
                    m_left_view->setCurrentIndex(testindex.siblingAtRow(testindex.row() - 1));
                }
            } else {
                int testindex = m_right_model[m_current_left_index]->selectedIndex();
                if (testindex == -1) {
                    m_right_view->setCurrentIndex(m_right_model[m_current_left_index]->index(0, 0));
                    emit m_right_view->currentSelectedChange(0);
                } else if((testindex - 1) >= 0) {
                    m_right_view->setCurrentIndex(m_right_model[m_current_left_index]->index(testindex - 1, 0));
                    emit m_right_view->currentSelectedChange(testindex - 1);
                }
            }
        }
        break;
    case Qt::Key_Down: {
            if (leftWrapWidget->hasFocus()) {
                QModelIndex testindex = m_left_view->currentIndex();
                if ((testindex.row() + 1) < m_left_view->count()) {
                    m_left_view->setCurrentIndex(testindex.siblingAtRow(testindex.row() + 1));
                }
            } else {
                int testindex = m_right_model[m_current_left_index]->selectedIndex();
                if (testindex == -1) {
                    m_right_view->setCurrentIndex(m_right_model[m_current_left_index]->index(0, 0));
                    emit m_right_view->currentSelectedChange(0);
                } else if((testindex + 1) < m_right_view->count()) {
                    m_right_view->setCurrentIndex(m_right_model[m_current_left_index]->index(testindex + 1, 0));
                    emit m_right_view->currentSelectedChange(testindex + 1);
                }
            }
        }
        break;
    case Qt::Key_Left: {
            leftWrapWidget->setFocus();
        }
        break;
    case Qt::Key_Right: {
            rightWrapWidget->setFocus();
        }
        break;
    }

    return true;
}

bool MultipleDiskInstallationWidget::isLeftRightWidgetHasFocus()
{
    QWidget *leftWrapWidget = this->findChild<QWidget *>("leftWrapWidget");
    QWidget *rightWrapWidget = this->findChild<QWidget *>("rightWrapWidget");
    if(leftWrapWidget->hasFocus() || rightWrapWidget->hasFocus()) {
        return true;
    } else {
        return false;
    }
}

void MultipleDiskInstallationWidget::initConnections()
{
    connect(m_left_view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [&] (const QModelIndex &current, const QModelIndex &previous) {
        Q_UNUSED(previous);
        onInstallationSelectedChanged(current);
    });
    connect(m_right_view, &DiskInstallationDetailView::currentSelectedChange,
            this, &MultipleDiskInstallationWidget::onInstallationDetailSelectedChanged);
}

void MultipleDiskInstallationWidget::initUI()
{
    m_left_model = new QStringListModel(getDiskTypes());
    m_left_view = new DListView();
    m_left_view->setObjectName("LeftListView");
    m_left_view->setFixedWidth(kLeftListViewWidth - 20);
    m_left_view->setItemSize(QSize(kLeftListViewWidth - 20, 46));
    m_left_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_left_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_left_view->setModel(m_left_model);

    m_right_view = new DiskInstallationDetailView();
    m_right_view->setObjectName("RightListView");
    m_right_view->setFixedWidth(kRightListViewWidth);
    m_right_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_right_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    DiskInstallationDetailDelegate* delegate = new DiskInstallationDetailDelegate(m_right_view);
    delegate->setItemSize(QSize(kRightListViewWidth - 20, 46));
    m_right_view->setItemDelegate(delegate);

    for (int i = 0; i < kDiskModelMaxCount; i++) {
        m_right_model[i] = new DiskInstallationDetailModel();
    }

    QHBoxLayout * leftlayout = new QHBoxLayout();
    leftlayout->setContentsMargins(10, 10, 10, 10);
    leftlayout->setSpacing(0);
    leftlayout->addWidget(m_left_view);
    QWidget *leftWrapWidget = new QWidget;
    leftWrapWidget->setObjectName("leftWrapWidget");
    leftWrapWidget->setContentsMargins(0, 0, 0, 0);
    leftWrapWidget->setLayout(leftlayout);
    leftWrapWidget->setStyleSheet("QWidget#leftWrapWidget::focus{border:1px solid; border-color:rgb(1, 128, 255); border-radius:5px; padding:2px 4px;}");

    QHBoxLayout * rightlayout = new QHBoxLayout();
    rightlayout->setContentsMargins(0, 0, 0, 0);
    rightlayout->setSpacing(0);
    rightlayout->addWidget(m_right_view);
    QWidget *rightWrapWidget = new QWidget;
    rightWrapWidget->setObjectName("rightWrapWidget");
    rightWrapWidget->setContentsMargins(0, 0, 0, 0);
    rightWrapWidget->setLayout(rightlayout);
    rightWrapWidget->setStyleSheet("QWidget#rightWrapWidget::focus{border:1px solid; border-color:rgb(1, 128, 255); border-radius:5px; padding:2px 4px;}");

    DVerticalLine* verticalLine = new DVerticalLine;

    QHBoxLayout * hboxlayout = new QHBoxLayout();
    hboxlayout->setContentsMargins(0, 0, 0, 0);
    hboxlayout->setSpacing(0);

    hboxlayout->addStretch();
    hboxlayout->addWidget(leftWrapWidget, 0, Qt::AlignRight);
    hboxlayout->addWidget(verticalLine);
    hboxlayout->addWidget(rightWrapWidget, 0, Qt::AlignLeft);
    hboxlayout->addStretch();

    DFrame *frame = new DFrame;
    frame->setFrameRounded(true);
    frame->setContentsMargins(1, 1, 1, 1);
    frame->setLayout(hboxlayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addStretch();
    mainLayout->addWidget(frame);
    mainLayout->addStretch();

    setFrameRounded(false);
    setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
    setFocusProxy(leftWrapWidget);
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
        m_right_model[i]->setSelectedIndex(-1);
   }

   m_right_view->setCurrentIndex(QModelIndex());
   m_left_view->setCurrentIndex(m_left_model->index(0, 0));
}

void MultipleDiskInstallationWidget::onInstallationSelectedChanged(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    if (index.row() >= kDiskModelMaxCount) {
        return;
    }

    int current_detail_index;
    m_current_left_index = index.row();

    current_detail_index = m_right_model[m_current_left_index]->selectedIndex();

    // all disks are unavailable for data disk before system disk is selected
    // you can select any other disk for data disk, except the one which is selected as system disk
    if (static_cast<int>(DiskModelType::DataDisk) == m_current_left_index) {
        int sys_index = m_right_model[static_cast<int>(DiskModelType::SystemDisk)]->selectedIndex();
        if (-1 == sys_index) {
            m_right_model[m_current_left_index]->disableIndex(
                        DiskInstallationTypes::ItemIndexs(0, m_right_model[m_current_left_index]->devices().length()));
        }
        else {
            m_right_model[m_current_left_index]->disableIndex(DiskInstallationTypes::ItemIndexs { sys_index });
        }
    }

    m_right_view->blockSignals(true);
    m_right_view->setModel(m_right_model[m_current_left_index]);
    m_right_view->setCurrentIndex(m_right_model[m_current_left_index]->index(current_detail_index, 0));
    m_right_model[m_current_left_index]->setSelectedIndex(current_detail_index);
    m_right_view->blockSignals(false);
}

void MultipleDiskInstallationWidget::onInstallationDetailSelectedChanged(int index)
{
   m_right_model[m_current_left_index]->setSelectedIndex(index);
   if (-1 == index) {
       return;
   }

   Device::Ptr device = m_right_model[m_current_left_index]->getDevice(index);

   if (static_cast<int>(DiskModelType::SystemDisk) == m_current_left_index) {
       int id = m_right_model[static_cast<int>(DiskModelType::DataDisk)]->getIndex(device);
       if (id >= 0) {
           m_right_model[static_cast<int>(DiskModelType::DataDisk)]->disableIndex(DiskInstallationTypes::ItemIndexs{id});
       }
   }

   emit currentDeviceChanged(m_current_left_index, device);
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
    return  QStringList { ::QObject::tr("System Disk"), ::QObject::tr("Data Disk") };
}

}
