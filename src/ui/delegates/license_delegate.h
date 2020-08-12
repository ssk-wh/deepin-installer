#ifndef LICENSE_DELEGATE_H
#define LICENSE_DELEGATE_H

#include <QString>

namespace installer {

class LicenseDelegate
{
public:
    static QString licenseTitle();
    static QString userExperienceTitle();
    static QString privacyLicenseTitle();

    static QString logo();
    static QString product();

    static QString OSType();

private:
    LicenseDelegate() = delete;
    LicenseDelegate(const LicenseDelegate &) = delete;
};


}

#endif // LICENSE_DELEGATE_H
