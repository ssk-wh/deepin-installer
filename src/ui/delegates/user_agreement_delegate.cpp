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

#include "user_agreement_delegate.h"
#include "service/settings_manager.h"
#include <QFileInfoList>
#include <QDir>
#include <QRegularExpression>

namespace installer {
const char kOemLicenseFileNamePattern[] = "^(.*)_([a-z]{2}_[A-Z]{2})\\.txt$";
const char kOemLicenseDir[] = "/oem_license/license";

static QString GetMaybeOemLicenseDir()
{
    QString dirs[] = {
        GetOemDir().absoluteFilePath("license"),
        QString(kOemLicenseDir)
    };

    for (auto dir : dirs) {
        QFileInfo  fileInfo(dir);
        if(fileInfo.isDir()){
           return dir;
        }
    }
    return dirs[0];
}

LicenseItem::LicenseItem()
    : basicName_(QString(""))
    , localeName_(QString(""))
    , fileName_(QString(""))
{

}

LicenseItem::LicenseItem(const QString &basicName, const QString &localeName, const QString &fileName)
    : basicName_(basicName)
    , localeName_(localeName)
    , fileName_(fileName)
{

}

const QString &LicenseItem::basicName() const
{
    return basicName_;
}

const QString &LicenseItem::localeName() const
{
    return localeName_;
}

const QString &LicenseItem::fileName() const
{
    return fileName_;
}

bool LicenseItem::isValid() const
{
    return !fileName_.isEmpty() && !basicName_.isEmpty() && !localeName_.isEmpty();
}

UserAgreementDelegate::UserAgreementDelegate()
{
    license_dir_ = GetMaybeOemLicenseDir();
    scanLicenses();
}

bool UserAgreementDelegate::isLicenseDirExists() const
{
    QFileInfo fileInfo(license_dir_);
    return fileInfo.isDir();
}

void UserAgreementDelegate::setLicenseDir(const QString &licenseDir)
{
    license_dir_ = licenseDir;
}

const QString &UserAgreementDelegate::licenseDirPath() const
{
    return license_dir_;
}

void UserAgreementDelegate::scanLicenses()
{
    license_map_.clear();

    if (license_dir_.isEmpty()) {
        return;
    }

    QRegularExpression re(kOemLicenseFileNamePattern);
    QFileInfoList fileInfoList = QDir(license_dir_).entryInfoList();
    for (QFileInfo fileInfo : fileInfoList) {
        if (!fileInfo.isFile()) {
            continue;
        }
        QRegularExpressionMatch match = re.match(fileInfo.fileName());
        if (match.isValid()) {
            LicenseItem license_item(match.captured(1), match.captured(2), fileInfo.absoluteFilePath());
            license_map_[license_item.localeName()] = license_item;
        }
    }
}

int UserAgreementDelegate::licenseCount() const
{
    return license_map_.count();
}

const LicenseItem UserAgreementDelegate::getLicenseByLocale(const QString &localeName) const
{
    auto it = license_map_.find(localeName);
    if (it != license_map_.constEnd()) {
        return it.value();
    }
    return LicenseItem();
}

const LicenseItem UserAgreementDelegate::getPrimaryAdaptiveLicense(const QString &localeName) const
{
    LicenseItem license_item;
    license_item = getLicenseByLocale(localeName);
    if (!license_item.isValid() && localeName != "en_US") {
        license_item = getLicenseByLocale("en_US");
    }
    return license_item;
}

}
