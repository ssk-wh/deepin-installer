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

#ifndef INSTALLER_UI_SETUP_WINDOW_H
#define INSTALLER_UI_SETUP_WINDOW_H

#include "ui/interfaces/frameproxyinterface.h"

#include <QFrame>
#include <DMainWindow>
#include <DImageButton>
#include <DListView>

DWIDGET_USE_NAMESPACE

class QLabel;
class QResizeEvent;
class QStackedLayout;
class QThread;
class QVBoxLayout;

class GlobalShortcut;

namespace installer {

enum class FrameLabelState{
    Initial,
    Show,
    FinishedConfig
};

class FirstBootLoadingFrame;
class FirstBootHookWorker;
class MultiHeadManager;
class SystemInfoFrame;
class TimezoneFrame;
class LanguageFrame;
class NetworkFrame;
class ControlPlatformFrame;

// Main window of deepin_installer_first_boot.
class FirstBootSetupWindow : public DMainWindow, public FrameProxyInterface {
  Q_OBJECT

 public:
  explicit FirstBootSetupWindow(QWidget* parent = nullptr);
  ~FirstBootSetupWindow();

  // Show fullscreen.
  void fullscreen();

  void nextFrame() override;
  void showChildFrame(FrameInterface* frame) override;
  void exitInstall(bool reboot = false) override;
  void hideChildFrame() const override;

 private:
  void initConnections();
  void initUI();

  void registerShortcut();

  DImageButton*          back_button_         = nullptr;
  QLabel*                background_label_    = nullptr;
  SystemInfoFrame*       system_info_frame_   = nullptr;
  TimezoneFrame*         timezone_frame_      = nullptr;
  FirstBootLoadingFrame* loading_frame_       = nullptr;
  LanguageFrame*         language_frame_      = nullptr;
  NetworkFrame*          network_frame_       = nullptr;
  ControlPlatformFrame*  control_platform_frame_ = nullptr;
  QStackedLayout*        stacked_layout_      = nullptr;

  QThread*             hook_worker_thread_ = nullptr;
  FirstBootHookWorker* hook_worker_        = nullptr;

  // Shortcut used to switch mirror modes.
  GlobalShortcut*   monitor_mode_shortcut_ = nullptr;
  MultiHeadManager* multi_head_manager_    = nullptr;

  QVBoxLayout* m_frameSelectedLayout = nullptr;

  QList<FrameInterface*> m_originalFrames;
  QList<FrameInterface*> m_frames;
  bool m_showPastFrame = false;

  QList<QString> m_frameTitles;
  DListView* m_frameLabelsView = nullptr;
  QStandardItemModel* m_frameLabelsModel = nullptr;
  QMap<FrameInterface*, DStandardItem*> m_frameModelItemMap;

  static const int FramePointerRole = Dtk::UserRole + 1;

  private slots:
  // Handles result of hook worker.
  void onHookFinished(bool ok);

  void onPrimaryScreenChanged(const QRect& geometry);

  void onLanguageSelected();
  void onSystemInfoFinished();

  void onNetworkFinished();

  void onControlPlatformFinished();

  // Run "first_boot_setup.sh" after system_info_frame_ is finished.
  void onTimezoneFinished();

  void backPage();
  void updateBackButtonVisible(QWidget* page);

  bool changeToTTY(int ttyNum) const;

  // When user mouse press left page tables.
  void previousFrameSelected(FrameInterface* frame);

  void updateFrameLabelState(FrameInterface *frame, FrameLabelState state);
};

}  // namespace installer

#endif  // INSTALLER_UI_SETUP_WINDOW_H
