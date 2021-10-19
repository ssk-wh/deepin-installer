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
#include "ui/utils/mouse_shape.h"

#include <QFrame>
#include <DMainWindow>
#include <DIconButton>
#include <DListView>

DWIDGET_USE_NAMESPACE

class QLabel;
class QResizeEvent;
class QStackedLayout;
class QThread;
class QVBoxLayout;

class GlobalShortcut;
class QShortcut;

namespace installer {

enum class FrameLabelState{
    Initial,
    Show,
    FinishedConfig,
    Previous
};

class FirstBootLoadingFrame;
class FirstBootHookWorker;
class SystemInfoFrame;
class TimezoneFrame;
class LanguageFrame;
class NetworkFrame;
class ControlPlatformFrame;
class SystemInfoKeyboardFrame;
class ControlPanelFrame;
class ShadowWidget;
class WarnningFrame;

// Main window of deepin_installer_first_boot.
class FirstBootSetupWindow : public DMainWindow, public FrameProxyInterface {
  Q_OBJECT

 public:
  explicit FirstBootSetupWindow(QWidget* parent = nullptr);
  ~FirstBootSetupWindow() override;

  // Show fullscreen.
  void fullscreen();

  void setScreen(QScreen *screen);
  void updateGeometry();

  void nextFrame() override;
  void exitInstall(bool reboot = false) override;
  void showChildFrame(BaseFrameInterface* childFrameInterface) override;
  void hideChildFrame() const override;

  // Set the icon in the upper left corner of the window. The size of the icon is recommended: 32px.
  void setWindowIcon(const QString &path);

protected:
  // Show ConfirmQuitFrame when top right corner is clicked.
  void onCloseEvent();
  void changeEvent(QEvent *event) override;
  void resizeEvent(QResizeEvent* event) override;
  bool eventFilter(QObject *target, QEvent *event) override;
 private:
  void initConnections();
  void initUI();
  void initPages();
  void constructLabelView();
  void showExtFrameFullscreen(BaseFrameInterface* childFrameInterface);

  void registerShortcut();

  void showFailedLog();

  DIconButton*           back_button_         = nullptr;
  DIconButton*           close_button_        = nullptr;
  QLabel*                background_label_    = nullptr;
  SystemInfoFrame*       system_info_frame_   = nullptr;
  TimezoneFrame*         timezone_frame_      = nullptr;
  FirstBootLoadingFrame* loading_frame_       = nullptr;
  LanguageFrame*         language_frame_      = nullptr;
  ControlPlatformFrame*  control_platform_frame_ = nullptr;
  QStackedLayout*        stacked_layout_      = nullptr;
  SystemInfoKeyboardFrame* m_keyboardFrame    = nullptr;
  ShadowWidget*          shadow_widget = nullptr;
  WarnningFrame*        optimize_failed_frame_  = nullptr;
  WarnningFrame*        confirm_quit_frame_  = nullptr;

  QThread*             hook_worker_thread_ = nullptr;
  FirstBootHookWorker* hook_worker_        = nullptr;

  // Shortcut used to switch mirror modes.
  GlobalShortcut*   monitor_mode_shortcut_ = nullptr;

  QVBoxLayout* m_frameSelectedLayout = nullptr;

  QList<FrameInterface*> m_originalFrames;
  QList<FrameInterface*> m_frames;
  QList<FrameInterface*> m_hasShowFrames;
  bool m_showPastFrame = false;
  bool m_currentAllowPreviousState = true;

  QList<QString> m_frameTitles;
  DListView* m_frameLabelsView = nullptr;
  QWidget* m_frameSelectedListWidget = nullptr;
  QWidget* m_frameLabelsViewCoverWidget = nullptr;
  QStandardItemModel* m_frameLabelsModel = nullptr;
  QMap<FrameInterface*, DStandardItem*> m_frameModelItemMap;

  // Shortcut used to toggle visibility of log-viewer.
  QShortcut* control_panel_shortcut_ = nullptr;

  ControlPanelFrame* control_panel_frame_ = nullptr;

  MouseShape m_mouseShape;

  QScreen* m_screen = nullptr;

  static const int FramePointerRole = Dtk::UserRole + 1;

private slots:
  // Handles result of hook worker.
  void onHookFinished(bool ok);

  FrameInterface *getFrameInterface(QStandardItem *item) const;

  void onLanguageSelected();
  void onSystemInfoFinished();

  void onNetworkFinished();

  void onStartRunHooks();

  // Run "first_boot_setup.sh" after system_info_frame_ is finished.
  void onTimezoneFinished();

  void backPage();
  void updateBackButtonVisible(QWidget* page);

  bool changeToTTY(int ttyNum) const;

  // When user mouse press left page tables.
  void previousFrameSelected(FrameInterface* frame);
  void onFrameLabelsViewClicked(const QModelIndex& index);

  void updateFrameLabelState(FrameInterface *frame, FrameLabelState state);
  void updateFrameLabelPreviousState(bool allow);

  void shutdownSystem();

  void setCloseButtonVisible(bool visible);
};

}  // namespace installer

#endif  // INSTALLER_UI_SETUP_WINDOW_H
