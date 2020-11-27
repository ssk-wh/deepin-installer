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
#include <QDesktopWidget>
#include <DStandardItem>
#include <DBackgroundGroup>
#include <DTitlebar>

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
#include "ui/frames/install_component_frame.h"
#include "ui/frames/install_results_frame.h"
#include "ui/frames/repair_system_frame.h"
#include "ui/widgets/shadow_widget.h"
#include "ui/frames/repair_system_frame.h"
#include "ui/frames/inner/system_info_keyboard_frame.h"
#include "ui/frames/networkframe.h"

#include "ui/utils/widget_util.h"
#include "ui/widgets/pointer_button.h"
#include "ui/xrandr/multi_head_manager.h"

Q_DECLARE_METATYPE(installer::FrameInterface*)

DWIDGET_USE_NAMESPACE

namespace {
    const int kLeftViewWidth = 200;
    const int kLeftViewItemWidth = 180;
    const int kLeftViewItemHeight = 36;
    const int kLeftViewItemSpacing = 10;
}

namespace installer {

MainWindow::MainWindow(QWidget* parent)
    : DMainWindow(parent),
      FrameProxyInterface(),
      shadow_widget(new ShadowWidget),
      pages_(),
      prev_page_(PageId::NullId),
      current_page_(PageId::NullId),
      log_file_(),
      //auto_install_(false),
      m_showPastFrame(false)
{
    this->setObjectName("main_window");

    this->initUI();
    this->initPages();
    this->registerShortcut();
    this->initConnections();
    DTitlebar* titleBar = titlebar();
    titleBar->installEventFilter(this);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint & ~Qt::WindowMinMaxButtonsHint);

    titlebar()->setMenuVisible(false);
    titlebar()->setFullScreenButtonVisible(false);

    SetBrightness(GetSettingsInt(kScreenDefaultBrightness));
    WriteDisplayPort(getenv("DISPLAY"));

    for (auto it = m_frames.begin(); it != m_frames.end();) {
        if ((*it)->shouldDisplay()) {
            break;
        }
        else {
            (*it)->init();
            (*it)->finished();
            it = m_frames.erase(it);
        }
    }

    stacked_layout_->setCurrentWidget(nullptr);

    Q_ASSERT(m_frames.count() > 0);
    m_frames.first()->init();

    if (m_frames.first()->frameType() == FrameType::Frame) {
        stacked_layout_->setCurrentWidget(m_frames.first());
        updateFrameLabelState(m_frames.first(), FrameLabelState::Show);
    }
    else if (m_frames.first()->frameType() == FrameType::NoLeftLabelExtFrame) {
        stacked_layout_->setCurrentWidget(m_frames.first());
    }
    else if (m_frames.first()->frameType() == FrameType::FullScreenExtFrame) {
        showExtFrameFullscreen(m_frames.first());
    }
    else {
        showChildFrame(m_frames.first());
    }
}

void MainWindow::fullscreen() {
  if (GetSettingsBool(kPartitionDoAutoPart)) {
    // Read default locale from settings.ini and go to InstallProgressFrame.
    // 当配置了页面跳过和自动安装时，对于磁盘空间检测的异常需要有异常提示界面
    // current_page_ = PageId::PartitionId;

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
    // this->showFullScreen();

    if (GetSettingsBool(kPartitionDoAutoPart) || GetSettingsBool("DI_LUPIN")) {
    // In auto-install mode, partitioning is done in hook script.
    // So notify InstallProgressFrame to run hooks directly.
    partition_frame_->autoPart();
  }
}

void MainWindow::scanDevicesAndTimezone() {
  // Do nothing in auto-install mode.
  if (GetSettingsBool(kPartitionDoAutoPart)) {
    return;
  }

  // If timezone page is not skipped, scan wireless hot spot and update
  // timezone in background.
  if (!GetSettingsBool(kSkipTimezonePage)) {
    timezone_frame_->init();
  }

   if (!GetSettingsBool("DI_LUPIN")) {
    partition_frame_->scanDevices();
   }
}

//void MainWindow::setEnableAutoInstall(bool auto_install) {
//  auto_install_ = auto_install;
//  disk_space_insufficient_frame_->setEnableAutoInstall(auto_install);
//}

void MainWindow::setLogFile(const QString& log_file) {
    log_file_ = log_file;
}

void MainWindow::nextFrame()
{
    FrameInterface* frame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
    Q_ASSERT(frame != nullptr);

    frame->finished();

    if (frame->frameType() == FrameType::Frame) {
        updateFrameLabelState(frame, FrameLabelState::FinishedConfig);
        m_hasShowFrames << frame;
    }

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

            if ((*it)->frameType() == FrameType::Frame) {
                stacked_layout_->setCurrentWidget(*it);
                // Can only appear back or not back, to traverse the updates
                if ((*it)->allowPrevious() != m_currentAllowPreviousState || !(*it)->allowPrevious()) {
                    updateFrameLabelPreviousState((*it)->allowPrevious());
                    m_currentAllowPreviousState = (*it)->allowPrevious();
                }

                updateFrameLabelState(*it, FrameLabelState::Show);
            }
            else if ((*it)->frameType() == FrameType::NoLeftLabelExtFrame) {
                stacked_layout_->setCurrentWidget(*it);
            }
            else if ((*it)->frameType() == FrameType::FullScreenExtFrame) {
                showExtFrameFullscreen(*it);
            }
            else {
                // FrameType::ChildFrame and FrameType::PopupExtFrame display mode.
                showChildFrame(*it);
            }
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

    QModelIndex currentIndex = m_frameLabelsView->currentIndex();

    FrameInterface* currentFrame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
    Q_ASSERT(currentFrame);
    if (currentFrame->frameType() != FrameType::Frame) {
        // TODO: will user another way implement.
        m_frameLabelsView->setCurrentIndex(currentIndex);
        return;
    }

    FrameInterface* framePointer = index.data(FramePointerRole).value<FrameInterface*>();
    Q_ASSERT(framePointer);
    if (framePointer == currentFrame) {
        // TODO: will user another way implement.
        m_frameLabelsView->setCurrentIndex(currentIndex);
        return;
    }

    if (!m_frameModelItemMap[framePointer]->flags().testFlag(Qt::ItemFlag::ItemIsEnabled)){
        // TODO: will user another way implement.
        m_frameLabelsView->setCurrentIndex(currentIndex);
        return;
    }

    previousFrameSelected(framePointer);
}

void MainWindow::coverFrameLabelsView(bool cover) const
{
    if (cover) {
        if (m_frameLabelsViewCoverWidget->isVisible()) {
            return;
        }

        m_frameLabelsViewCoverWidget->setFixedSize(m_frameSelectedListWidget->size());
        m_frameLabelsViewCoverWidget->raise();
        m_frameLabelsViewCoverWidget->show();
    }
    else {
        m_frameLabelsViewCoverWidget->hide();
    }
}

void MainWindow::exitInstall(bool reboot)
{
    return reboot ? rebootSystem() : shutdownSystem();
}

void MainWindow::showChildFrame(BaseFrameInterface* childFrameInterface)
{
    if (shadow_widget->isVisible()) {
        return;
    }
    shadow_widget->setContent(childFrameInterface);
    shadow_widget->setParent(this);
    shadow_widget->setGeometry(rect());
    shadow_widget->raise();
    shadow_widget->show();
}

void MainWindow::hideChildFrame() const
{
    shadow_widget->eraseContent();
    shadow_widget->hide();
}

void MainWindow::showExtFrameFullscreen(BaseFrameInterface* childFrameInterface)
{
    Q_UNUSED(childFrameInterface);
    m_frameSelectedListWidget->hide();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    shadow_widget->setFixedSize(event->size());

    if (close_button_) {
      const int marginSize = this->layout()->margin();
      close_button_->move(width() - close_button_->width() - marginSize, marginSize);
      close_button_->raise();
    }

    DMainWindow::resizeEvent(event);
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if (target == titlebar()) {
        if(event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::MouseButton::RightButton) {
                return true;
            }
        }
    }

    if (m_frameLabelsView != nullptr && target == m_frameLabelsView->viewport()) {
        if (event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::MouseButton::RightButton
                || mouseEvent->button() == Qt::MouseButton::MiddleButton) {
                return true;
            }
        }
    }

    return DMainWindow::eventFilter(target, event);
}

void MainWindow::onCloseEvent()
{
    confirm_quit_frame_->display();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        for (int i = 0; i < m_frameLabelsModel->rowCount(); ++i) {
            QStandardItem* item = m_frameLabelsModel->item(i);
            FrameInterface* frame = getFrameInterface(item);
            if (frame) {
                item->setText(frame->returnFrameName());
                item->setToolTip(frame->returnFrameName());
            }
        }
    }
    else {
        QWidget::changeEvent(event);
    }
}

void MainWindow::initConnections() {
  connect(close_button_, &DImageButton::clicked, this, &MainWindow::onCloseEvent);

  connect(confirm_quit_frame_, &ConfirmQuitFrame::quitCancelled, this, [=](){
             confirm_quit_frame_->close();
             qApp->setActiveWindow(this);
          });
  connect(confirm_quit_frame_, &ConfirmQuitFrame::quitConfirmed,
          this, &MainWindow::shutdownSystem);

  connect(control_panel_frame_, &ControlPanelFrame::currentPageChanged,
          this, &MainWindow::onCurrentPageChanged);
  connect(control_panel_frame_, &ControlPanelFrame::requestRefreshDevices,
          partition_frame_, &PartitionFrame::scanDevices);

  connect(control_panel_frame_, &ControlPanelFrame::requestSimulateSlide,
          install_progress_frame_, &InstallProgressFrame::simulate);

  connect(disk_space_insufficient_frame_, &DiskSpaceInsufficientFrame::abortInstall,
          this, &MainWindow::shutdownSystem);

  connect(partition_frame_, &PartitionFrame::reboot,
          this, &MainWindow::rebootSystem);
  connect(partition_frame_, &PartitionFrame::coverMainWindowFrameLabelsView, this, [=] (bool cover) {
      updateFrameLabelPreviousState(!cover);
  });

  connect(select_language_frame_, &LanguageFrame::timezoneUpdated,
          timezone_frame_, &TimezoneFrame::updateTimezoneBasedOnLanguage);

  connect(select_language_frame_, &LanguageFrame::coverMainWindowFrameLabelsView, this, [=] (bool cover) {
      updateFrameLabelPreviousState(!cover);
  });

  connect(m_repairSystemFrame, &RepairSystemFrame::finished,
          this, &MainWindow::goNextPage);

  connect(m_repairSystemFrame, &RepairSystemFrame::installerMode, this, [=] {
      m_frameSelectedListWidget->show();
  });

  connect(m_repairSystemFrame, &RepairSystemFrame::repair,
          qApp, &QApplication::quit);

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

  connect(m_installResultsFrame, &InstallResultsFrame::successFinished, this, [=] {
      GetSettingsBool(kRebootWhenInstallFinished) ?
          this->rebootSystem() : this->shutdownSystem();
  });
  connect(m_installResultsFrame, &InstallResultsFrame::failedFinished, this
          , &MainWindow::shutdownSystem);
  connect(m_installResultsFrame, &InstallResultsFrame::closeButtionChange, this
          , &MainWindow::setCloseButtonVisible);
  connect(m_installResultsFrame, &InstallResultsFrame::updateQuitFrameTs, this, [=] (bool result) {
      if (result) {
          confirm_quit_frame_->updateTsForSuccessPage();
      }
  });

  connect(install_progress_frame_, &InstallProgressFrame::closeButtionChange,
          this, &MainWindow::setCloseButtonVisible);

  connect(m_frameLabelsView, &DListView::clicked, this, &MainWindow::onFrameLabelsViewClicked);

  connect(m_repairSystemFrame, &RepairSystemFrame::repair, this, [=] {
    qInfo() << "System repair...";
    this->close();
  });
}

void MainWindow::initPages() {
  confirm_quit_frame_ = new ConfirmQuitFrame(this);
  confirm_quit_frame_->hide();

  m_repairSystemFrame = new RepairSystemFrame(this);
  stacked_layout_->addWidget(m_repairSystemFrame);

  select_language_frame_ = new LanguageFrame(this);
  stacked_layout_->addWidget(select_language_frame_);

  disk_space_insufficient_frame_ = new DiskSpaceInsufficientFrame(this);

  install_progress_frame_ = new InstallProgressFrame(this);
  stacked_layout_->addWidget(install_progress_frame_);

  partition_frame_ = new PartitionFrame(this);
  stacked_layout_->addWidget(partition_frame_);

  privilege_error_frame_ = new PrivilegeErrorFrame(this);

  system_info_frame_ = new SystemInfoFrame(this);
  stacked_layout_->addWidget(system_info_frame_);

  timezone_frame_ = new TimezoneFrame(this);
  stacked_layout_->addWidget(timezone_frame_);

  virtual_machine_frame_ = new VirtualMachineFrame(this);
  stacked_layout_->addWidget(virtual_machine_frame_);

  m_selectComponentFrame = new SelectInstallComponentFrame(this);
  stacked_layout_->addWidget(m_selectComponentFrame);

  m_installResultsFrame = new InstallResultsFrame(this);
  stacked_layout_->addWidget(m_installResultsFrame);

  network_frame_ = new NetworkFrame(this);
  stacked_layout_->addWidget(network_frame_);

  m_originalFrames = {
      // TODO: move the front new statement over here
#ifdef QT_DEBUG_TEST

#else
      m_repairSystemFrame,
      privilege_error_frame_,
      select_language_frame_,
      disk_space_insufficient_frame_,
      virtual_machine_frame_,
      network_frame_,
      timezone_frame_,
      system_info_frame_,
      m_selectComponentFrame,
      partition_frame_,
#endif // QT_DEBUG
      install_progress_frame_,
      m_installResultsFrame,
  };

  for (FrameInterface* frame : m_originalFrames){
      // TODO: move the front addWidget statement over here
      m_frames << frame;
  }

  FrameInterface* network = network_frame_;
  if (network->shouldDisplay()) {
      QDBusInterface ddeNetworkMnager(
                "com.deepin.daemon.Network",
                "/com/deepin/daemon/Network",
                "com.deepin.daemon.Network",
                QDBusConnection::sessionBus());

      // If dde-daemon is exists, it will interfere with the network configuration
      // in deepin-installer. The solution for now is to fake it and have a scare.
      // Then the network connection settings is stable.
      if (ddeNetworkMnager.isValid()) {
          qInfo() << "Fake dde-daemon and have a scare.";
          network_frame_->shockDdeDaemon();
      }
  }

  m_frameLabelsView = new DListView(this);
  m_frameLabelsView->viewport()->installEventFilter(this);
  m_frameLabelsView->setResizeMode(QListView::Adjust);
  m_frameLabelsView->setItemSize(QSize(kLeftViewItemWidth, kLeftViewItemHeight + kLeftViewItemSpacing));
  m_frameLabelsModel = new QStandardItemModel();
  m_frameLabelsView->setModel(m_frameLabelsModel);

  m_frameSelectedLayout->addSpacing(60);
  m_frameSelectedLayout->addWidget(m_frameLabelsView, 0, Qt::AlignHCenter);

  constructLabelView();
}

void MainWindow::constructLabelView()
{
    m_frameLabelsModel->clear();

    int i = 1;
    for (FrameInterface* frame : m_originalFrames){
        if (!frame->shouldDisplay() || frame->frameType() != FrameType::Frame){
            continue;
        }

        DStandardItem* item = new DStandardItem;
        QString pixPathTemplate(":/images/NO_inactive%1.svg");
        item->setIcon(QIcon(pixPathTemplate.arg(i)));
        ++i;

        item->setText(tr(frame->returnFrameName().toLatin1().data()));
        item->setToolTip(frame->returnFrameName());

        QVariant framePointer = QVariant::fromValue(frame);
        item->setData(framePointer, FramePointerRole);
        item->setFlags(Qt::ItemFlag::NoItemFlags);

        DViewItemAction* action = new DViewItemAction(Qt::AlignmentFlag::AlignVCenter);
        action->setIcon(QIcon(":/images/done_inactive.svg"));
        action->setVisible(false);
        item->setActionList(Qt::Edge::RightEdge, {action});

        m_frameLabelsModel->appendRow(item);
        m_frameModelItemMap[frame] = item;
    }
}

void MainWindow::initUI() {
    this->setFocusPolicy(Qt::TabFocus);
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

  // TODO: use titleBar implement.
  close_button_ = new DImageButton(this);
  close_button_->setObjectName("close_button");
  close_button_->setFocusPolicy(Qt::TabFocus);
  close_button_->setFixedSize(40, 40);
  close_button_->setNormalPic(":/images/close_normal.svg");
  close_button_->setHoverPic(":/images/close_normal.svg");
  close_button_->setPressPic(":/images/close_normal.svg");

  stacked_layout_ = new QStackedLayout();
  stacked_layout_->setContentsMargins(0, 0, 0, 0);
  stacked_layout_->setSpacing(0);

  QVBoxLayout* vbox_layout = new QVBoxLayout();
  vbox_layout->setContentsMargins(0, 0, 0, 0);
  vbox_layout->setSpacing(0);
  vbox_layout->addLayout(stacked_layout_);

  QWidget* contentWidget = new QWidget;
  contentWidget->setContentsMargins(0, 0, 0, 0);
  contentWidget->setLayout(vbox_layout);

  m_frameSelectedLayout = new QVBoxLayout;
  m_frameSelectedLayout->setContentsMargins(kLeftViewItemSpacing, 0, kLeftViewItemSpacing, 0);
  m_frameSelectedLayout->setSpacing(0);

  m_frameSelectedListWidget = new QWidget;
  m_frameSelectedListWidget->setObjectName("frameSelectedListWidget");
  m_frameSelectedListWidget->setLayout(m_frameSelectedLayout);
  m_frameSelectedListWidget->setFixedWidth(kLeftViewWidth);

  m_frameLabelsViewCoverWidget = new QWidget(m_frameSelectedListWidget);
  m_frameLabelsViewCoverWidget->setObjectName("frameLabelsViewCoverWidget");
  m_frameLabelsViewCoverWidget->setStyleSheet("{background-color: rgba(255, 255, 255, 0.5);}");
  m_frameLabelsViewCoverWidget->hide();

  QHBoxLayout* mainLayout = new QHBoxLayout;
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  mainLayout->addWidget(m_frameSelectedListWidget);
  mainLayout->addSpacing(1);
  mainLayout->addWidget(contentWidget);

  DBackgroundGroup* bgGroup = new DBackgroundGroup;
  bgGroup->setContentsMargins(10, 10, 10, 10);
  bgGroup->setLayout(mainLayout);

  setContentsMargins(0, 0, 0, 0);
  setCentralWidget(bgGroup);

  control_panel_frame_ = new ControlPanelFrame(this);
  control_panel_frame_->hide();

  multi_head_manager_ = new MultiHeadManager(this);
  back_button_->raise();

  shadow_widget->hide();
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

void MainWindow::setWindowIcon(const QString &path)
{
    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        qDebug() << "set window icon is null. path: " << path;
    } else {
        const int marginSize = this->layout()->margin();
        const int iconSize = this->menuWidget()->height() - 2 * marginSize;

        QLabel *windowLabel = new QLabel;
        windowLabel->setFixedSize(iconSize, iconSize);
        windowLabel->setPixmap(pixmap);
        windowLabel->setParent(this);
        windowLabel->raise();
        windowLabel->setVisible(true);
        windowLabel->move(QPoint(x() + marginSize, y() + marginSize));

        // 设置任务栏窗口图标
        return QMainWindow::setWindowIcon(QIcon(path));
    }
}

void MainWindow::setCloseButtonVisible(bool visible)
{
    close_button_->setVisible(visible);
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

void MainWindow::onPrimaryScreenChanged(const QRect& geometry) {
  qDebug() << "onPrimaryScreenChanged()" << geometry;
  ShowFullscreen(this, geometry);
}

void MainWindow::updateFrameLabelState(FrameInterface *frame, FrameLabelState state)
{
    if (frame->frameType() != FrameType::Frame) {
        return;
    }

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
    case FrameLabelState::Previous:
        item->actionList(Qt::Edge::RightEdge).first()->setVisible(true);
        item->setFlags(Qt::ItemFlag::NoItemFlags);
        break;
    default:
        qWarning() << "invalid state value";
        break;
    }
}

void MainWindow::updateFrameLabelPreviousState(bool allow)
{
    FrameInterface* currentFrame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
    Q_ASSERT(currentFrame != nullptr);
    for (FrameInterface* frame : m_hasShowFrames) {
        if (frame != currentFrame) {
            if (allow) {
                updateFrameLabelState(frame, FrameLabelState::FinishedConfig);
            } else {
                updateFrameLabelState(frame, FrameLabelState::Previous);
            }
        }
    }
}

FrameInterface *MainWindow::getFrameInterface(QStandardItem *item) const
{
    for (auto it = m_frameModelItemMap.begin(); it != m_frameModelItemMap.end(); ++it) {
        if (it.value() == item) {
            return it.key();
        }
    }

    return nullptr;
}

void MainWindow::onCurrentPageChanged(int index) {
  // Ignore null id.
    int a = index;
  const PageId id = static_cast<PageId>(index + 1);
  this->setCurrentPage(id);
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

  if (!ShutdownSystem()) {
      qWarning() << "ShutdownSystem() failed!";
  }

  if (!ShutdownSystemWithMagicKey()) {
      qWarning() << "ShutdownSystemWithMagicKey() failed!";
  }

}

}  // namespace installer
