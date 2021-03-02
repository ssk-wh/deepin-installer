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

#ifndef SAVEINSTALLFAILEDLOGFRAME_H
#define SAVEINSTALLFAILEDLOGFRAME_H

#include "ui/interfaces/frameinterface.h"

#include <QWidget>
#include <QMap>
#include <QSharedPointer>

class QLabel;
class QPushButton;
class QGridLayout;
class QButtonGroup;
class DBlockDevice;
class DDiskDevice;
class DDiskManager;

namespace installer {
class CommentLabel;
class Partition;
class SaveInstallFailedLogFrame : public ChildFrameInterface
{
    Q_OBJECT
public:
    explicit SaveInstallFailedLogFrame(FrameProxyInterface* frameProxyInterface, QWidget *parent = nullptr);

    void startDeviceWatch(bool enable);

    bool doSelect() override;
signals:
    void requestBack() const;

protected:
    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void updateTs();
    void refreshDevices();
    void onBlockDeviceAdded(const QString& path);
    void onBlockDeviceRemoved(const QString& path);
    void onDeviceRemoved(const QString& path);
    void onPartitionButtonClicked();
    void saveLog();

private:
    QLabel* m_title;
    CommentLabel* m_subTitle;
    QGridLayout* m_partitionGridLayout;
    QButtonGroup* m_button_group;
    QPushButton* m_saveBtn;
    QPushButton* m_backBtn;
    QSharedPointer<Partition> m_selectPartition;
    QMap<QSharedPointer<Partition>, QSharedPointer<DBlockDevice>> m_deviceButtonMap;
    DDiskManager* m_diskManager;
    QMap<QSharedPointer<DDiskDevice>, QList<QSharedPointer<DBlockDevice>>> m_deviceMap;
};
}

#endif // SAVEINSTALLFAILEDLOGFRAME_H
