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

#include "base/consts.h"
#include "service/log_manager.h"
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

void dump(int signo)
{
        char buf[1024];
        char cmd[1024];
        FILE *fh;

        snprintf(buf, sizeof(buf), "/proc/%d/cmdline", getpid());
        if(!(fh = fopen(buf, "r")))
                exit(0);
        if(!fgets(buf, sizeof(buf), fh))
                exit(0);
        fclose(fh);
        if(buf[strlen(buf) - 1] == '\n')
                buf[strlen(buf) - 1] = '\0';
        snprintf(cmd, sizeof(cmd), "gdb %s %d", buf, getpid());
        system(cmd);

        exit(0);
}

int main(int argc, char* argv[]) {
#ifdef QT_DEBUG
  signal(SIGSEGV, &dump);
#endif // QT_DEBUG

  qputenv("LC_ALL", installer::kDefaultLang);
  qputenv("LANG", installer::kDefaultLang);

//  installer::xrandr();
  Utils::AutoScreenScale();

  DApplication::loadDXcbPlugin();
  DApplication app(argc, argv);
  app.setAttribute(Qt::AA_UseHighDpiPixmaps);
  app.setAttribute(Qt::AA_EnableHighDpiScaling);
  app.setApplicationDisplayName("Deepin Installer First Boot");
  app.setApplicationName(" ");
  app.setApplicationVersion(installer::kAppVersion);
  app.setOrganizationDomain(installer::kOrganizationDomain);
  app.setWindowIcon(QIcon(":/images/deepin_installer.svg"));

  const char kLogFileName[] = "deepin-installer-first-boot.log";
  QString log_file;
  if (!installer::HasRootPrivilege())
  {
    qCritical() << "Root privilege is required!";
    log_file = QString("/tmp/%1").arg(kLogFileName);
  }
  else
  {
    log_file = QString("/var/log/%1").arg(kLogFileName);
  }
  // Initialize log service.
#ifdef QT_DEBUG_test
#else
  installer::RedirectLog(log_file);
#endif // QT_DEBUG

  QFont font(app.font());
  font.setFamily(installer::GetUIDefaultFont());
  app.setFont(font);

  installer::KeyboardMonitor::instance()->setNumlockStatus(installer::KeyboardMonitor::instance()->isNumlockOn());

  installer::FirstBootSetupWindow window;
  window.fullscreen();
  window.setWindowIcon(":/images/deepin-installer-32px.svg");
  window.show();

  return app.exec();
}
