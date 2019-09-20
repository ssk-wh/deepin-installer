/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
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

#ifndef USER_AGREEMENT_DELEGATE_H
#define USER_AGREEMENT_DELEGATE_H

#include <QList>
#include <QMap>

namespace installer {

class LicenseItem
{
public:
    explicit LicenseItem();
    explicit LicenseItem(const QString &basicName, const QString &localeName, const QString &fileName);

    const QString &basicName() const;
    const QString &localeName() const;
    const QString &fileName() const;

    bool isValid() const;

private:
    QString basicName_;
    QString localeName_;
    QString fileName_;
};

typedef QMap<QString, LicenseItem> QLicenseItemMap;

class UserAgreementDelegate
{
public:
    explicit UserAgreementDelegate();
    void setLicenseDir(const QString &licenseDir);
    const QString &licenseDirPath() const;
    bool isLicenseDirExists() const;
    void scanLicenses();
    int  licenseCount() const;
    const LicenseItem getLicenseByLocale(const QString &localeName) const;
    const LicenseItem getPrimaryAdaptiveLicense(const QString &localeName) const;

private:
    QString           license_dir_;
    QLicenseItemMap   license_map_;
};

}

#endif // USER_AGREEMENT_DELEGATE_H
