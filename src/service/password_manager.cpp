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
                  };

        /* 更新密码配置 */
        file.write(list.join("\n").toUtf8() + "\n");

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
    switch (type) {
        case PW_NO_ERR:break;

        case PW_ERR_PASSWORD_EMPTY: {
            info = ::QObject::tr("The password cannot be empty​");
            reset = false;
        } break;

        case PW_ERR_LENGTH_SHORT:
        case PW_ERR_LENGTH_LONG: {
            info = ::QObject::tr("Password must be between %1 and %2 characters")
                    .arg(m_passwdMinLength)
                    .arg(m_passwdMaxLength);
            reset = false;
        } break;

        case PW_ERR_CHARACTER_INVALID:{
            info = ::QObject::tr("The password should contain at least %1 of the four available character types: lowercase letters, uppercase letters, numbers, and symbols (%2)").arg(m_passwdValidateRequired, QString(m_passwdValidatePolicy).replace(QRegExp("[0-9a-zA-Z;]"), ""));
            reset = false;
        } break;

        case PW_ERR_PALINDROME: {
            info = err_to_string(type);
            reset = false;
        } break;

        case PW_ERR_WORD: {
            info = err_to_string(type);
            reset = false;
        } break;

        case PW_ERR_PW_REPEAT: {
            info = err_to_string(type);     // 密码重复需求：安装器没有需求文案
            reset = false;
        } break;

        case PW_ERR_PW_MONOTONE: {
            info = err_to_string(type);     // 密码单调：安装器没有需求文案
            reset = false;
        } break;

        case PW_ERR_PW_CONSECUTIVE_SAME: {
            info = err_to_string(type);     // 密码连续重复字符需求：安装器没有需求文案
            reset = false;
        } break;

        case PW_ERR_PW_FIRST_UPPERM: {
            info = err_to_string(type);     // 安装器没有需求文案
            reset = false;
        } break;

        case PW_ERR_PARA: {
            info = err_to_string(type);     // 安装器没有需求文案
            reset = false;
        } break;

        case PW_ERR_INTERNAL: {
            info = err_to_string(type);     // 安装器没有需求文案
            reset = false;
        } break;

        case PW_ERR_USER: {
            info = err_to_string(type);     // 安装器没有需求文案
            reset = false;
        } break;

        case PW_ERR_MAX: {
            info = err_to_string(type);     // 安装器没有需求文案
            reset = false;
        } break;

        default: {
            info = err_to_string(type);     // 安装器没有需求文案
            reset = false;
        } break;
    }

    return reset;
}

installer::PasswdLevel installer::PasswordManager::passwdLevel(const QString &passwd)
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
