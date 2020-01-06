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

#include "ui/main_window.h"

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QShortcut>
#include <QStackedLayout>
#include <QTranslator>
#include <QList>
#include <DStandardItem>
#include <DBackgroundGroup>

#include "ui/interfaces/frameinterface.h"
#include "base/file_util.h"
#include "service/power_manager.h"
#include "service/screen_brightness.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "sysinfo/users.h"
#include "sysinfo/virtual_machine.h"
#include "third_party/global_shortcut/global_shortcut.h"
#include "ui/delegates/language_delegate.h"
#include "ui/delegates/main_window_util.h"
#include "ui/frames/confirm_quit_frame.h"
#include "ui/frames/control_panel_frame.h"
#include "ui/frames/disk_space_insufficient_frame.h"
#include "ui/frames/install_progress_frame.h"
#include "ui/frames/partition_frame.h"
#include "ui/frames/privilege_error_frame.h"
#include "ui/frames/language_frame.h"
#include "ui/frames/system_info_frame.h"
#include "ui/frames/timezone_frame.h"
#include "ui/frames/virtual_machine_frame.h"
#include "ui/frames/saveinstallfailedlogframe.h"
#include "ui/frames/install_component_frame.h"
#include "ui/frames/install_results_frame.h"

#include "ui/utils/widget_util.h"
#include "ui/widgets/pointer_button.h"
#include "ui/xrandr/multi_head_manager.h"

Q_DECLARE_METATYPE(installer::FrameInterface*)

DWIDGET_USE_NAMESPACE

namespace installer {

MainWindow::MainWindow(QWidget* parent)
    : DMainWindow(parent),
      FrameProxyInterface(),
      pages_(),
      prev_page_(PageId::NullId),
      current_page_(PageId::NullId),
      log_file_(),
      auto_install_(false),
      m_showPastFrame(false)
{
    this->setObjectName("main_window");

    this->initUI();
    this->initPages();
    this->registerShortcut();
    this->initConnections();

    SetBrightness(GetSettingsInt(kScreenDefaultBrightness));
    WriteDisplayPort(getenv("DISPLAY"));

    Q_ASSERT(m_frames.count() > 0);
    m_frames.first()->init();

    updateFrameLabelState(m_frames.first(), FrameLabelState::Show);
    stacked_layout_->setCurrentWidget(m_frames.first());
}

void MainWindow::fullscreen() {
  if (auto_install_) {
    // Read default locale from settings.ini and go to InstallProgressFrame.
    current_page_ = PageId::PartitionId;

    // Set language.
    QTranslator* translator = new QTranslator(this);
    const QString locale(ReadLocale());
    translator->load(GetLocalePath(locale));
    qApp->installTranslator(translator);
  }

#ifndef QT_DEBUG
    multi_head_manager_->updateWallpaper();
#endif // !QT_DEBUG

  ShowFullscreen(this);

  if (auto_install_) {
    // In auto-install mode, partitioning is done in hook script.
    // So notify InstallProgressFrame to run hooks directly.
    partition_frame_->autoPart();
  }
}

void MainWindow::scanDevicesAndTimezone() {
  // Do nothing in auto-install mode.
  if (auto_install_) {
    return;
  }

  // If timezone page is not skipped, scan wireless hot spot and update
  // timezone in background.
  if (!GetSettingsBool(kSkipTimezonePage)) {
    timezone_frame_->init();
  }

  if (!GetSettingsBool(kSkipPartitionPage) &&
      !GetSettingsBool(kPartitionDoAutoPart)) {
    // Notify background thread to scan device list.
    // When device is refreshed in partition_frame_, call fullscreen() method
    // to display main window.
    partition_frame_->scanDevices();
  }
}

void MainWindow::setEnableAutoInstall(bool auto_install) {
  auto_install_ = auto_install;
}

void MainWindow::setLogFile(const QString& log_file) {
    log_file_ = log_file;
}

void MainWindow::nextFrame()
{
    FrameInterface* frame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
    Q_ASSERT(frame != nullptr);

    frame->finished();
    updateFrameLabelState(frame, FrameLabelState::FinishedConfig);

    if (!m_showPastFrame){
        m_frames.removeFirst();
    }

    for (auto it = m_frames.begin(); it != m_frames.end();) {
        if ((*it)->shouldDisplay()){
            // If the current display page is the fallback page clicked by the user
            // , then, the next page is the one that has displayed earlier.
            // If not, then, the next page is the one that displayed for the first time
            // , so, must be initial.
            if (!m_showPastFrame){
                (*it)->init();
            }

            updateFrameLabelState(*it, FrameLabelState::Show);
            stacked_layout_->setCurrentWidget(*it);
            m_showPastFrame = false;
            break;
        }
        else {
            (*it)->init();
            (*it)->finished();
            it = m_frames.erase(it);
            m_showPastFrame = false;
        }
    }

    // TODO: reboot or shutdown
}

void MainWindow::previousFrameSelected(FrameInterface* frame)
{
    FrameInterface* currentFrame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
    Q_ASSERT(currentFrame != nullptr);

    if (m_showPastFrame){
        updateFrameLabelState(currentFrame, FrameLabelState::FinishedConfig);
    }
    else {
        updateFrameLabelState(currentFrame, FrameLabelState::Initial);
    }

    updateFrameLabelState(frame, FrameLabelState::Show);
    stacked_layout_->setCurrentWidget(frame);

    m_showPastFrame = true;
}

void MainWindow::onFrameLabelsViewClicked(const QModelIndex& index)
{
    Q_ASSERT(sender() == m_frameLabelsView);

    FrameInterface* framePointer = index.data(FramePointerRole).value<FrameInterface*>();
    Q_ASSERT(framePointer);
    if (!m_frameModelItemMap[framePointer]->flags().testFlag(Qt::ItemFlag::ItemIsEnabled)){
        // TODO: will user another way implement.
        FrameInterface* frame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
        Q_ASSERT(frame);
        m_frameLabelsView->setCurrentIndex(m_frameLabelsModel->indexFromItem(
                                               m_frameModelItemMap[frame]));
        return;
    }

    previousFrameSelected(framePointer);
}

void MainWindow::showChildFrame(FrameInterface *frame) {

}

void MainWindow::exitInstall(bool reboot)
{
    return reboot ? rebootSystem() : shutdownSystem();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    stacked_layout_->setCurrentWidget(confirm_quit_frame_);
}

void MainWindow::initConnections() {
  connect(confirm_quit_frame_, &ConfirmQuitFrame::quitCancelled,
          this, [=](){
             setCurrentPage(prev_page_);
             prev_page_ = PageId::NullId;
          });
  connect(confirm_quit_frame_, &ConfirmQuitFrame::quitConfirmed,
          this, &MainWindow::shutdownSystem);

  connect(control_panel_frame_, &ControlPanelFrame::currentPageChanged,
          this, &MainWindow::onCurrentPageChanged);
  connect(control_panel_frame_, &ControlPanelFrame::requestRefreshDevices,
          partition_frame_, &PartitionFrame::scanDevices);

  connect(control_panel_frame_, &ControlPanelFrame::requestSimulateSlide,
          install_progress_frame_, &InstallProgressFrame::simulate);

  connect(disk_space_insufficient_frame_, &DiskSpaceInsufficientFrame::finished,
          this, &MainWindow::shutdownSystem);

  connect(install_progress_frame_, &InstallProgressFrame::finished,
          this, &MainWindow::goNextPage);

  connect(partition_frame_, &PartitionFrame::reboot,
          this, &MainWindow::rebootSystem);
  connect(partition_frame_, &PartitionFrame::finished,
          this, &MainWindow::goNextPage);

  connect(privilege_error_frame_, &PrivilegeErrorFrame::finished,
          this, &MainWindow::goNextPage);

  connect(select_language_frame_, &LanguageFrame::finished,
          this, &MainWindow::goNextPage);

  connect(select_language_frame_, &LanguageFrame::timezoneUpdated,
          timezone_frame_, &TimezoneFrame::updateTimezoneBasedOnLanguage);
  connect(timezone_frame_, &TimezoneFrame::finished,
          this, &MainWindow::goNextPage);

  connect(virtual_machine_frame_, &VirtualMachineFrame::finished,
          this, &MainWindow::goNextPage);

  // Notify InstallProgressFrame that partition job has finished.
  connect(partition_frame_, &PartitionFrame::autoPartDone,
          install_progress_frame_, &InstallProgressFrame::runHooks);
  connect(partition_frame_, &PartitionFrame::manualPartDone,
          install_progress_frame_, &InstallProgressFrame::runHooks);

  connect(multi_head_manager_, &MultiHeadManager::primaryScreenChanged,
          this, &MainWindow::onPrimaryScreenChanged);

  connect(control_panel_shortcut_, &QShortcut::activated,
          control_panel_frame_, &ControlPanelFrame::toggleVisible);
  connect(monitor_mode_shortcut_, &GlobalShortcut::activated,
          multi_head_manager_, &MultiHeadManager::switchXRandRMode);
  connect(brightness_increase_shortcut_, &QShortcut::activated,
          IncreaseBrightness);
  connect(brithtness_decrease_shortcut_, &QShortcut::activated,
          DecreaseBrightness);

  connect(back_button_, &DImageButton::clicked, this, &MainWindow::backPage);

  connect(m_selectComponentFrame, &SelectInstallComponentFrame::finished,
          this, &MainWindow::goNextPage);

  connect(save_failedLog_frame_, &SaveInstallFailedLogFrame::requestBack, this, &MainWindow::backPage);

  connect(m_frameLabelsView, &DListView::clicked, this, &MainWindow::onFrameLabelsViewClicked);
}

void MainWindow::initPages() {
  confirm_quit_frame_ = new ConfirmQuitFrame(this);
  pages_.insert(PageId::ConfirmQuitId,
               stacked_layout_->addWidget(confirm_quit_frame_));

  select_language_frame_ = new LanguageFrame(this);
  pages_.insert(PageId::SelectLanguageId,
                stacked_layout_->addWidget(select_language_frame_));

  disk_space_insufficient_frame_ = new DiskSpaceInsufficientFrame(this);
  pages_.insert(PageId::DiskSpaceInsufficientId,
                stacked_layout_->addWidget(disk_space_insufficient_frame_));

  install_progress_frame_ = new InstallProgressFrame(this);
  pages_.insert(PageId::InstallProgressId,
                stacked_layout_->addWidget(install_progress_frame_));

  partition_frame_ = new PartitionFrame(this);
  pages_.insert(PageId::PartitionId,
                stacked_layout_->addWidget(partition_frame_));

  privilege_error_frame_ = new PrivilegeErrorFrame(this);
  pages_.insert(PageId::PrivilegeErrorId,
                stacked_layout_->addWidget(privilege_error_frame_));

  system_info_frame_ = new SystemInfoFrame(this);
  pages_.insert(PageId::SystemInfoId,
                stacked_layout_->addWidget(system_info_frame_));

  timezone_frame_ = new TimezoneFrame(this);
  pages_.insert(PageId::TimezoneId,
                stacked_layout_->addWidget(timezone_frame_));

  virtual_machine_frame_ = new VirtualMachineFrame(this);
  pages_.insert(PageId::VirtualMachineId,
                stacked_layout_->addWidget(virtual_machine_frame_));

  m_selectComponentFrame = new SelectInstallComponentFrame(this);
  pages_.insert(PageId::SelectComponentId,
                stacked_layout_->addWidget(m_selectComponentFrame));

  m_installResultsFrame = new InstallResultsFrame(this);
  pages_.insert(PageId::InstallResultsId, stacked_layout_->addWidget(m_installResultsFrame));

  save_failedLog_frame_ = new SaveInstallFailedLogFrame;
  stacked_layout_->addWidget(save_failedLog_frame_);

  m_originalFrames = {
      // TODO: move the front new statement over here
      privilege_error_frame_,
      select_language_frame_,
      virtual_machine_frame_,
      m_selectComponentFrame,
      partition_frame_,
      install_progress_frame_,
      m_installResultsFrame,
  };

  for (FrameInterface* frame : m_originalFrames){
      // TODO: move the front addWidget statement over here
      m_frames << frame;
  }

  // TODO: for current test, will be replaced later.
  m_frameTitles = {
      "PrivilegeFrame",
      "LanguageFrame",
      "VirtualMachineFrame",
      "SelectComponentFrame",
      "PartitionFrame",
      "InstallProgressFrame",
      "InstallResultsFrame"
  };

  m_frameLabelsView = new DListView(this);
  m_frameLabelsView->setOrientation(QListView::TopToBottom, true);
  m_frameLabelsView->setItemSize(QSize(250, 80));
  m_frameLabelsModel = new QStandardItemModel();
  m_frameLabelsView->setModel(m_frameLabelsModel);

  for (FrameInterface* frame : m_originalFrames){
      if (!frame->shouldDisplay()){
          continue;
      }

      DStandardItem* item = new DStandardItem;
      item->setIcon(QIcon(installer::renderPixmap(":/images/NO_inactive.svg")));
      // TODO: for current test, will be replaced in another way.
      item->setText(m_frameTitles[m_originalFrames.indexOf(frame)]);
      QVariant framePointer = QVariant::fromValue(frame);
      item->setData(framePointer, FramePointerRole);
      item->setFlags(Qt::ItemFlag::NoItemFlags);

      DViewItemAction* action = new DViewItemAction;
      action->setIcon(QIcon(installer::renderPixmap(":/images/done_inactive.svg")));
      action->setVisible(false);
      item->setActionList(Qt::Edge::RightEdge, {action});

      m_frameLabelsModel->appendRow(item);
      m_frameModelItemMap[frame] = item;
  }

  m_frameSelectedLayout->addSpacing(80);
  m_frameSelectedLayout->addWidget(m_frameLabelsView, 0, Qt::AlignHCenter);
}

void MainWindow::initUI() {
  background_label_ = new QLabel(this);

  back_button_ = new DImageButton(this);
  back_button_->setObjectName("back_button");
  back_button_->setFixedSize(48, 38);
  back_button_->move(20, 20);
  back_button_->hide();
  back_button_->setNormalPic(":/images/back_normal.svg");
  back_button_->setHoverPic(":/images/back_hover.svg");
  back_button_->setPressPic(":/images/back_pressed.svg");
  back_button_->setDisabledPic(":/images/back_disabled.svg");

  stacked_layout_ = new QStackedLayout();

  QVBoxLayout* vbox_layout = new QVBoxLayout();
  vbox_layout->setContentsMargins(0, 0, 0, 0);
  vbox_layout->setSpacing(0);
  vbox_layout->addLayout(stacked_layout_);
  vbox_layout->addSpacing(32);

  QWidget* contentWidget = new QWidget;
  contentWidget->setLayout(vbox_layout);

  m_frameSelectedLayout = new QVBoxLayout;
  m_frameSelectedLayout->setMargin(0);
  m_frameSelectedLayout->setSpacing(0);

  QWidget* frameSelectedListWidget = new QWidget;
  frameSelectedListWidget->setObjectName("frameSelectedListWidget");
  frameSelectedListWidget->setLayout(m_frameSelectedLayout);
  frameSelectedListWidget->setFixedWidth(300);

  QHBoxLayout* mainLayout = new QHBoxLayout;
  mainLayout->setMargin(0);
  mainLayout->setSpacing(0);

  mainLayout->addWidget(frameSelectedListWidget);
  mainLayout->addSpacing(1);
  mainLayout->addWidget(contentWidget);

  DBackgroundGroup* bgGroup = new DBackgroundGroup;
  bgGroup->setContentsMargins(10, 10, 10, 10);
  bgGroup->setLayout(mainLayout);

  setCentralWidget(bgGroup);

  control_panel_frame_ = new ControlPanelFrame(this);
  control_panel_frame_->hide();

  multi_head_manager_ = new MultiHeadManager(this);
  back_button_->raise();
}

void MainWindow::registerShortcut() {
  control_panel_shortcut_ = new QShortcut(QKeySequence("Ctrl+L"), this);
  control_panel_shortcut_->setContext(Qt::ApplicationShortcut);
  // Note(xushaohua): Super key is named Meta key in Qt namespace.
  monitor_mode_shortcut_ = new GlobalShortcut(QKeySequence("Meta+P"), this);
  if (!monitor_mode_shortcut_->registerNow()) {
    qWarning() << "Failed to register global shortcut of Meta+P"
               << "Fallback to Ctrl+Alt+P";
    delete monitor_mode_shortcut_;
    monitor_mode_shortcut_ = new GlobalShortcut(QKeySequence("Ctrl+Alt+P"),
                                                this);
    if (!monitor_mode_shortcut_->registerNow()) {
      qWarning() << "Failed to register global shortcut of Ctrl+Alt+P";
    }
  }

  brightness_increase_shortcut_ = new QShortcut(QKeySequence("Ctrl+="), this);
  brithtness_decrease_shortcut_ = new QShortcut(QKeySequence("Ctrl+-"), this);
}

void MainWindow::saveLogFile() {
  if (!log_file_.isEmpty()) {
    // Copy log file.
    CopyLogFile(log_file_);
  }
}

void MainWindow::updateWidgetVisible()
{
    const bool checkVisible{ checkBackButtonAvailable(current_page_) };
    back_button_->setVisible(m_old_frames.size() > 1 && checkVisible);

    // Raise control_panel_frame explicitly.
    if (control_panel_frame_->isVisible()) {
        control_panel_frame_->raise();
    }

    back_button_->raise();
}

void MainWindow::setCurrentPage(PageId page_id) {
  Q_ASSERT(pages_.contains(page_id));
  Q_ASSERT(page_id != PageId::NullId);

  prev_page_ = current_page_;
  current_page_ = page_id;
  stacked_layout_->setCurrentIndex(pages_.value(page_id));

  updateWidgetVisible();
}

void MainWindow::backPage()
{
    // 在goNextPage()中已经更新过返回按钮的可见了
    // 第一页是看不到返回按钮的，这个函数也不会调用。
    Q_ASSERT(!m_old_frames.isEmpty());

    m_old_frames.takeLast();
    QWidget* frame = m_old_frames.last();
    PageId id = pages_.key(stacked_layout_->indexOf(frame));
    setCurrentPage(id);

    back_button_->setVisible(m_old_frames.size() > 1 && checkBackButtonAvailable(id));
}

bool MainWindow::checkBackButtonAvailable(PageId id) {
    return !QList<PageId>({
                             PageId::ConfirmQuitId,
                             PageId::InstallProgressId,
                         })
            .contains(id);
}

void MainWindow::updateFrameLabelState(FrameInterface *frame, FrameLabelState state)
{
    if (!m_frameModelItemMap.contains(frame)){
        return;
    }

    DStandardItem* item = m_frameModelItemMap[frame];
    switch (state) {
    case FrameLabelState::Initial:
        item->actionList(Qt::Edge::RightEdge).first()->setVisible(false);
        item->setFlags(Qt::ItemFlag::NoItemFlags);
        break;
    case FrameLabelState::Show:
        item->actionList(Qt::Edge::RightEdge).first()->setVisible(false);
        item->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);

        m_frameLabelsView->setCurrentIndex(m_frameLabelsModel->indexFromItem(m_frameModelItemMap[frame]));
        break;
    case FrameLabelState::FinishedConfig:
        item->actionList(Qt::Edge::RightEdge).first()->setVisible(true);
        item->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
        break;
    default:
        qWarning() << "invalid state value";
        break;
    }
}

void MainWindow::onCurrentPageChanged(int index) {
  // Ignore null id.
    int a = index;
  const PageId id = static_cast<PageId>(index + 1);
  this->setCurrentPage(id);
}

void MainWindow::onPrimaryScreenChanged(const QRect& geometry) {
  qDebug() << "onPrimaryScreenChanged()" << geometry;
  ShowFullscreen(this, geometry, devicePixelRatioF());
}

void MainWindow::goNextPage() {
    // Page order:
    //   * privilege error frame;
    //   * select language frame;
    //   * disk space insufficient page;
    //   * virtual machine page;
    //   * system info page;
    //   * timezone page;
    //   * partition page;
    //   * install progress page;
    //   * install success page or install failed page;
    // And confirm-quit-page can be triggered at any moment except in
    // install progress page.

    bool isMainPage = false;

    switch (current_page_) {
    case PageId::NullId: {
        if (HasRootPrivilege()) {
            prev_page_ = current_page_;
            current_page_ = PageId::PrivilegeErrorId;
        } else {
            this->setCurrentPage(PageId::PrivilegeErrorId);
            break;
        }
    }

    case PageId::PrivilegeErrorId: {
        select_language_frame_->init();
        if (GetSettingsBool(kSkipSelectLanguagePage)) {
            select_language_frame_->finished();
            prev_page_ = current_page_;
            current_page_ = PageId::SelectLanguageId;
        } else {
            isMainPage = true;
            this->setCurrentPage(PageId::SelectLanguageId);
            break;
        }
    }

    case PageId::SelectLanguageId: {
        // Check whether to show DiskSpaceInsufficientPage.
        if (!GetSettingsBool(kSkipDiskSpaceInsufficientPage) &&
                IsDiskSpaceInsufficient()) {
            isMainPage = true;
            this->setCurrentPage(PageId::DiskSpaceInsufficientId);
            break;
        }
        else {
            prev_page_ = current_page_;
            current_page_ = PageId::DiskSpaceInsufficientId;
        }
    }

    case PageId::DiskSpaceInsufficientId: {
        // Check whether to show VirtualMachinePage.
        if (!GetSettingsBool(kSkipVirtualMachinePage) &&
                IsVirtualMachine()) {
            this->setCurrentPage(PageId::VirtualMachineId);
            break;
        } else {
            prev_page_ = current_page_;
            current_page_ = PageId::VirtualMachineId;
        }
    }

    case PageId::VirtualMachineId: {
        // Check whether to show SystemInfoPage.
        system_info_frame_->init();
        if (GetSettingsBool(kSystemInfoSetupAfterReboot) || GetSettingsBool(kSkipSystemInfoPage)) {
            system_info_frame_->finished();
            prev_page_ = current_page_;
            current_page_ = PageId::SystemInfoId;
        } else {
            isMainPage = true;
            this->setCurrentPage(PageId::SystemInfoId);
            break;
        }
    }

    case PageId::SystemInfoId: {
        if (GetSettingsBool(kSystemInfoSetupAfterReboot) || GetSettingsBool(kSkipTimezonePage)) {
            timezone_frame_->finished();
            prev_page_ = current_page_;
            current_page_ = PageId::TimezoneId;
        } else {
            isMainPage = true;
            this->setCurrentPage(PageId::TimezoneId);
            break;
        }
    }

    case PageId::TimezoneId: {
      //Check whether to show SelectComponentPage.
#ifdef QT_DEBUG
        if (false) {
#else
        if(GetSettingsBool(kSkipSelectComponentPage)){
#endif // QT_DEBUG
            prev_page_ = current_page_;
            current_page_ = PageId::SelectComponentId; //PageId::PartitionId;
        } else {
            m_selectComponentFrame->init();
            isMainPage = true;
            this->setCurrentPage(PageId::SelectComponentId); //(PageId::PartitionId);
            break;
        }
    }

    case PageId::SelectComponentId: {
        m_selectComponentFrame->finished();

        // Check whether to show PartitionPage.
        if (GetSettingsBool(kSkipPartitionPage)) {
            if (GetSettingsBool(kPartitionDoAutoPart)) {
                partition_frame_->autoPart();
            }
            prev_page_ = current_page_;
            current_page_ = PageId::PartitionId;
        }
        else{
            isMainPage = true;
            this->setCurrentPage(PageId::PartitionId);
            break;
        }
    }

    case PageId::PartitionId: {
        // Show InstallProgressFrame.
        isMainPage = true;
        install_progress_frame_->startSlide();
        this->setCurrentPage(PageId::InstallProgressId);
        break;
    }

    case PageId::InstallProgressId: {
        isMainPage = true;
        break;
    }

    default: {
        qWarning() << "[MainWindow]::goNextPage() We shall never reach here"
                   << static_cast<int>(current_page_) << this->sender();
        break;
    }
    }

    if(isMainPage){
        m_old_frames << stacked_layout_->currentWidget();
    }

    updateWidgetVisible();
}

void MainWindow::rebootSystem() {
  this->saveLogFile();

  if (!RebootSystemWithMagicKey()) {
      qWarning() << "RebootSystemWithMagicKey() failed!";
  }

  if (!RebootSystem()) {
      qWarning() << "RebootSystem() failed!";
  }
}

void MainWindow::shutdownSystem() {
  this->saveLogFile();

  if (!ShutdownSystemWithMagicKey()) {
      qWarning() << "ShutdownSystemWithMagicKey() failed!";
  }

  if (!ShutdownSystem()) {
      qWarning() << "ShutdownSystem() failed!";
  }
}

void MainWindow::showSaveLogFrame()
{
    stacked_layout_->setCurrentWidget(save_failedLog_frame_);
    save_failedLog_frame_->startDeviceWatch(true);
    m_old_frames << stacked_layout_->currentWidget();
}

}  // namespace installer
