#include "third_party/googletest/include/gtest/gtest.h"
#include "ui/delegates/componentinstallmanager.h"

namespace installer {
namespace {
TEST(ComponentUtil, InstallPackage)
{
    ComponentInstallManager*        manager{ ComponentInstallManager::Instance(false) };
    auto                            list = manager->list();
    QSharedPointer<ComponentStruct> ddeDesktop;
    for (auto it = list.cbegin(); it != list.cend(); ++it) {
        if (it->get()->id() == "dde-desktop") {
            ddeDesktop = *it;
            break;
        }
    }

    ASSERT_NE(ddeDesktop.get(), nullptr);

    for (auto v : ddeDesktop->defaultValue()) {
        v->Selected = true;
    }

    for (auto v : ddeDesktop->extra()) {
        v->Selected = true;
    }

    QStringList packageList;

    for (auto info : manager->packageList()) {
        packageList << info->PackageList;
    }

    ASSERT_FALSE(manager->integrateList(ddeDesktop->defaultValue(), packageList).isEmpty());
    ASSERT_FALSE(manager->integrateList(ddeDesktop->extra(), packageList).isEmpty());
}
}  // namespace
}  // namespace installer
