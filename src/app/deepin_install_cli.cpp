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


#include <QDebug>

#include "ui/delegates/componentinstallmanager.h"
#include "base/consts.h"
#include "service/log_manager.h"
#include "service/settings_manager.h"
#include "sysinfo/users.h"
#include "ui/delegates/installer_args_parser.h"
#include "ui/main_window_cli.h"
#include <QCoreApplication>

using namespace installer;

int main(int argc, char* argv[])
{
    //signal(SIGINT, SIG_IGN);
    // Reset LC_ALL to en_US.UTF-8.
    // NOTE(xushaohua): "LANG" might not set in some live environment.
    qputenv("LC_ALL", installer::kDefaultLang);
    qputenv("LANG", installer::kDefaultLang);


    QCoreApplication app(argc, argv);

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
    installer::RedirectLogWithOutStd(log_file);

    // Delete old settings file and generate a new one.
    installer::DeleteConfigFile();
    installer::AddConfigFile();

    qDebug() << "Version:" << installer::kAppVersion;

    const QString conf_file(args_parser.getConfFile());
    // Append customized configurations.
    if (!conf_file.isEmpty()) {
      if (!installer::AppendConfigFile(conf_file)) {
        qCritical() << "Failed to append conf file:" << conf_file;
      }
    }

    installer::ComponentInstallManager::Instance();

    auto currThreadId = QThread::currentThreadId();

    installer::MainWindow main_window;
    main_window.setLogFile(args_parser.getLogFile());
    //QThread thread;
    //QObject::connect(&thread, &QThread::started, &main_window, &MainWindow::start);
    // Notify background thread to scan device info.
    main_window.scanDevicesAndTimezone();
    //main_window.moveToThread(&thread);
    //thread.start();
    main_window.show();
    main_window.showChild(0);
    return app.exec();
}
