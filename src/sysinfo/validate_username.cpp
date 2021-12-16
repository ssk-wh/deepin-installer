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

#include "sysinfo/validate_username.h"

#include <QDebug>
#include <QRegExp>

#include "base/file_util.h"
#include "base/command.h"

namespace installer {

namespace {

//判断创建的用户名是否是系统上已有的用户名或者组名
bool IsReservedUsername(const QString& username) {
    QString reservedUsername = "";
    QString err ="";
    SpawnCmd("/bin/bash", (QStringList() << "-c" << "compgen -g -u"), reservedUsername, err, 2);
    if (!err.isEmpty()) {
      qWarning() << "run compgen -g -u is failed";
      return false;
    }
    if (reservedUsername.isEmpty()) {
      qWarning() << "Reserved username list is empty";
      return false;
    }
    const QStringList lines = reservedUsername.split('\n');
    for (const QString& line : lines) {
      if (line.isEmpty()) {
        continue;
      }
      if (line == username) {
        return true;
      }
    }
    return false;
}

}  // namespace

ValidateUsernameState ValidateUsername(const QString& username,
                                       int min_len,
                                       int max_len) {
  if (username.isEmpty() && (min_len > 0)) {
    return ValidateUsernameState::EmptyError;
  }
  if (username.length() < min_len) {
    return ValidateUsernameState::TooShortError;
  }
  if (username.length() > max_len) {
    return ValidateUsernameState::TooLongError;
  }
//创建的用户名是否符合命名规则，用户名以字母或数字开头，只允许使用字母、数字、连接符（-）和下划线（_），且不可以使用纯数字
  const QRegExp reg("[a-zA-Z0-9][a-zA-Z0-9_-]*");
  if (!reg.exactMatch(username)) {
    return ValidateUsernameState::InvalidCharError;
  }
//创建的用户名是否为纯数字
  if (QString(username).replace(QRegExp("[0-9]"),"").isEmpty()) {
      return ValidateUsernameState::Digital;
  }
//创建的用户名是否为Linux系统已有的用户名或组名
  if (IsReservedUsername(username)) {
      return ValidateUsernameState::ReservedError;
  }
  return ValidateUsernameState::Ok;
}

}  // namespace installer
