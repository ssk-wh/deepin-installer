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

#ifndef INSTALLER_SYSINFO_VALIDATE_USERNAME_H
#define INSTALLER_SYSINFO_VALIDATE_USERNAME_H

#include <QString>

namespace installer {

enum class ValidateUsernameState {
  Ok,
  EmptyError,  // Username is empty.
  FirstCharError,  // First character of username is not in a-z.
  InvalidCharError,  // Contains invalid word.
  ReservedError,  // Username already exists.
  TooLongError,
  TooShortError,
  Digital, // 字符串全是数字
};

/*
校验用户名1:用户名只能包含大小写字母、数字、连接符（-）和下划线（_）；
        2:用户名只能以字母或数字开头；
        3:用户名长度不能超过32个字符，不能小于3个字符；
        4:不能只包含数字；
        5:判断用户名是否是系统已存在的用户名或组名
*/
ValidateUsernameState ValidateUsername(const QString& username,
                                       int min_len,
                                       int max_len);

}  // namespace installer

#endif  // INSTALLER_SYSINFO_VALIDATE_USERNAME_H
