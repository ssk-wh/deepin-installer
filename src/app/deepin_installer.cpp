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

// Main program of installer.

#include <DApplication>
#include <QDebug>
#include <QIcon>
#include <DLog>
#include <DSysInfo>

#include "ui/delegates/componentinstallmanager.h"
#include "base/consts.h"
#include "service/log_manager.h"
#include "service/settings_name.h"
#include "service/settings_manager.h"
#include "sysinfo/users.h"
#include "ui/delegates/installer_args_parser.h"
#include "ui/main_window.h"
#include "base/auto_screen_scale.h"
#include "ui/utils/keyboardmonitor.h"
#include "base/command.h"

DCORE_USE_NAMESPACE

int main(int argc, char* argv[]) {
#ifdef QT_DEBUG
    // add system version log
    qInfo() << "DSysInfo::productType() = " << DSysInfo::productType();
    qInfo() << "system version: " << QSysInfo().prettyProductName();
    // add kernel versionlog
    qInfo() << "kernel version: " << QSysInfo().kernelVersion();
#endif // QT_DEBUG

  // Reset LC_ALL to en_US.UTF-8.
  // NOTE(xushaohua): "LANG" might not set in some live environment.
  qputenv("LC_ALL", installer::kDefaultLang);
  qputenv("LANG", installer::kDefaultLang);

  Utils::AutoScreenScale();

  DApplication::loadDXcbPlugin();
  DApplication app(argc, argv);
  app.setAttribute(Qt::AA_UseHighDpiPixmaps);
  app.setAttribute(Qt::AA_EnableHighDpiScaling);
  app.setApplicationDisplayName("Deepin Installer Reborn");
  app.setApplicationName(" ");
  app.setApplicationVersion(installer::kAppVersion);
  app.setOrganizationDomain(installer::kOrganizationDomain);
  app.setWindowIcon(QIcon(":/images/deepin_installer.svg"));

  QFont font(app.font());
  font.setFamily(installer::GetUIDefaultFont());
  app.setFont(font);

  installer::KeyboardMonitor::instance()->setNumlockStatus(installer::KeyboardMonitor::instance()->isNumlockOn());

  // Parse argument list.
  installer::InstallerArgsParser args_parser;
  // Returns false if failed to parse arguments.
  if (!args_parser.parse(app.arguments())) {
    return 1;
  }

  // Initialize log service.
  const char kLogFileName[] = "deepin-installer.log";
  QString log_file;
  if (!installer::HasRootPrivilege()) {
    qCritical() << "Root privilege is required!";
    log_file = QString("/tmp/%1").arg(kLogFileName);
  } else {
    log_file = QString("/var/log/%1").arg(kLogFileName);
  }
  installer::RedirectLog(log_file);

  DLogManager::registerConsoleAppender();

  // Delete old settings file and generate a new one.
  installer::DeleteConfigFile();
  installer::AddConfigFile();
  installer::BeforeInstallHook();

  qDebug() << "Version:" << installer::kAppVersion;
  // add system version log
  qInfo() << "DSysInfo::productType() = " << DSysInfo::productType();
  qInfo() << "system version: " << QSysInfo().prettyProductName();
  // add kernel versionlog
  qInfo() << "kernel version: " << QSysInfo().kernelVersion();

  const QString conf_file(args_parser.getConfFile());
  // Append customized configurations.
  if (!conf_file.isEmpty()) {
    if (!installer::AppendConfigFile(conf_file)) {
      qCritical() << "Failed to append conf file:" << conf_file;
    }
  }

  installer::ComponentInstallManager::Instance();
  installer::MainWindow main_window;
  main_window.setLogFile(args_parser.getLogFile());

  main_window.setWindowIcon(":/images/deepin-installer-32px.svg");

  // Notify background thread to scan device info.
  main_window.scanDevicesAndTimezone();
  main_window.fullscreen();

  main_window.show();

  return app.exec();
}
