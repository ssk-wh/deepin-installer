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
#include <linux/vt.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/kd.h>

#include "widgets/pointer_button.h"
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

namespace installer {

FirstBootSetupWindow::FirstBootSetupWindow(QWidget *parent)
    : QFrame(parent),
      hook_worker_thread_(new QThread(this)),
      hook_worker_(new FirstBootHookWorker()) {
  this->setObjectName("first_boot_setup_window");

  hook_worker_thread_->start();
  hook_worker_->moveToThread(hook_worker_thread_);

  this->initUI();
  this->registerShortcut();
  this->initConnections();

  // Read default settings.
  language_frame_->readConf();
  system_info_frame_->readConf();
  timezone_frame_->readConf();

    if (GetSettingsBool(kSkipSelectLanguagePageOnFirstBoot)) {
        language_frame_->writeConf();
        onLanguageSelected();
    }
}

FirstBootSetupWindow::~FirstBootSetupWindow() {
  QuitThread(hook_worker_thread_);
}

void FirstBootSetupWindow::fullscreen() {
  ShowFullscreen(this);
}

void FirstBootSetupWindow::resizeEvent(QResizeEvent *event) {
  this->updateBackground();
  QFrame::resizeEvent(event);
}

void FirstBootSetupWindow::initConnections() {
    connect(language_frame_, &LanguageFrame::finished, this,
            &FirstBootSetupWindow::onLanguageSelected);
    connect(system_info_frame_, &SystemInfoFrame::finished,
            this, &FirstBootSetupWindow::onSystemInfoFinished);
    connect(network_frame_, &NetworkFrame::requestNext,
            this, &FirstBootSetupWindow::onNetworkFinished);
    connect(control_platform_frame_, &ControlPlatformFrame::requestFinished, this,
            &FirstBootSetupWindow::onControlPlatformFinished);
    connect(timezone_frame_, &TimezoneFrame::finished,
            this, &FirstBootSetupWindow::onTimezoneFinished);
    connect(hook_worker_, &FirstBootHookWorker::hookFinished,
            this, &FirstBootSetupWindow::onHookFinished);

    connect(monitor_mode_shortcut_, &GlobalShortcut::activated,
            multi_head_manager_, &MultiHeadManager::switchXRandRMode);
    connect(multi_head_manager_, &MultiHeadManager::primaryScreenChanged,
            this, &FirstBootSetupWindow::onPrimaryScreenChanged);
    connect(back_button_, &PointerButton::clicked, this, &FirstBootSetupWindow::backPage);
}

void FirstBootSetupWindow::initUI() {
    back_button_ = new PointerButton;
    back_button_->setObjectName("back_button");
    back_button_->setFixedSize(48, 38);
    back_button_->setFlat(true);
    back_button_->setFocusPolicy(Qt::TabFocus);
    back_button_->setStyleSheet(ReadFile(":/styles/back_button.css"));
    back_button_->hide();

    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->setMargin(10);
    topLayout->setSpacing(0);
    topLayout->addWidget(back_button_, 0, Qt::AlignLeft);
    topLayout->addStretch();

  background_label_ = new QLabel(this);
  language_frame_ = new LanguageFrame;
  system_info_frame_ = new SystemInfoFrame;
  network_frame_ = new NetworkFrame;
  timezone_frame_ = new TimezoneFrame;
  loading_frame_ = new FirstBootLoadingFrame;
  control_platform_frame_ = new ControlPlatformFrame;

  m_frames = {
      language_frame_,
      timezone_frame_,
      system_info_frame_,
      network_frame_,
      control_platform_frame_,
      loading_frame_,
  };

  stacked_layout_ = new QStackedLayout;
  stacked_layout_->setContentsMargins(0, 0, 0, 0);
  stacked_layout_->setSpacing(0);

  for (QWidget* widget : m_frames) {
      stacked_layout_->addWidget(widget);
  }

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setSpacing(0);
  mainLayout->setMargin(0);
  mainLayout->addLayout(topLayout);
  mainLayout->addSpacing(36);
  mainLayout->addLayout(stacked_layout_);
  mainLayout->addSpacing(36);

  setLayout(mainLayout);
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

void FirstBootSetupWindow::updateBackground() {
  if (!background_label_) {
    qWarning() << "background_label is not initialized!";
    return;
  }
  const QString image_path = GetWindowBackground();
  // Other platforms may have performance issue.
  const QPixmap pixmap = QPixmap(image_path).scaled(
      size(), Qt::KeepAspectRatioByExpanding);
  background_label_->setPixmap(pixmap);
  background_label_->setFixedSize(size());
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
  this->setFixedSize(geometry.size());
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
    stacked_layout_->setCurrentWidget(system_info_frame_);
}

void FirstBootSetupWindow::backPage()
{
    stacked_layout_->setCurrentWidget(m_frames.at(m_frames.indexOf(stacked_layout_->currentWidget()) - 1));
    updateBackButtonVisible(stacked_layout_->currentWidget());
}

void FirstBootSetupWindow::updateBackButtonVisible(QWidget* page)
{
    back_button_->setVisible(static_cast<bool>(m_frames.indexOf(page)));
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
        int fd, i;

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

}  // namespace installer
