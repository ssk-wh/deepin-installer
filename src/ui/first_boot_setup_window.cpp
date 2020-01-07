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

#include "ui/first_boot_setup_window.h"

#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QResizeEvent>
#include <QStackedLayout>
#include <QThread>
#include <QMap>
#include <DBackgroundGroup>
#include <linux/vt.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/kd.h>

#include "base/thread_util.h"
#include "service/first_boot_hook_worker.h"
#include "service/power_manager.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "third_party/global_shortcut/global_shortcut.h"
#include "ui/frames/first_boot_loading_frame.h"
#include "ui/frames/system_info_frame.h"
#include "ui/frames/timezone_frame.h"
#include "ui/utils/widget_util.h"
#include "ui/xrandr/multi_head_manager.h"
#include "base/command.h"
#include "base/file_util.h"

#include "ui/frames/language_frame.h"
#include "ui/frames/networkframe.h"
#include "ui/frames/control_platform_frame.h"

DWIDGET_USE_NAMESPACE

namespace installer {

FirstBootSetupWindow::FirstBootSetupWindow(QWidget *parent)
    : DMainWindow(parent),
      hook_worker_thread_(new QThread(this)),
      hook_worker_(new FirstBootHookWorker()),
      m_showPastFrame(false)
{
  this->setObjectName("first_boot_setup_window");

  hook_worker_thread_->start();
  hook_worker_->moveToThread(hook_worker_thread_);

  this->initUI();
  this->registerShortcut();
  this->initConnections();

  Q_ASSERT(m_frames.count() > 0);
  m_frames.first()->init();

  // TODO: update frame label state.
  stacked_layout_->setCurrentWidget(m_frames.first());

  if ( !GetSettingsBool(kSkipSelectLanguagePage) ||
        GetSettingsBool(kSkipSelectLanguagePageOnFirstBoot)) {
      Q_ASSERT(m_frames.first() == language_frame_);
      nextFrame();
  }


  stacked_layout_->setCurrentWidget(system_info_frame_);
}

FirstBootSetupWindow::~FirstBootSetupWindow() {
  QuitThread(hook_worker_thread_);
}

void FirstBootSetupWindow::fullscreen() {
    ShowFullscreen(this);
}

void FirstBootSetupWindow::nextFrame()
{
    FrameInterface* frame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
    Q_ASSERT(frame != nullptr);

    frame->finished();
    // TODO: update frame label state.

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

            // TODO: update frame label state.
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

void FirstBootSetupWindow::showChildFrame(FrameInterface *frame)
{

}

void FirstBootSetupWindow::exitInstall(bool reboot)
{

}

void FirstBootSetupWindow::initConnections() {
    connect(network_frame_, &NetworkFrame::requestNext,
            this, &FirstBootSetupWindow::onNetworkFinished);
    connect(hook_worker_, &FirstBootHookWorker::hookFinished,
            this, &FirstBootSetupWindow::onHookFinished);

    connect(monitor_mode_shortcut_, &GlobalShortcut::activated,
            multi_head_manager_, &MultiHeadManager::switchXRandRMode);
    connect(multi_head_manager_, &MultiHeadManager::primaryScreenChanged,
            this, &FirstBootSetupWindow::onPrimaryScreenChanged);
    connect(back_button_, &DImageButton::clicked, this, &FirstBootSetupWindow::backPage);
    connect(stacked_layout_, &QStackedLayout::currentChanged, back_button_, &DImageButton::raise);
}

void FirstBootSetupWindow::initUI() {
    back_button_ = new DImageButton(this);
    back_button_->setFixedSize(48, 38);
    back_button_->move(20, 20);
    back_button_->setFocusPolicy(Qt::TabFocus);
    back_button_->setNormalPic(":/images/back_normal.svg");
    back_button_->setHoverPic(":/images/back_hover.svg");
    back_button_->setPressPic(":/images/back_pressed.svg");
    back_button_->setDisabledPic(":/images/back_disabled.svg");
    back_button_->hide();

  background_label_ = new QLabel(this);
  language_frame_ = new LanguageFrame(this);
  system_info_frame_ = new SystemInfoFrame(this);
  network_frame_ = new NetworkFrame;
  timezone_frame_ = new TimezoneFrame(this);
  loading_frame_ = new FirstBootLoadingFrame;
  control_platform_frame_ = new ControlPlatformFrame(this);

  m_originalFrames = {
      language_frame_,
      system_info_frame_,
      timezone_frame_,
    //   control_platform_frame_,
    //   loading_frame_,
  };

  stacked_layout_ = new QStackedLayout;
  stacked_layout_->setContentsMargins(0, 0, 0, 0);
  stacked_layout_->setSpacing(0);

  for (FrameInterface* frame : m_originalFrames) {
      stacked_layout_->addWidget(frame);
      m_frames << frame;
  }

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setSpacing(0);
  mainLayout->setMargin(0);
  mainLayout->addLayout(stacked_layout_);

  DBackgroundGroup* bgGroup = new DBackgroundGroup;
  bgGroup->setContentsMargins(10, 10, 10, 10);
  bgGroup->setLayout(mainLayout);

  setCentralWidget(bgGroup);
}

void FirstBootSetupWindow::registerShortcut() {
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

  multi_head_manager_ = new MultiHeadManager(this);
}

void FirstBootSetupWindow::onHookFinished(bool ok) {
  if (!ok) {
    qCritical() << "First boot hook failed!";
  }

  if (QFile::exists("/usr/sbin/lightdm")) {
      qDebug() << SpawnCmd("systemctl", QStringList() << "restart" << "lightdm");
  }
  else {
    if (!changeToTTY(2)) {
      if (!RebootSystemWithMagicKey()) {
          RebootSystem();
      }
      qDebug() << SpawnCmd("killall", QStringList() << "lightdm");
    }
  }
}

void FirstBootSetupWindow::onPrimaryScreenChanged(const QRect& geometry) {
  qDebug() << "on primary screen changed" << geometry;
  this->move(geometry.topLeft());
  this->setFixedSize(geometry.size() / devicePixelRatioF());
}

void FirstBootSetupWindow::onLanguageSelected()
{
    if (GetSettingsBool(kSkipTimezonePage)) {
        return onTimezoneFinished();
    }
    else {
      stacked_layout_->setCurrentWidget(timezone_frame_);
      updateBackButtonVisible(stacked_layout_->currentWidget());
    }
}

void FirstBootSetupWindow::onSystemInfoFinished() {
  if (GetSettingsBool(kSkipNetworkPage)) {
    this->onNetworkFinished();
  } else {
    stacked_layout_->setCurrentWidget(network_frame_);
    updateBackButtonVisible(stacked_layout_->currentWidget());
  }
}

void FirstBootSetupWindow::onNetworkFinished()
{
    if (GetSettingsBool(kSkipControlPlatformPage)) {
        return onControlPlatformFinished();
    }
    else {
        stacked_layout_->setCurrentWidget(control_platform_frame_);
        updateBackButtonVisible(stacked_layout_->currentWidget());
    }
}

void FirstBootSetupWindow::onControlPlatformFinished()
{
    // Display loading frame.
    stacked_layout_->setCurrentWidget(loading_frame_);
    back_button_->hide();
    emit hook_worker_->startHook();
}

void FirstBootSetupWindow::onTimezoneFinished() {
    if (!GetSettingsBool(kSkipSystemInfoPage)) {
        stacked_layout_->setCurrentWidget(system_info_frame_);
    }
    else {
        onSystemInfoFinished();
    }
}

void FirstBootSetupWindow::backPage()
{
    // TODO: this function will be delete later
}

void FirstBootSetupWindow::updateBackButtonVisible(QWidget* page)
{
    // TODO: this function will be delete later
}

bool FirstBootSetupWindow::changeToTTY(int ttyNum) const
{
    auto is_a_console = [=](int fd) {
        char arg { 0 };
        return (isatty(fd) && ioctl(fd, KDGKBTYPE, &arg) == 0 && ((arg == KB_101) || (arg == KB_84)));
    };

    auto open_a_console = [=](const char *fnam) -> int {
        int fd;
        fd = open(fnam, O_RDWR);
        if (fd < 0)
            fd = open(fnam, O_WRONLY);
        if (fd < 0)
            fd = open(fnam, O_RDONLY);
        if (fd < 0)
            return -1;
        return fd;
    };

    auto getfd = [=](const char *fnam) -> int {
        int fd;

        if (fnam) {
            if ((fd = open_a_console(fnam)) >= 0) {
                if (is_a_console(fd))
                    return fd;
            }
            qWarning() << stderr << "Couldn't open %s" << fnam;
        }

        QStringList conspath {
            "/proc/self/fd/0",
            "/dev/tty",
            "/dev/tty0",
            "/dev/vc/0",
            "/dev/systty",
            "/dev/console",
        };

        for (const QString& path : conspath) {
            if ((fd = open_a_console(path.toUtf8().data())) >= 0) {
                if (is_a_console(fd)) {
                    return fd;
                }
            }
        }

        for (fd = 0; fd < 3; fd++)
            if (is_a_console(fd))
                return fd;

        qWarning() << stderr << "Couldn't get a file descriptor referring to the console";

        return fd;
    };

    int fd;
    if ((fd = getfd(nullptr)) < 0) {
        qWarning() << "Couldn't get a file descriptor referring to the console";
    }

    if (ioctl(fd, VT_ACTIVATE, ttyNum)) {
        qWarning() << "ioctl VT_ACTIVATE" << ttyNum;
        return false;
    }

    if (ioctl(fd, VT_WAITACTIVE, ttyNum)) {
        qWarning() << "ioctl VT_WAITACTIVE" << ttyNum;
        return false;
    }

    return true;
}

void FirstBootSetupWindow::previousFrameSelected(FrameInterface *frame)
{
    FrameInterface* currentFrame = qobject_cast<FrameInterface*>(stacked_layout_->currentWidget());
    Q_ASSERT(currentFrame != nullptr);

    if (m_showPastFrame){
        // TODO: update frame label state.
    }
    else {
        // TODO: update frame label state.
    }

    // TODO: update frame label state.
    stacked_layout_->setCurrentWidget(frame);

    m_showPastFrame = true;
}

}  // namespace installer
