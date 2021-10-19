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

#ifndef INSTALLER_UI_MAIN_WINDOW_H
#define INSTALLER_UI_MAIN_WINDOW_H

#include "ui/interfaces/frameproxyinterface.h"
#include "ui/utils/mouse_shape.h"

#include <QWidget>
#include <QHash>
#include <QMap>
#include <DIconButton>
#include <DListView>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class QLabel;
class QPushButton;
class QResizeEvent;
class QShortcut;
class QStackedLayout;
class QVBoxLayout;

class GlobalShortcut;

namespace installer {

enum class FrameLabelState{
    Initial,
    Show,
    FinishedConfig,
    Previous
};

class FrameInterface;
class ControlPanelFrame;
class DiskSpaceInsufficientFrame;
class InstallProgressFrame;
class PageIndicator;
class PartitionFrame;
class PartitionTableWarningFrame;
class PrivilegeErrorFrame;
class LanguageFrame;
class SystemInfoFrame;
class TimezoneFrame;
class VirtualMachineFrame;
class SelectInstallComponentFrame;
class InstallResultsFrame;
class ShadowWidget;
class RepairSystemFrame;
class SystemInfoKeyboardFrame;
class NetworkFrame;
class WarnningFrame;
// MainWindow is a fullscreen window of deepin-installer.
// All of ui frames are placed in MainWindow.
// It does following jobs:
//   * draw window background with a blur image;
//   * handles keyboard shortcut like Ctrl+P and Ctrl+L;
//   * handles window navigation (by providing a content area);
//   * quit / abort installation process.
class MainWindow : public DMainWindow, public FrameProxyInterface {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);

  // 设置主程序的翻译和无人值守分区
  void setup();

  void setScreen(QScreen *screen);
  void updateGeometry();

  // Notify background thread to scan disk devices if needed.
  // And read current timezone.
  void scanDevicesAndTimezone();

  // Enable auto-install mode.
  //void setEnableAutoInstall(bool auto_install);

  // Set filepath to which log file will be backup.
  void setLogFile(const QString& log_file);

  // Set the icon in the upper left corner of the window. The size of the icon is recommended: 32px.
  void setWindowIcon(const QString &path);

  void setCloseButtonVisible(bool visible);

  void nextFrame() override;
  void exitInstall(bool reboot = false) override;
  void showChildFrame(BaseFrameInterface* childFrameInterface) override;
  void hideChildFrame() const override;
  void showExtFrameFullscreen(BaseFrameInterface* childFrameInterface);
  void coverFrameLabelsView(bool cover) const;

 protected:
  // Show ConfirmQuitFrame when top right corner is clicked.
  void onCloseEvent();
  void changeEvent(QEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  bool eventFilter(QObject *target, QEvent *event) override;
private:
  enum PageId {
      NullId,  // page not set.

      ConfirmQuitId,
      DiskSpaceInsufficientId,
      InstallProgressId,
      PartitionId,
      PrivilegeErrorId,
      SelectLanguageId,
      SystemInfoId,
      TimezoneId,
      InstallResultsId,
      VirtualMachineId,
      UserAgreementId,
      SelectComponentId,
      RepairSystemId
  };

  void initConnections();
  void initPages();
  void initUI();
  void constructLabelView();
  void registerShortcut();

  // Copy log file if needed.
  void saveLogFile();

  // Switch frame page based on name.
  void setCurrentPage(PageId page_id);

  void updateWidgetVisible();

  void backPage();

  static bool checkBackButtonAvailable(PageId id);
  void updateFrameLabelState(FrameInterface *frame, FrameLabelState state);
  void updateFrameLabelPreviousState(bool allow);

  FrameInterface* getFrameInterface(QStandardItem *item) const;

  QLabel* background_label_ = nullptr;
  DIconButton* back_button_ = nullptr;
  DIconButton* close_button_ = nullptr;
  PageIndicator* page_indicator_ = nullptr;
  // All of frame pages are stored in this layout.
  // And they are referenced by id in |pages_|.
  QStackedLayout* stacked_layout_ = nullptr;
  QVBoxLayout* m_frameSelectedLayout = nullptr;

  WarnningFrame* confirm_quit_frame_ = nullptr;
  ControlPanelFrame* control_panel_frame_ = nullptr;
  RepairSystemFrame* m_repairSystemFrame = nullptr;
  DiskSpaceInsufficientFrame* disk_space_insufficient_frame_ = nullptr;
  InstallProgressFrame* install_progress_frame_ = nullptr;
  PartitionFrame* partition_frame_ = nullptr;
  PrivilegeErrorFrame* privilege_error_frame_ = nullptr;
  LanguageFrame* select_language_frame_ = nullptr;
  SystemInfoFrame* system_info_frame_ = nullptr;
  TimezoneFrame* timezone_frame_ = nullptr;
  VirtualMachineFrame* virtual_machine_frame_ = nullptr;
  SelectInstallComponentFrame* m_selectComponentFrame = nullptr;
  InstallResultsFrame* m_installResultsFrame = nullptr;
  ShadowWidget* shadow_widget = nullptr;
  SystemInfoKeyboardFrame *m_keyboarFrame = nullptr;
  NetworkFrame*          network_frame_       = nullptr;

  // To store frame pages, page_name => page_id.
  QHash<PageId, int> pages_;
  QList<QWidget*> m_old_frames;

  // Keep previous page id. It is used by ConfirmQuitPage.
  PageId prev_page_;
  PageId current_page_;

  // Shortcut used to toggle visibility of log-viewer.
  QShortcut* control_panel_shortcut_ = nullptr;

  // Shortcut used to switch mirror modes.
  GlobalShortcut* monitor_mode_shortcut_ = nullptr;

  // Shortcut for screen brightness.
  QShortcut* brightness_increase_shortcut_ = nullptr;
  QShortcut* brithtness_decrease_shortcut_ = nullptr;

  QString log_file_;
  //bool auto_install_;

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

    MouseShape m_mouseShape;

    QScreen* m_screen = nullptr;

    static const int FramePointerRole = Dtk::UserRole + 1;

 private slots:
  // Go next page when current page index is changed in ControlPanelFrame.
  void onCurrentPageChanged(int index);

  void goNextPage();
  void rebootSystem();
  void shutdownSystem();

  // When user mouse press left page tables.
  void previousFrameSelected(FrameInterface* frame);

  void onFrameLabelsViewClicked(const QModelIndex& index);
};

}  // namespace installer

#endif  // INSTALLER_UI_MAIN_WINDOW_H
