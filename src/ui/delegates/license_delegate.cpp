#include "license_delegate.h"

#include "service/settings_manager.h"

#include <DSysInfo>

DCORE_USE_NAMESPACE

QString installer::LicenseDelegate::licenseTitle()
{
    QString product_type = (DSysInfo::productType() == DSysInfo::Deepin) ?
                ::QObject::tr("Deepin") : ::QObject::tr("UOS");

    switch (GetCurrentType()) {
        case OSType::Personal: return ::QObject::tr("End User License Agreement for UnionTech OS Desktop Home");
        case OSType::Professional: return QObject::tr("End User License Agreement for UnionTech OS Desktop Professional");
        case OSType::Community:
        case OSType::Server:
        default: return QObject::tr("%1 Software End User License Agreement").arg(product_type);
    }
}

QString installer::LicenseDelegate::userExperienceTitle()
{
    switch (GetCurrentType()) {
        case OSType::Professional:
        case OSType::Personal: return ::QObject::tr("UnionTech OS User Experience Program License Agreement");
        case OSType::Community:
        case OSType::Server:
        default: return ::QObject::tr("User Experience Program License Agreement");
    }
}

QString installer::LicenseDelegate::logo()
{
    switch (GetCurrentType()) {
        case OSType::Professional: return ":/images/logo.svg";
        case OSType::Personal:
        case OSType::Community:
        case OSType::Server:
        default: return installer::GetVendorLogo();
    }
}

QString installer::LicenseDelegate::product()
{
    switch (GetCurrentType()) {
        case OSType::Professional: return ::QObject::tr("UnionTech OS");
        case OSType::Community:
        case OSType::Server:
        case OSType::Personal:
        default: return ::QObject::tr("UOS");
    }
}

