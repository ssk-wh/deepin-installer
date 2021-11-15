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

// Run this program to setup username, password, keyboard and timezone after
// reboot system. Only if `system_info_setup_after_reboot` flag is enabled.

#include <QDebug>
#include <QIcon>
#include <DLog>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <QScreen>

#include "service/multiscreenmanager.h"
#include "base/consts.h"
#include "service/log_manager.h"
#include "service/settings_name.h"
#include "service/settings_manager.h"
#include "sysinfo/users.h"
#include "ui/delegates/language_delegate.h"
#include "ui/first_boot_setup_window.h"
#include "ui/utils/keyboardmonitor.h"

#include <cstdlib>
#include <DApplication>
#include "base/auto_screen_scale.h"
#include "base/command.h"

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char* argv[]) {
    //for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")){
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    qputenv("LC_ALL", installer::kDefaultLang);
    qputenv("LANG", installer::kDefaultLang);

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    DApplication app(argc, argv);
    app.setApplicationDisplayName("Deepin Installer First Boot");
    app.setApplicationName(" ");
    app.setApplicationVersion(installer::kAppVersion);
    app.setOrganizationDomain(installer::kOrganizationDomain);
    app.setWindowIcon(QIcon(":/images/deepin_installer.svg"));

    const char kLogFileName[] = "deepin-installer-first-boot.log";
    QString log_file;
    if (!installer::HasRootPrivilege()) {
        qCritical() << "Root privilege is required!";
        log_file = QString("/tmp/%1").arg(kLogFileName);
    }
    else {
        log_file = QString("/var/log/%1").arg(kLogFileName);
    }
    // Initialize log service.
    #ifdef QT_DEBUG_test
    #else
    installer::RedirectLog(log_file);
    #endif // QT_DEBUG

    // 判断是否是烧录系统情况下的后配置流程
    if (!installer::GetSettingsString("system_installer_start_mode").isEmpty()) {
        installer::DeleteConfigFile();
        installer::AddConfigFile();
    }

    QFont font(app.font());
    font.setPixelSize(installer::GetSettingsInt(installer::kSystemDefaultFontSize));
    font.setFamily(installer::GetUIDefaultFont());
    app.setFont(font);

    installer::KeyboardMonitor::instance()->setNumlockStatus(installer::KeyboardMonitor::instance()->isNumlockOn());

    installer::FirstBootSetupWindow* main_window = new installer::FirstBootSetupWindow;
    app.installEventFilter(main_window);
    main_window->setWindowIcon(":/images/deepin-installer-64px.svg");
    // 安装器的主界面输出到主屏，由启动初始化阶段的脚本克隆到其他屏幕
    main_window->setScreen(app.primaryScreen());
    main_window->show();

    return app.exec();
}
