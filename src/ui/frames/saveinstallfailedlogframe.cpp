/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     justforlxz <justforlxz@outlook.com>
 *
 * Maintainer: justforlxz <justforlxz@outlook.com>
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

#include "saveinstallfailedlogframe.h"

#include "ui/widgets/comment_label.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/device_model_label.h"
#include "ui/widgets/simple_partition_button.h"
#include "ui/widgets/nav_button.h"
#include "ui/delegates/main_window_util.h"
#include "base/file_util.h"
#include "service/log_manager.h"
#include "base/command.h"

#include <QSaveFile>
#include <QEvent>
#include <QLabel>
#include <QGridLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QFileInfo>
#include <QDateTime>
#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>

using namespace installer;

// 4 partitions are displays at each row.
const int kDiskColumns = 4;
const int kWindowWidth = 960;

SaveInstallFailedLogFrame::SaveInstallFailedLogFrame(QWidget *parent) : QWidget(parent)
{
    m_title = new QLabel;
    m_subTitle = new CommentLabel;
    m_partitionGridLayout = new QGridLayout;
    m_button_group = new QButtonGroup;
    QScrollArea* area = new  QScrollArea;
    QWidget* widget = new QWidget;
    widget->setObjectName("grid_wrapper");
    widget->setLayout(m_partitionGridLayout);

    m_title->setStyleSheet("color: white;"
                           "font-size: 32px;");

    m_partitionGridLayout->setSpacing(0);
    m_partitionGridLayout->setContentsMargins(0, 0, 0, 0);
    m_partitionGridLayout->setHorizontalSpacing(20);
    m_partitionGridLayout->setVerticalSpacing(20);
    m_partitionGridLayout->setColumnStretch(kDiskColumns, 1);

    area->setWidgetResizable(true);
    area->setObjectName("scroll_area");
    area->setFrameStyle(QFrame::NoFrame);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setContentsMargins(0, 0, 0, 0);
    area->setWidget(widget);
    area->setStyleSheet("background: transparent;");
    widget->setFixedWidth(kWindowWidth);
    area->setFixedWidth(kWindowWidth);

    m_saveBtn = new NavButton;
    m_backBtn = new NavButton;

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addSpacing(50);
    mainLayout->addWidget(m_title, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_subTitle, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(area, 0, Qt::AlignHCenter);
    mainLayout->addStretch();
    mainLayout->addWidget(m_saveBtn, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_backBtn, 0, Qt::AlignHCenter);

    m_saveBtn->setDisabled(true);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addLayout(mainLayout);
    layout->addStretch();

    setLayout(layout);

    setStyleSheet(ReadFile(":/styles/simple_partition_frame.css"));

    updateTs();

    connect(m_saveBtn, &NavButton::clicked, this, &SaveInstallFailedLogFrame::saveLog);
    connect(m_backBtn, &NavButton::clicked, this, &SaveInstallFailedLogFrame::requestBack);

    m_diskManager = new DDiskManager;
    m_diskManager->setWatchChanges(false);
    connect(m_diskManager, &DDiskManager::blockDeviceAdded, this, &SaveInstallFailedLogFrame::onBlockDeviceAdded, Qt::QueuedConnection);
    connect(m_diskManager, &DDiskManager::blockDeviceRemoved, this, &SaveInstallFailedLogFrame::onBlockDeviceRemoved, Qt::QueuedConnection);
    connect(m_diskManager, &DDiskManager::diskDeviceRemoved, this, &SaveInstallFailedLogFrame::onDeviceRemoved, Qt::QueuedConnection);
}

void SaveInstallFailedLogFrame::startDeviceWatch(bool enable) {
    m_saveBtn->setEnabled(false);

    if (m_diskManager->watchChanges() == enable) {
        return;
    }

    m_diskManager->setWatchChanges(enable);
    m_deviceMap.clear();
    for (const QString& path : m_diskManager->blockDevices()) {
        onBlockDeviceAdded(path);
    }
}

bool SaveInstallFailedLogFrame::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        updateTs();
    }

    return QWidget::event(event);
}

void SaveInstallFailedLogFrame::updateTs()
{
    m_title->setText(tr("Save Log"));
    m_subTitle->setText(tr("Save the error log to a removable device"));
    m_saveBtn->setText(tr("Save as"));
    m_backBtn->setText(tr("Back"));
}

void SaveInstallFailedLogFrame::refreshDevices()
{
    ClearLayout(m_partitionGridLayout);

    for (QAbstractButton* button : m_button_group->buttons()) {
        m_button_group->removeButton(button);
        button->deleteLater();
    }

    m_deviceButtonMap.clear();

    int row = 0, column = 0;
    for (auto it = m_deviceMap.constBegin(); it != m_deviceMap.constEnd(); ++it) {
        DeviceModelLabel* device_model_label = new DeviceModelLabel();
        device_model_label->setText(it->first()->id());
        device_model_label->setFixedSize(kWindowWidth, 20);

        // Make sure that widgets in grid are left-aligned.
        m_partitionGridLayout->addWidget(device_model_label,
                                         row,
                                         0,
                                         1,
                                         kDiskColumns,
                                         Qt::AlignLeft);
        row += 1;

        auto list = it.value();
        for (auto part = list.constBegin(); part != list.constEnd(); ++part) {
            Partition::Ptr partition(new Partition);
            partition->type = PartitionType::Normal;
            partition->path = part->get()->device();
            partition->start_sector = 0;
            partition->end_sector = static_cast<qint64>(part->get()->size());
            partition->sector_size = 1;
            partition->fs = GetFsTypeByName(part->get()->idVersion());
            partition->label = part->get()->idLabel();
            SimplePartitionButton* button = new SimplePartitionButton(partition);
            m_button_group->addButton(button);
            m_partitionGridLayout->addWidget(button, row, column, Qt::AlignHCenter);
            connect(button, &QPushButton::clicked,
                    this, &SaveInstallFailedLogFrame::onPartitionButtonClicked);

            m_deviceButtonMap[partition] = *part;

            column += 1;
            // Add rows.
            if (column >= kDiskColumns) {
                column = 0;
                row += 1 ;
            }
        }

        // Go to next row.
        column = 0;
        row += 1;
    }

    // Add place holder. It is used for install_tip
    row += 1;
    QLabel* place_holder_label = new QLabel(this);
    place_holder_label->setObjectName("place_holder_label");
    place_holder_label->setFixedSize(kWindowWidth, 30);
    m_partitionGridLayout->addWidget(place_holder_label, row, 0,
                            1, kDiskColumns, Qt::AlignHCenter);

    m_saveBtn->setEnabled(false);
}

void SaveInstallFailedLogFrame::onBlockDeviceAdded(const QString &path)
{
    QSharedPointer<DBlockDevice> device(DDiskManager::createBlockDevice(path));

    if (device->readOnly() || !device->hasPartition()) {
        return;
    }

    //try mount to check writable
    auto checkWritable = [=](const QString& mount) -> bool {
        return QFileInfo(mount).isWritable();
    };

    const QByteArrayList& mountPoints = device->mountPoints();
    if (!mountPoints.isEmpty()) {
        for (const QByteArray& array : mountPoints) {
            if (!checkWritable(array)) {
                return;
            }
        }
    }

    QSharedPointer<DDiskDevice> disk(DDiskManager::createDiskDevice(device->drive()));

    if (!disk->removable()) {
        return;
    }

    for (auto it = m_deviceMap.begin(); it != m_deviceMap.end(); ++it) {
        if (it.key()->path() == disk->path()) {
            QList<QSharedPointer<DBlockDevice>> &list = it.value();
            for (QSharedPointer<DBlockDevice> block : list) {
                if (block->path() == device->path()) {
                    return;
                }
            }
            list << device;
            return refreshDevices();
        }
    }

    m_deviceMap[disk] << device;
    return refreshDevices();
}

void SaveInstallFailedLogFrame::onBlockDeviceRemoved(const QString &path)
{
    for (auto it = m_deviceMap.begin(); it != m_deviceMap.end(); ++it) {
        QList<QSharedPointer<DBlockDevice>> &list = it.value();
        for (auto part = list.begin(); part != list.end(); ++part) {
            if (part->data()->path() == path) {
                part = list.erase(part);
                if (list.isEmpty()) {
                    it = m_deviceMap.erase(it);
                }
                return refreshDevices();
            }
        }
    }
}

void SaveInstallFailedLogFrame::onDeviceRemoved(const QString &path)
{
    for (auto it = m_deviceMap.begin(); it != m_deviceMap.end(); ++it) {
        if (it.key()->path() == path) {
            it.value().clear();
            m_deviceMap.erase(it);
            return refreshDevices();
        }
    }
}

void SaveInstallFailedLogFrame::onPartitionButtonClicked()
{
    m_saveBtn->setEnabled(true);
    m_selectPartition = static_cast<SimplePartitionButton*>(sender())->partition();
}

void SaveInstallFailedLogFrame::saveLog()
{
    const QString& logPath {
        QString("%1/deepin-installer.%2.log")
                .arg(m_deviceButtonMap[m_selectPartition]->mount({}))
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss"))
    };

    qDebug() << "Log file: " << logPath;

    CopyLogFile(logPath);

    m_deviceButtonMap[m_selectPartition]->unmount({});

    emit requestBack();
}
