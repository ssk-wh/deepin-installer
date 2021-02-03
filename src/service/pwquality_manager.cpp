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
//    m_pwqualitySetting.reset(pwquality_default_settings());
}

void installer::PwqualityManager::setup()
{
//    m_dictChecked = GetSettingsBool(kSystemInfoPasswordDistCheck);
//    if (m_dictChecked) {
//        pwquality_set_int_value(m_pwqualitySetting.get(), PWQ_SETTING_DICT_CHECK, 1);
//    } else {
//        pwquality_set_int_value(m_pwqualitySetting.get(), PWQ_SETTING_DICT_CHECK, 0);
//    }
//    m_dictPath = GetSettingsBool(kSystemInfoPasswordDistPath);
//    if (!m_dictPath.isEmpty()) {
//        pwquality_set_str_value(m_pwqualitySetting.get(), PWQ_SETTING_DICT_PATH, m_dictPath.toStdString().c_str());
//    }

//    m_palindromeChecked = GetSettingsBool(kSystemInfoPasswordPalindromeCheck);
//    m_palindromeLength = GetSettingsInt(kSystemInfoPasswordPalindromeLength);
    m_monotonousLength = GetSettingsInt(kSystemInfoPasswordMonotonousLength);
    m_continuousLength = GetSettingsInt(kSystemInfoPasswordContinuousLength);
}

installer::PwqualityManager *installer::PwqualityManager::instance()
{
    static PwqualityManager pwquality;
    return &pwquality;
}

installer::ValidateState installer::PwqualityManager::checked(const QString &text)
{
    Q_UNUSED(text);
    return ValidateState::Ok;
}

QString installer::PwqualityManager::palindromeChecked(const QString &text)
{
    Q_UNUSED(text);
//    if (m_palindromeChecked) {
//        QStringList list;
//        for (int pos = 0; pos < text.size() + 1 - m_palindromeLength; pos++) {
//            list.append(text.mid(pos, m_palindromeLength));
//        }

//        for (QString str : list) {
//            int code = pwquality_check(m_pwqualitySetting.get(),
//                                       str.toStdString().c_str(),
//                                       NULL, NULL, NULL);

//            if (code == PWQ_ERROR_PALINDROME) {
//                return str;
//            }
//        }
//    }

    return QString();
}


QString installer::PwqualityManager::dictChecked(const QString &text)
{
    Q_UNUSED(text);
//    int code = pwquality_check(m_pwqualitySetting.get(),
//                               text.toStdString().c_str(),
//                               NULL, NULL, NULL);

//    if (code == PWQ_ERROR_CRACKLIB_CHECK) {
//        return text;
//    }
    return QString();
}

bool installer::PwqualityManager::lengthChecked(const QString &text)
{
    Q_UNUSED(text);
//    pwquality_set_int_value(m_pwqualitySetting.get(), PWQ_SETTING_MIN_LENGTH, 6);
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

installer::PasswdLevel installer::PwqualityManager::passwdLevel(const QString &passwd)
{
    PasswdLevel level = LowerLevel;

    // 依次获取密码等级中的高和中，剩余情况为低
    if (passwdStrength(passwd) >= 3 && passwd.size() >= 8) {
        level = HigherLevel;

    }  else if (passwdStrength(passwd) >= 2 && passwd.size() >= 6) {
        level = MediumLevel;
    }

    qDebug() << "passwdLevel = " << level;

    return level;
}

int installer::PwqualityManager::passwdStrength(const QString &passwd)
{
    QStringList validatePolicyList = GetSettingsString(kSystemInfoPasswordValidate).split(";");
    qDebug() << "passwdStrength = " << validatePolicyList;
    int reset = int(std::count_if(validatePolicyList.cbegin(), validatePolicyList.cend(),
                      [=](const QString& policy) {
                          for (const QChar& c : policy) {
                              if (passwd.contains(c)) {
                                  return true;
                              }
                          }
                          return false;
                      }));

    return reset;
}

bool installer::PwqualityManager::passwdMonotonous(const QString &passwd)
{
    qDebug() << "m_monotonousLength = " << m_monotonousLength;
    // 过滤长度为0的情况
    if (m_monotonousLength == 0) {
        return false;
    }

    // 按单调字符长度截取密码
    QStringList list;
    for (int pos = 0; pos < passwd.size() + 1 - m_monotonousLength; pos++) {
        list.append(passwd.mid(pos, m_monotonousLength));
    }

    // 依次处理每次截取的密码串判断是否是单调加1的字符串
    for (QString str : list) {
        /* 过滤掉尾部长度不符合要求的字符串 */
        if (str.length() < m_monotonousLength) {
            continue;
        }

        /* 单调加1字符串：统计 字符串中前后字符之间差为1的次数是 = 单调字符长度 - 1 */
        QChar ch = str[0];
        if (std::count_if(str.cbegin() + 1, str.cend(),
                          [&](const QChar& policy) {
                              if (policy.toLatin1() - ch.toLatin1() == 1) {
                                  ch = policy;
                                  return true;
                              }
                              return false;
            }) == (m_monotonousLength - 1)) {

            return true;
        }
    }

    return false;
}

bool installer::PwqualityManager::passwdContinuous(const QString &passwd)
{
    qDebug() << "m_continuousLength = " << m_continuousLength;
    // 过滤长度为0的情况
    if (m_continuousLength == 0) {
        return false;
    }

    // 按单调字符长度截取密码
    QStringList list;
    for (int pos = 0; pos < passwd.size() + 1 - m_continuousLength; pos++) {
        list.append(passwd.mid(pos, m_continuousLength));
    }

    // 依次处理每次截取的密码串判断每个子串是否是相同字符
    for (QString str : list) {
        /* 过滤掉尾部长度不符合要求的字符串 */
        if (str.length() < m_continuousLength) {
            continue;
        }

        /* 统计字符串是否都相等 */
        QChar ch = str[0];
        if (std::count_if(str.cbegin(), str.cend(),
                          [&](const QChar& policy) {
                              if (policy == ch) {
                                  return true;
                              }
                              return false;
            }) == m_continuousLength) {

            return true;
        }
    }

    return false;
}

