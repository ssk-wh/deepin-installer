/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     justforlxz <justforlxz@outlook.com>
 *
 * Maintainer: justforlxz <justforlxz@outlook.com>
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

#include "componentinstallmanager.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "base/file_util.h"
#include "base/command.h"

#include <QDebug>
#include <QPair>
#include <DSysInfo>

DCORE_USE_NAMESPACE

using namespace installer;

ComponentInstallManager *ComponentInstallManager::Instance(bool showWarning)
{
    static ComponentInstallManager manager(showWarning);
    return &manager;
}

void ComponentInstallManager::readStandartSortFile()
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(GetComponentSort().toUtf8(), &error);

    if (error.error == QJsonParseError::NoError){
        if(doc.isArray()){
            QJsonArray array = doc.array();
            for (auto it = array.begin(); it != array.end(); ++it) {
                QPair<QString, QStringList> pair;
                const QJsonObject obj = it->toObject();
                for (auto objIt = obj.begin(); objIt != obj.end(); ++objIt) {
                    pair.first = objIt.key();
                    if(objIt.value().isArray()){
                        QJsonArray valueArray = objIt.value().toArray();
                        for (auto valIt = valueArray.begin(); valIt != valueArray.end()
                             ; ++valIt) {
                            pair.second << valIt->toString();
                        }
                    }

                    m_standartSort << pair;
                }
            }
        }
    }
}

QStringList ComponentInstallManager::getComponentSortList(QSharedPointer<ComponentStruct> componentStruct)
{
    // Traverse m_standartSort looking for items matching componentStruct->id().
    auto it = std::find_if(m_standartSort.begin(), m_standartSort.end(), [=] (const QPair<QString, QStringList>& pair) {
        return pair.first == componentStruct->id();
    });

    if (it != m_standartSort.end()) {
        return it->second;
    }

    return QStringList();
}

QStringList ComponentInstallManager::GetAvailablePackages() const {
    QDir dir("/lib/live/mount/medium/dists/");
    if (!dir.exists()) {
        dir.setPath("/run/live/medium/dists/");
    }

    if (!dir.exists()) {
        qDebug() << "/media/cdrom not exist.";
        return {};
    }

    QSet<QString> packagesList;

    QFileInfoList list = dir.entryInfoList();
    for (const QFileInfo& info : list) {
        if (info.fileName() == "." || info.fileName() == ".." || !info.isDir()) {
            continue;
        }

        QDir          distDir(QString("%1/main/").arg(info.filePath()));
        QFileInfoList distList = distDir.entryInfoList();
        for (const QFileInfo& i : distList) {
            QFile file(QString("%1/Packages").arg(i.filePath()));
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                QString     line;
                while (stream.readLineInto(&line)) {
                    if (line.startsWith("Package")) {
                        packagesList << line.split(":").last().simplified();
                    }
                }
            }
        }
    }

    QStringList allPack;

    for (auto it = m_packageList.cbegin(); it != m_packageList.cend(); ++it) {
        allPack << it->get()->PackageList;
    }

    if (allPack.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "all package is empty!";
    }

    return integrateList(allPack, packagesList.toList());
}

QSharedPointer<ComponentStruct> ComponentInstallManager::findComponentById(const QString &id)
{
    return *std::find_if(m_list.cbegin(), m_list.cend(), [=] (const QSharedPointer<ComponentStruct>& info) {
        return info->id() == id;
    });
}

ComponentInstallManager::ComponentInstallManager(bool showWarning, QObject *parent) : QObject(parent)
{
    readStandartSortFile();

    QJsonDocument doc = QJsonDocument::fromJson(GetComponentDefault().toUtf8());
    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QSharedPointer<ComponentStruct> component(new ComponentStruct(it.key(), it.value().toObject()));
        m_list << component;
    }

    std::sort(m_list.begin(), m_list.end(), [=] (QSharedPointer<ComponentStruct> left
              , QSharedPointer<ComponentStruct> right) {
        int leftIndex = 0;
        int rightIndex = 0;

        for (QPair<QString, QStringList> pair : m_standartSort) {
            if (pair.first == left->id()) {
                leftIndex = m_standartSort.indexOf(pair);
            }

            if (pair.first == right->id()) {
                rightIndex = m_standartSort.indexOf(pair);
            }
        }

        return leftIndex < rightIndex;
    });

    for (QSharedPointer<ComponentStruct> componentStruct : m_list) {
        QList<QSharedPointer<ComponentInfo>> extra = componentStruct->extra();
        std::sort(extra.begin(), extra.end(), [=] (QSharedPointer<ComponentInfo> left
                  , QSharedPointer<ComponentInfo> right) {
            int leftIndex = 0;
            int rightIndex = 0;

            QStringList componentList = getComponentSortList(componentStruct);
            for(QString str : componentList){
                if(left->Id == str){
                    leftIndex = componentList.indexOf(str);
                }

                if(right->Id == str){
                    rightIndex = componentList.indexOf(str);
                }
            }

            return leftIndex < rightIndex;
        });
    }

    QJsonDocument packageDoc = QJsonDocument::fromJson(GetComponentExtra().toUtf8());
    obj = packageDoc.object();

    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QSharedPointer<ComponentInfo> info(new ComponentInfo);
        info->Id = it.key();
        for (QJsonValue value : it.value().toArray()) {
            info->PackageList << value.toString();
        }

        m_packageList << info;
    }

    if (!showWarning) {
        return;
    }

    // 加載所有的deb包
    const QStringList packagesList{ GetAvailablePackages() };

    for (auto it = obj.begin(); it != obj.end(); ++it) {
        for (QJsonValue value : it.value().toArray()) {
            if (!packagesList.contains(value.toString())) {
                qWarning() << QString("Package %1 not found!").arg(value.toString());
            }
        }
    }
}

QStringList ComponentInstallManager::integrateList(QList<QSharedPointer<ComponentInfo>> list, const QStringList& packageList) const {
    QStringList       result;
    for (QSharedPointer<ComponentInfo> info : list) {
        if (!info->Selected) continue;

        // Traverse m_packageList looking for items matching info->Id.
        auto it = std::find_if(m_packageList.begin(), m_packageList.end(), [=] (const QSharedPointer<ComponentInfo>& l) {
            return l->Id == info->Id;
        });

        if (it != m_packageList.end()) {
            result += integrateList((*it)->PackageList, packageList);
        }
    }

    if (result.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "package list is empty!";
    }

    return result;
}

QStringList ComponentInstallManager::integrateList(const QStringList &list1, const QStringList &list2) const
{
    QStringList tmpList;

    for (auto it : list1) {
        if (list2.contains(it)) {
            tmpList << it;
        }
    }

    return tmpList;
}

QStringList ComponentInstallManager::packageListByComponentStruct(QSharedPointer<ComponentStruct> componentStruct) const {
    return QStringList() << integrateList(componentStruct->defaultValue(),
                                          GetAvailablePackages())
                         << integrateList(componentStruct->extra(),
                                          GetAvailablePackages());
}

QStringList ComponentInstallManager::uninstallPackageListByComponentStruct(QSharedPointer<ComponentStruct> componentStruct
                                                                           , const bool isMinimalGhaphicInstall) const {
    QList<QSharedPointer<ComponentInfo>> uninstallList = componentStruct->uninstall();
    QSet<QString>       result;
    for (QSharedPointer<ComponentInfo> info : uninstallList) {
        // Traverse m_packageList looking for items matching info->Id.
        auto it = std::find_if(m_packageList.begin(), m_packageList.end(), [=] (const QSharedPointer<ComponentInfo>& i) {
            return  info->Id == i->Id;
        });

        if (it != m_packageList.end()) {
            result += QSet<QString>((*it)->PackageList.toSet());
        }
    }

    if (isMinimalGhaphicInstall) {
        QList<QSharedPointer<ComponentInfo>> choiceUninstallList = componentStruct->choiceUninstall();
        for (QSharedPointer<ComponentInfo> info : choiceUninstallList) {
            // Traverse m_packageList looking for items matching info->Id.
            auto it = std::find_if(m_packageList.begin(), m_packageList.end(), [=] (const QSharedPointer<ComponentInfo>& i) {
                return info->Id == i->Id;
            });

            if (it != m_packageList.end()) {
                result += QSet<QString>((*it)->PackageList.toSet());
            }
        }
    }

    return result.toList();
}

QStringList ComponentInstallManager::loadStructForLanguage(const QString &lang) const
{
    // Traverse m_packageList looking for items matching lang.
    auto it = std::find_if(m_packageList.begin(), m_packageList.end(), [=] (const QSharedPointer<ComponentInfo>& i) {
        return i->Id == lang;
    });

    if (it != m_packageList.end()) {
        return integrateList((*it)->PackageList, GetAvailablePackages());
    }

    return QStringList();
}

QPair<QString, QString> ComponentInstallManager::updateTs(const QString& id) const {
    return QMap<QString, QPair<QString, QString>> {
    {"core", {::QObject::tr("Core"), ::QObject::tr("Core packages for server OS.")}},
    {"base", {::QObject::tr("Base"), ::QObject::tr("Base packages for server OS.")}},
    {"Basic-Server-Environment", {::QObject::tr("Basic Server Environment"), ::QObject::tr("Applies to standard common service environments, such as Web services and database services.")}},
    {"Cloud-and-Virtualzalition", {::QObject::tr("Cloud and Virtualization"), ::QObject::tr("Applies to IaaS environments, such as virtualization, containers, and clouds.")}},
    {"Big-Data", {::QObject::tr("Big Data"), ::QObject::tr("Applies to servers that provide parallel processing of massive data.")}},
    {"Server-Environment-with-GUI", {::QObject::tr("Server Environment with GUI"), ::QObject::tr("Operates the services and applications on the graphical user interface.")}},
    {"Custom-Installation", {::QObject::tr("Custom Installation"), ::QObject::tr("Provides most package groups for custom selection, covering minimum to maximum installation.")}},
    {"debugging-tools", {::QObject::tr("Debugging Tools"), ::QObject::tr("Provide debugging package for program function and performance.")}},
    {"deepin", {::QObject::tr("Applications with GUI"), ::QObject::tr("Commonly used server application packages in graphical user interface.")}},
    {"development-tools", {::QObject::tr("Development Tools"), ::QObject::tr("Development tool packages in multiple programming languages.")}},
    {"directory-client", {::QObject::tr("Directory Client"), ::QObject::tr("Clients for integration into a network managed by a directory service.")}},
    {"dns-server", {::QObject::tr("Domain Name Service"), ::QObject::tr("Environments for running the domain name service (DNS).")}},
    {"file-server", {::QObject::tr("File and Storage Server"), ::QObject::tr("CIFS, SMB, NFS, iSCSI, iSER, and iSNS network storage server.")}},
    {"graphics", {::QObject::tr("Graphics Creation Tools"), ::QObject::tr("Software for creating and editing images.")}},
    {"load-balancer", {::QObject::tr("Load Balancer"), ::QObject::tr("Load balancing support for network traffic.")}},
    {"mariadb-server", {::QObject::tr("MariaDB Database Server"), ::QObject::tr("The MariaDB SQL database server, and associated packages.")}},
    {"mail-server", {::QObject::tr("E-mail Server"), ::QObject::tr("Allows the system to act as a SMTP and/or IMAP e-mail server.")}},
    {"network-file-system-client", {::QObject::tr("Network File System Client"), ::QObject::tr("Environments for network storage devices.")}},
    {"performance-tools", {::QObject::tr("Performance Tools"), ::QObject::tr("Packages for monitoring and diagnosing hardware devices, system programs, and applications.")}},
    {"security-tools", {::QObject::tr("Security Tools"), ::QObject::tr("Packages for integrity and trust verification.")}},
    {"web-server", {::QObject::tr("Web Server"), ::QObject::tr("Provides Java servlets and Web common service components.")}},
    {"guest-agents", {::QObject::tr("Guest Agents"), ::QObject::tr("Agents used when running under a hypervisor.")}},
    {"virtualization-client", {::QObject::tr("Virtualization Client"), ::QObject::tr("Clients for installing and managing virtualization instances.")}},
    {"virtualization-hypervisor", {::QObject::tr("Virtualization Hypervisor"), ::QObject::tr("Smallest possible virtualization host installation.")}},
    {"virtualization-tools", {::QObject::tr("Virtualization Tools"), ::QObject::tr("Tools for offline virtual image management.")}},
    {"virtualization-platform", {::QObject::tr("Virtualization Platform"), ::QObject::tr("Provides an interface for acessing and controlling virtualized guests and containers.")}},
    {"ha", {::QObject::tr("High Availability"), ::QObject::tr("Infrastructure for highly available services and/or shared storage.")}},
    {"infiniband", {::QObject::tr("InfiniBand Support"), ::QObject::tr("Software designed for supporting clustering and grid connectivity using RDMA-based InfiniBand and iWARP fabrics.")}},
    {"large-systems", {::QObject::tr("Large Systems Performance"), ::QObject::tr("Performance support tools for large systems.")}},
    {"platform-devel", {::QObject::tr("Platform Development"), ::QObject::tr("Recommended development headers and libraries for developing applications to run on UOS.")}},
    {"internet-applications", {::QObject::tr("Internet Applications"), ::QObject::tr("Email, chat, and video conferencing software.")}},
    {"virtualization-tool-set", {::QObject::tr("Virtualization Toolset"), ::QObject::tr("Toolset for virtualization enviroment (host and guest).")}},
    {"dde", {::QObject::tr("Server Environment with GUI"), ::QObject::tr("Operates the services and applications on the graphical user interface.")}},
    {"postgresql-server", {::QObject::tr("PostgreSQL Database Server"), ::QObject::tr("The PostgreSQL SQL database server, and associated packages.")}},
    }[id];
}

QPair<QString, QString> ComponentInstallManager::updateTs(QSharedPointer<ComponentStruct> componentStruct) const {
    return updateTs(componentStruct->id());
}

QPair<QString, QString> ComponentInstallManager::updateTs(QSharedPointer<ComponentInfo> info) const {
    return updateTs(info->Id);
}
