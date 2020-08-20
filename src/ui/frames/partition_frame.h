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

#ifndef INSTALLER_UI_FRAMES_PARTITION_FRAME_H
#define INSTALLER_UI_FRAMES_PARTITION_FRAME_H

#include "ui/interfaces/frameinterface.h"

#include "partman/operation.h"
#include "partman/partition.h"

#include <QFrame>
#include <QPushButton>
#include <QScopedPointer>

class QStackedLayout;

namespace installer {

class AdvancedPartitionDelegate;
class AdvancedPartitionFrame;
class CommentLabel;
class EditPartitionFrame;
class FullDiskDelegate;
class FullDiskFrame;
class NewPartitionFrame;
class NewTableLoadingFrame;
class NewTableWarningFrame;
class PartitionLoadingFrame;
class PartitionModel;
class PartitionNumberLimitationFrame;
class PartitionTableWarningFrame;
class PointerButton;
class PrepareInstallFrame;
class SelectBootloaderFrame;
class SimplePartitionDelegate;
class SimplePartitionFrame;
class Full_Disk_Encrypt_frame;
class DynamicDiskWarningFrame;
class TitleLabel;
class PartitionFramePrivate;

// Handles partition operations.
class PartitionFrame : public FrameInterface {
  Q_OBJECT

    friend PartitionFramePrivate;
 public:
  explicit PartitionFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
  ~PartitionFrame() override;
  void init() override;
  void finished() override;
  bool shouldDisplay() const override;
  QString returnFrameName() const override;

  void autoPart();

 signals:
  // Emitted when partition edition is done.
//  void finished();

  // Emitted when requesting reboot system.
  void reboot();

  // Emitted when partition job is done.
  void autoPartDone(bool ok);
  void manualPartDone(bool ok);

  void coverMainWindowFrameLabelsView(bool cover);

 public slots:
  // Notify delegate to scan devices.
  void scanDevices() const;
  void onAutoInstallPrepareFinished(bool finished);

 protected:
  void changeEvent(QEvent* event) override;
  void showEvent(QShowEvent *event) override;

  QScopedPointer<PartitionFramePrivate> m_private;
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_PARTITION_FRAME_H
