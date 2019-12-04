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

#include "sysinfo/validate_password.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"

#include <QSet>
#include <QStringList>
#include <QDebug>
namespace installer {
ValidatePasswordState ValidatePassword(const QString&     password,
                                       int                min_len,
                                       int                max_len,
                                       bool               strong_pwd_check,
                                       const QStringList& validatePolicy      = {},
                                       int                validateRequiredNum = 0)
{
    Q_ASSERT(min_len >= 0);
    Q_ASSERT(max_len > min_len);

    if (password.isEmpty() && min_len > 0) {
        return ValidatePasswordState::EmptyError;
    }
    if (password.length() < min_len) {
        return ValidatePasswordState::TooShortError;
    }
    if (password.length() > max_len) {
        return ValidatePasswordState::TooLongError;
    }

    if (strong_pwd_check) {
        if (!(password.split("").toSet() - validatePolicy.join("").split("").toSet())
                 .isEmpty()) {
            return ValidatePasswordState::StrongError;
        }

        if (std::count_if(validatePolicy.cbegin(), validatePolicy.cend(),
                          [=](const QString& policy) {
                              for (const QChar& c : policy) {
                                  if (password.contains(c)) {
                                      return true;
                                  }
                              }

                              return false;
                          }) < validateRequiredNum) {
            return ValidatePasswordState::StrongError;
        }
    }

    return ValidatePasswordState::Ok;
}

}  // namespace installer
