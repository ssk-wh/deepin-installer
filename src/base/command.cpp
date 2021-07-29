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

#include "base/command.h"

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QSettings>

namespace installer {

bool RunScriptFile(const QStringList& args) {
  Q_ASSERT(!args.isEmpty());
  if (args.isEmpty()) {
    qCritical() << "RunScriptFile() args is empty!";
    return false;
  }

  // Change working directory.
  const QString current_dir(QFileInfo(args.at(0)).absolutePath());
  if (!QDir::setCurrent(current_dir)) {
    qCritical() << "Failed to change working directory:" << current_dir;
    return false;
  }

  // NOTE(xushaohua): If args[0] is not a script file, bash may raise
  // error message.
  return SpawnCmd("/bin/bash", args);
}

// mode 参数的默认值是为了保持历史逻辑不变
bool RunScriptFile(const QStringList& args, QString& output, QString& err, QProcess::ProcessChannelMode mode, int timeout) {
    Q_ASSERT(!args.isEmpty());
    if (args.isEmpty()) {
        qCritical() << "RunScriptFile() arg is empty!";
        return false;
    }

    // Change working directory.
    const QString current_dir(QFileInfo(args.at(0)).absolutePath());
    if (!QDir::setCurrent(current_dir)) {
        qCritical() << "Failed to change working directory:" << current_dir;
        return false;
    }

    // TODO(xushaohua): Remove bash
    return SpawnCmd("/bin/bash", args, output, err, mode, timeout);
}

bool SpawnCmd(const QString& cmd, const QStringList& args) {
  QString err, out;
  // Merge stdout and stderr of subprocess with main process.
#ifdef QT_DEBUG// Absolute path to installer config file.
  QSettings settings("/tmp/deepin-installer.conf", QSettings::IniFormat);
#else
  QSettings settings("/etc/deepin-installer.conf", QSettings::IniFormat);
#endif // QT_DEBUG

  QProcess::ProcessChannelMode mode = QProcess::ForwardedChannels;
  if (settings.contains("DI_NECURESCLIINSTALL_MODE")
          && settings.value("DI_NECURESCLIINSTALL_MODE").toBool()) {
    mode = QProcess::MergedChannels;
  }

  return SpawnCmd(cmd, args, err, out, mode);
}

bool SpawnCmd(const QString& cmd, const QStringList& args, QString& output) {
  QString err;
  return SpawnCmd(cmd, args, output, err);
}

bool SpawnCmd(const QString& cmd, const QStringList& args,
              QString& output, QString& err, QProcess::ProcessChannelMode mode,
              int timeout) {
    QProcess process;
    process.setProgram(cmd);
    process.setArguments(args);
    process.setProcessChannelMode(mode);

    process.start();
    // Wait for process to finish without timeout.
    if (!process.waitForFinished(timeout)) {
        return false;
    }

    output += process.readAllStandardOutput();
    err += process.readAllStandardError();

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        return true;
    }

    return false;
}

bool SpawnCmd(const QString& cmd, const QStringList& args,
              QString& output, QString& err, int tryCount) {
    int loop_num = 0;
    while (loop_num++ < tryCount) {
        bool result = SpawnCmd(cmd, args, output, err);
        if (result) {
            return true;
        }

        qDebug() << err;
        qDebug() << "try again!!!!!";
        QThread::sleep(1);
    }

    return false;
}

void xrandr() {

    QString msg;
    QString file = XRANDR_DIR"/deepin-installer-xrandr";
    bool exitCode = installer::SpawnCmd(file, QStringList(), msg);
    if (exitCode) {
        if (!msg.isEmpty()) qCritical() << QString("xrandr exec failed. file:<%1> err: <%2>").arg(file, msg);
    } else {
        qCritical() << QString("xrandr start failed. file: <%1>").arg(file);
    }
}

}  // namespace installer
