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

#ifndef DEEPIN_INSTALLER_PWQUALITY_MANAGER_H
#define DEEPIN_INSTALLER_PWQUALITY_MANAGER_H

#include <cstddef>
#include <pwquality.h>
#include <QString>
#include <QScopedPointer>

namespace installer {

enum class ValidateState {
    Ok,
    EmptyError,  // Password is empty.
    TooShortError,  // Too few characters in password.
    TooLongError,  // Too many characters in password.
    NoNumberError,  // No number in password.
    NoLowerCharError,  // No lower case characters in password.
    NoUpperCharError,  // No upper case characters in password.
    NoSpecialCharError,  // No special characters in password.
    StrongError, // No strong check in password
    PalindromeError, //
    DictError
};

class PwqualityManager {
public:
    static PwqualityManager* instance();

    ValidateState checked(const QString &text);
    QString palindromeChecked(const QString &text);
    QString dictChecked(const QString &text);
    bool lengthChecked(const QString &text);

private:
    PwqualityManager();
    PwqualityManager(const PwqualityManager&) = delete;

    void init();
    void setup();

private:
    struct ScopedPointerCustomDeleter{
        static inline void cleanup(pwquality_settings_t *pointer){
            pwquality_free_settings(pointer);
        }
    };

    QScopedPointer<pwquality_settings_t, ScopedPointerCustomDeleter> m_pwqualitySetting;

    bool m_dictChecked = false;
    QString m_dictPath = "";
    bool m_palindromeChecked = false;
    int  m_palindromeLength = 1;
};

}  // namespace installer

#endif  // DEEPIN_INSTALLER_PWQUALITY_MANAGER_H
