/*
 * Copyright (C) 2018 Deepin Technology Co., Ltd.
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

#ifndef DEEPIN_INSTALLER_UI_FRAMES_INNER_FULL_DISK_FRAME_H
#define DEEPIN_INSTALLER_UI_FRAMES_INNER_FULL_DISK_FRAME_H

#include "partman/device.h"

#include <QFrame>
#include <functional>

class QAbstractButton;
class QButtonGroup;
class QGridLayout;
class QLabel;
class QShowEvent;
class QCheckBox;
class QStackedLayout;

namespace installer {

class FullDiskDelegate;
class FullDiskPartitionWidget;
class MultipleDiskInstallationWidget;

class FullDiskFrame : public QFrame {
    Q_OBJECT
public:
    explicit FullDiskFrame(FullDiskDelegate* delegate,
                           QWidget*          parent = nullptr);
    ~FullDiskFrame();

    bool validate() const;
    bool isEncrypt() const;
    bool isEnSaveData() const;

    void setSaveDataCheckEnabel(bool isenabel);

    bool focusSwitch();
    bool doSpace();
    bool doSelect();
    bool directionKey(int keyvalue);
    bool isSaveData();

signals:
    void currentDeviceChanged(const Device::Ptr device) const;
    void showDeviceInfomation();
    void enableNextButton(const bool enable) const;
    void showSaveDataPopWidget();

protected:
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void initConnections();
    void initUI();

    // Repaint partition list.
    void repaintDevices();

    // Show install_tip at bottom of |button|.
    void showInstallTip(bool isshow);

    // 判断是磁盘中否存在/data分区
    bool isExistDataPart(const QString &devicepath);

    // 判断是否是一个系统盘
    bool isSystemDisk(const QString &devicepath);

    // 判断否是全盘加密
    bool isFullDiskEncrypt(const QString &devicepath);

    // 设置保留用户数据勾选控件
    void setSaveDataCheckboxStat(const Device::Ptr device, const int type = 0);

    FullDiskDelegate* m_delegate     = nullptr;
    QButtonGroup*     m_button_group = nullptr;
    QFrame*           m_install_tip  = nullptr;
    QLabel*           m_tip_icon     = nullptr;
    QLabel*           m_tip_label    = nullptr;
    QGridLayout*      m_grid_layout  = nullptr;
    QFrame*           m_grid_wrapper = nullptr;
    QCheckBox*        m_encryptCheck = nullptr;
    QLabel*           m_errorTip     = nullptr;
    QLabel*           m_diskTooSmallTip     = nullptr;
    FullDiskPartitionWidget* m_diskPartitionWidget = nullptr;
    MultipleDiskInstallationWidget* m_diskInstallationWidget = nullptr;
    QStackedLayout*      m_disk_layout  = nullptr;
    std::list<std::pair<std::function<void (QString)>, QString>> m_trList;
    QCheckBox*        m_saveDataCheck = nullptr;

public slots:
    void onDeviceRefreshed();
    void onPartitionButtonToggled(QAbstractButton* button, bool checked);
    void onCurrentDeviceChanged(int type, const Device::Ptr device);
    void saveDataStateChanged(bool savedata);
    void cryptoStateChanged(bool crypto);
};

}  // namespace installer

#endif  // DEEPIN_INSTALLER_UI_FRAMES_INNER_FULL_DISK_FRAME_H
