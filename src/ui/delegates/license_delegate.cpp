#include "license_delegate.h"

#include "service/settings_manager.h"

#include <DSysInfo>
#include <QDebug>

DCORE_USE_NAMESPACE

QString installer::LicenseDelegate::licenseTitle()
{
    switch (GetCurrentType()) {
        case OSType::Personal: return ::QObject::tr("End User License Agreement for UnionTech OS Desktop Home");
        case OSType::Professional: return QObject::tr("End User License Agreement for UnionTech OS Desktop Professional");
        case OSType::Community: return QObject::tr("End User License Agreement for Deepin OS");
        case OSType::Server: return QObject::tr("End User License Agreement for UnionTech OS Server");
        default: {
            qCritical() << "Invalid current OS type";
            return QObject::tr("End User License Agreement for Deepin OS");
        }
    }
}

QString installer::LicenseDelegate::userExperienceTitle()
{
    switch (GetCurrentType()) {
        case OSType::Community:return ::QObject::tr("Deepin OS User Experience Program License Agreement");
        case OSType::Professional:
        case OSType::Personal:
        case OSType::Server:
        default: return ::QObject::tr("UnionTech OS User Experience Program License Agreement");
    }
}

QString installer::LicenseDelegate::privacyLicenseTitle()
{
    switch (GetCurrentType()) {
        case OSType::Community: return ::QObject::tr("Deepin OS Privacy Policy");
        case OSType::Professional:
        case OSType::Personal:
        case OSType::Server:
        default: return ::QObject::tr("UnionTech Software Privacy Policy");
    }
}

QString installer::LicenseDelegate::logo()
{
    switch (GetCurrentType()) {
        case OSType::Community: return ":/images/dcc_deepin_logo_164px.svg";
        case OSType::Professional:
        case OSType::Personal:
        case OSType::Server:
        default: return ":/images/distribution_logo";
    }
}

QString installer::LicenseDelegate::product()
{
    switch (GetCurrentType()) {
        case OSType::Community: return ::QObject::tr("Deepin");
        case OSType::Professional: return ::QObject::tr("UnionTech OS");
        case OSType::Personal:
        case OSType::Server:
        default: return ::QObject::tr("UOS");
    }
}

QString installer::LicenseDelegate::OSType()
{
    switch (GetCurrentType()) {
        case OSType::Professional: return "professional";
        case OSType::Community: return "community";
        case OSType::Server: return "server";
        case OSType::Personal: return "personal";
        default: {
            qCritical() << "Invalid current OS type";
            return "community";
        }
    }
}

