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

#include "pwquality_manager.h"

#include "service/settings_manager.h"
#include "service/settings_name.h"

#include <QDebug>
#include <QRegExp>

installer::PwqualityManager::PwqualityManager()
{
    init();
    setup();
}

void installer::PwqualityManager::init()
{
    m_pwqualitySetting.reset(pwquality_default_settings());
}

void installer::PwqualityManager::setup()
{
    m_dictChecked = GetSettingsBool(kSystemInfoPasswordDistCheck);
    if (m_dictChecked) {
        pwquality_set_int_value(m_pwqualitySetting.get(), PWQ_SETTING_DICT_CHECK, 1);
    } else {
        pwquality_set_int_value(m_pwqualitySetting.get(), PWQ_SETTING_DICT_CHECK, 0);
    }
    m_dictPath = GetSettingsBool(kSystemInfoPasswordDistPath);
    if (!m_dictPath.isEmpty()) {
        pwquality_set_str_value(m_pwqualitySetting.get(), PWQ_SETTING_DICT_PATH, m_dictPath.toStdString().c_str());
    }

    m_palindromeChecked = GetSettingsBool(kSystemInfoPasswordPalindromeCheck);
    m_palindromeLength = GetSettingsInt(kSystemInfoPasswordPalindromeLength);
}


installer::PwqualityManager *installer::PwqualityManager::instance()
{
    static PwqualityManager pwquality;
    return &pwquality;
}

installer::ValidateState installer::PwqualityManager::checked(const QString &text)
{
    return ValidateState::Ok;
}

QString installer::PwqualityManager::palindromeChecked(const QString &text)
{
    if (m_palindromeChecked) {
        QStringList list;
        for (int pos = 0; pos < text.size() + 1 - m_palindromeLength; pos++) {
            list.append(text.mid(pos, m_palindromeLength));
        }

        for (QString str : list) {
            int code = pwquality_check(m_pwqualitySetting.get(),
                                       str.toStdString().c_str(),
                                       NULL, NULL, NULL);

            if (code == PWQ_ERROR_PALINDROME) {
                return str;
            }
        }
    }

    return QString();
}


QString installer::PwqualityManager::dictChecked(const QString &text)
{
    int code = pwquality_check(m_pwqualitySetting.get(),
                               text.toStdString().c_str(),
                               NULL, NULL, NULL);

    if (code == PWQ_ERROR_CRACKLIB_CHECK) {
        return text;
    }
    return QString();
}

bool installer::PwqualityManager::lengthChecked(const QString &text)
{
    pwquality_set_int_value(m_pwqualitySetting.get(), PWQ_SETTING_MIN_LENGTH, 6);
    // ...

    return true;
}

bool installer::PwqualityManager::oem_lower_case(const QString &text)
{
    if (GetSettingsBool(kSystemInfoPasswordStrongCheck)
            && GetSettingsBool(kSystemInfoPasswordRequireLowerCase)) {

        return text.contains(QRegExp("[abcdefghijklmnopqrstuvwxyz]"));
    }

    return true;
}

bool installer::PwqualityManager::oem_require_number(const QString &text)
{
    if (GetSettingsBool(kSystemInfoPasswordStrongCheck)
            && GetSettingsBool(kSystemInfoPasswordRequireNumber)) {

        return text.contains(QRegExp("[1234567890]"));
    }

    return true;
}

bool installer::PwqualityManager::oem_upper_case(const QString &text)
{
    if (GetSettingsBool(kSystemInfoPasswordStrongCheck)
            && GetSettingsBool(kSystemInfoPasswordRequireUpperCase)) {

        return text.contains(QRegExp("[ABCDEFGHIJKLMNOPQRSTUVWXYZ]"));
    }

    return true;
}

bool installer::PwqualityManager::oem_special_char(const QString &text)
{
    qDebug() << "text = " << text;
    if (GetSettingsBool(kSystemInfoPasswordStrongCheck)
            && GetSettingsBool(kSystemInfoPasswordRequireSpecialChar)) {

        qDebug() << "text1 = " << text;
        const QString validatePolicy = "~!@#$%^&*()[]{}\\|/?,.<>";
        for (QChar ch : text) {
            if (validatePolicy.contains(ch)) {
                return true;
            }
        }

        return false;
    }

    return true;
}
