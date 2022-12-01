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

#include "password_manager.h"

#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "base/command.h"

#include <QDebug>
#include <QRegExp>
#include <QProcess>

// pw-check库1052中新加了枚举变量，为了保持低版本兼容，使用定义的枚举值对应的int值
#define PW_CHARACTER_TYPE_TOO_FEW 14
#define PW_USER_PASSWD_DUPLICATE 15

installer::PasswordManager::PasswordManager()
{
    init();
    setup();
}

void installer::PasswordManager::init()
{
    m_strongPasswordEnable = GetSettingsString(kSystemInfoPasswordStrongCheck);
    m_passwdValidatePolicy = GetSettingsString(kSystemInfoPasswordValidate);
    m_passwdValidateRequired = GetSettingsString(kSystemInfoPasswordValidateRequired);
    m_passwdMaxLength = GetSettingsString(kSystemInfoPasswordMaxLen);
    m_passwdMinLength = GetSettingsString(kSystemInfoPasswordMinLen);
    m_passwdPalindrome = GetSettingsString(kSystemInfoPasswordPalindromeLength);
    m_passwdContinuousLength = GetSettingsString(kSystemInfoPasswordContinuousLength);
    m_passwdMonotonousLength = GetSettingsString(kSystemInfoPasswordMonotonousLength);
}

void installer::PasswordManager::setup()
{
    QString info = "/etc/deepin/dde.conf";
    QString cmd = "/usr/bin/pwd-conf-update";

    QFile file(info);
    file.open(QIODevice::WriteOnly);
    if (file.isOpen()) {
        QStringList list= {"[Password]",
                       QString("STRONG_PASSWORD=%1").arg(m_strongPasswordEnable),
                       QString("PASSWORD_MIN_LENGTH=%1").arg(m_passwdMinLength),
                       QString("PASSWORD_MAX_LENGTH=%1").arg(m_passwdMaxLength),
                       QString("VALIDATE_POLICY=\"%1\"").arg(m_passwdValidatePolicy),
                       QString("VALIDATE_REQUIRED=%1").arg(m_passwdValidateRequired),
                       QString("PALINDROME_NUM=%1").arg(m_passwdPalindrome),
                       QString("CONSECUTIVE_SAME_CHARACTER_NUM=%1").arg(m_passwdContinuousLength),
                       QString("MONOTONE_CHARACTER_NUM=%1").arg(m_passwdMonotonousLength),
                  };

        /* 更新密码配置 */
        file.write(list.join("\n").toUtf8() + "\n");
        file.close();

        QString out;
        installer::SpawnCmd(cmd, QStringList(), out);
        qDebug() << "SpawnCmd " << cmd << ": " << out;

    } else {
        qWarning() << "failed file open. " << info;
    }
}

installer::PasswordManager *installer::PasswordManager::instance()
{
    static PasswordManager pwquality;
    return &pwquality;
}

bool installer::PasswordManager::checked(const QString &user, const QString &passwd, QString &info)
{
    bool reset = true;

    PW_ERROR_TYPE type = deepin_pw_check(user.toStdString().c_str(), passwd.toStdString().c_str(), 0, nullptr);
    qDebug() << "password checked: " << err_to_string(type);

    switch (int(type)) {
        case PW_NO_ERR:break;

        case PW_ERR_PASSWORD_EMPTY: {
            info = ::QObject::tr("The password cannot be empty");
            reset = false;
        } break;

        case PW_ERR_LENGTH_SHORT:
        case PW_ERR_LENGTH_LONG: {
            info = ::QObject::tr("Password must be between %1 and %2 characters")
                    .arg(m_passwdMinLength)
                    .arg(m_passwdMaxLength);
            reset = false;
        } break;

        case PW_CHARACTER_TYPE_TOO_FEW:
        case PW_ERR_CHARACTER_INVALID:{
            info = ::QObject::tr("The password should contain at least %1 of the four available character types: lowercase letters, uppercase letters, numbers, and symbols (%2)").arg(m_passwdValidateRequired, QString(m_passwdValidatePolicy).replace(QRegExp("[0-9a-zA-Z;]"), ""));
            reset = false;
        } break;

        case PW_ERR_PALINDROME: {
            info = ::QObject::tr("Password must not contain more than %1 palindrome characters").arg(m_passwdPalindrome);
            reset = false;
        } break;

        case PW_ERR_WORD: {
            info = ::QObject::tr("Do not use common words and combinations as password");
            reset = false;
        } break;

        case PW_ERR_PW_MONOTONE:
        case PW_ERR_PW_CONSECUTIVE_SAME: {
            info = ::QObject::tr("No more than %1 consecutive or repeated characters please").arg(m_passwdContinuousLength);
            reset = false;
        } break;
        case PW_USER_PASSWD_DUPLICATE: {
            info = ::QObject::tr("Different from the username");
            reset = false;
        } break;
        case PW_ERR_PW_REPEAT:
        case PW_ERR_PW_FIRST_UPPERM:
        case PW_ERR_PARA:
        case PW_ERR_INTERNAL:
        case PW_ERR_USER:
//        case PW_ERR_MAX:
        default: {
            info = ::QObject::tr("It does not meet password rules");
            reset = false;
        } break;
    }

    return reset;
}

installer::PasswdLevel installer::PasswordManager::passwdLevel(const QString &user, const QString &passwd)
{
    PasswdLevel level = LowerLevel;

    // 依次获取密码等级中的高和中，剩余情况为低
    if (passwdStrength(passwd) >= 3 && passwd.size() >= 8 && user != passwd) {
        level = HigherLevel;

    }  else if (passwdStrength(passwd) >= 2 && passwd.size() >= 6) {
        level = MediumLevel;
    }

    qDebug() << "passwdLevel = " << level;

    return level;
}

int installer::PasswordManager::passwdStrength(const QString &passwd)
{
    QStringList validatePolicyList = m_passwdValidatePolicy.split(";");

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

