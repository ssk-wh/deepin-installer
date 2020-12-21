/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#ifndef INSTALLER_UI_FRAMES_INNER_SELECT_BOOTLOADER_FRAME_H
#define INSTALLER_UI_FRAMES_INNER_SELECT_BOOTLOADER_FRAME_H

#include "partman/device.h"
#include "ui/interfaces/frameinterface.h"

#include <QFrame>
#include <QPushButton>
#include <DListView>

DWIDGET_USE_NAMESPACE

namespace installer {

class CommentLabel;
class BootloaderListModel;
class TitleLabel;

// Displays a window to select available boot path.
// This frame read recommended bootloader path, and notifies any changes to
// PartitionDelegate and AdvancedPartitionFrame.
// It refreshes available partition list for bootloader when mount-point list
// is updated in PartitionDelegate.
class SelectBootloaderFrame : public ChildFrameInterface {
  Q_OBJECT

 public:
  explicit SelectBootloaderFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);

    bool focusSwitch() override;
    bool doSpace() override;
    bool doSelect() override;
    bool directionKey(int keyvalue) override;

 signals:
  // Emitted when currently used bootloader path is updated.
  void bootloaderUpdated(const QString& bootloader_path, bool simple_mode);

  void deviceRefreshed(const DeviceList& devices);

  // Emitted when back-button is clicked.
  void finished();

 protected:
  void changeEvent(QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

 private:
  void initConnections();
  void initUI();

  TitleLabel* title_label_ = nullptr;
  CommentLabel* comment_label_ = nullptr;
  DListView* list_view_ = nullptr;
  BootloaderListModel* list_model_ = nullptr;
  QPushButton* back_button_ = nullptr;

 private slots:
  // Update partition list
  void onModelChanged();

  // Emit signals when new bootloader path is selected in list-view.
  void onPartitionListViewSelected(const QModelIndex& current,
                                   const QModelIndex& previous);
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_INNER_SELECT_BOOTLOADER_FRAME_H
