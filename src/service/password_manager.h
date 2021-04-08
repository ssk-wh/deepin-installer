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

#ifndef DEEPIN_INSTALLER_PASSWORD_MANAGER_H
#define DEEPIN_INSTALLER_PASSWORD_MANAGER_H

#include <QString>
#include <deepin_pw_check.h>

namespace installer {

enum PasswdLevel {
    LowerLevel = 1,
    MediumLevel,
    HigherLevel
};

class PasswordManager {
public:
    static PasswordManager* instance();

    /**
     * @brief checked
     * @param user：   传入检测的用户名参数
     * @param passwd： 传入检测的密码参数
     * @param info  ： 传出参数，用于返回检测密码的具体错误
     * @return      ： 返回false表示密码检测失败，否则返回true
     */
    bool checked(const QString &user, const QString &passwd, QString &info);

    PasswdLevel passwdLevel(const QString &passwd);

private:
    PasswordManager();
    PasswordManager(const PasswordManager&) = delete;

    void init();
    void setup();
    int passwdStrength(const QString &passwd);

private:
    QString m_strongPasswordEnable;        // 开启强密码策略配置
    QString m_passwdValidatePolicy;        // 密码策略配置
    QString m_passwdValidateRequired;      // 密码强度配置
    QString m_passwdMaxLength;             // 密码最大长度配置
    QString m_passwdMinLength;             // 密码最小长度配置
    QString m_passwdPalindrome;            // 密码回文长度配置
    QString m_passwdContinuousLength;      // 密码连续字符长度配置
    QString m_passwdMonotonousLength;      // 密码单调字符长度配置
};

}  // namespace installer

#endif  // DEEPIN_INSTALLER_PASSWORD_MANAGER_H
