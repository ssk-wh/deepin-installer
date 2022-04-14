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

#include "service/first_boot_hook_worker.h"
#include "base/command.h"
#include "service/settings_manager.h"
#include "service/log_manager.h"
#include "service/settings_name.h"

#include <QDebug>

namespace installer {

namespace {

// Absolute path to built-in first boot script.
const char kFirstBootHookFile[] = "/tmp/installer/hooks/first_boot_setup.sh";

}  // namespace

FirstBootHookWorker::FirstBootHookWorker(QObject* parent) : QObject(parent) {
  this->setObjectName("first_boot_hook_worker");

  connect(this, &FirstBootHookWorker::startHook, this, [=] {
      updateComponentUninstallPackages();
      doStartHook();
  });
}

void FirstBootHookWorker::doStartHook() {
    QString out, err;
    // QProcess::ForwardedOutputChannel 将正常日志输出到合并到程序的标准输出中
    int timeout = GetSettingsInt(kSystemFirstBootTimeout);
    const bool ok = RunScriptFile({kFirstBootHookFile}, out, err, QProcess::ForwardedChannels, timeout);
    emit this->hookFinished(ok);
}

void FirstBootHookWorker::updateComponentUninstallPackages()
{
    QString dpkgResult;
    qDebug() << SpawnCmd("dpkg", { "-l" }, dpkgResult);

    QTextStream stream(&dpkgResult);
    QString line;
    QStringList installedList;
    while (stream.readLineInto(&line)) {
        const QStringList list {
            line.simplified().split(" ")
        };

        if (list.startsWith("ii")) {
            installedList << QString(list.at(1)).split(":").first();
        }
    }

    WriteComponentUninstallPackages(QSet<QString>(ReadComponentUninstallPackages().toSet()
                                                  & installedList.toSet()).toList().join(" "));
}

}  // namespace installer
