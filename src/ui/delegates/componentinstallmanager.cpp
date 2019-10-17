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
#include <QApt/DebFile>
#include <QPair>

#ifndef QT_DEBUG
#include <QtConcurrent/QtConcurrent>
#endif

using namespace installer;

ComponentInstallManager *ComponentInstallManager::Instance()
{
    static ComponentInstallManager manager;
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
    for (auto it : m_standartSort) {
        if(it.first == componentStruct->id()){
            return it.second;
        }
    }

    return QStringList();
}

QStringList ComponentInstallManager::GetAvailablePackages() const {
    QDir dir("/lib/live/mount/medium/pool/main/");
    if (!dir.exists()) {
        dir.setPath("/run/live/medium/pool/main/");
    }

    if (!dir.exists()) {
        qDebug() << "/media/cdrom not exist.";
        return {};
    }

    const QStringList&   list = findAllDeb(dir.path());
    QList<QApt::DebFile> files;
    QStringList packagesList;
    QStringList allPack;

    for (const QString& l : list) {
        QApt::DebFile file(l);
        packagesList << file.packageName();
    }

    for (auto it = m_packageList.cbegin(); it != m_packageList.cend(); ++it) {
        allPack << it->get()->PackageList;
    }

    return QSet<QString>(packagesList.toSet() & allPack.toSet()).toList();
}

QSharedPointer<ComponentStruct> ComponentInstallManager::findComponentById(const QString &id)
{
    return *std::find_if(m_list.cbegin(), m_list.cend(), [=] (const QSharedPointer<ComponentStruct>& info) {
        return info->id() == id;
    });
}

ComponentInstallManager::ComponentInstallManager(QObject *parent) : QObject(parent)
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

#ifndef QT_DEBUG
    QtConcurrent::run([=] {
        // 加載所有的deb包
        const QStringList packagesList { GetAvailablePackages() };

        for (auto it = obj.begin(); it != obj.end(); ++it) {
            for (QJsonValue value : it.value().toArray()) {
                if (!packagesList.contains(value.toString())) {
                    qWarning() << QString("Package %1 not found!").arg(value.toString());
                }
            }
        }
    });
#endif
}

QStringList ComponentInstallManager::packageListByComponentStruct(QSharedPointer<ComponentStruct> componentStruct) const {
    const QSet<QString> packagesList { GetAvailablePackages().toSet() };

    auto integrateList = [=](QList<QSharedPointer<ComponentInfo>> list) -> QStringList {
        QSet<QString> result;
        for (QSharedPointer<ComponentInfo> info : list) {
            if (!info->Selected) continue;
            for (QSharedPointer<ComponentInfo> l : m_packageList) {
                if (l->Id == info->Id) {
                    result += QSet<QString>(l->PackageList.toSet() & packagesList);
                    break;
                }
            }
        }

        return result.toList();
    };

    return QStringList() << integrateList(componentStruct->defaultValue())
                         << integrateList(componentStruct->extra());
}

QStringList ComponentInstallManager::uninstallPackageListByComponentStruct(QSharedPointer<ComponentStruct> componentStruct) const {
    QString dpkgResult;
    qDebug() << SpawnCmd("dpkg", { "-l" }, dpkgResult);

    QTextStream stream(&dpkgResult);
    QString line;
    QStringList installedList;
    while (stream.readLineInto(&line)) {
        const QStringList list {
            line.simplified().split(" ")
        };

        if (list.startsWith("ii")) {
            installedList << QString(list.at(1)).split(":").first();
        }
    }

    QList<QSharedPointer<ComponentInfo>> uninstallList = componentStruct->uninstall();
    for (QSharedPointer<ComponentInfo> info : uninstallList) {
        for (QSharedPointer<ComponentInfo> i : m_packageList) {
            if (info->Id == i->Id) {
                return QSet<QString>(i->PackageList.toSet() & installedList.toSet()).toList();
            }
        }
    }

    return {};
}

QStringList ComponentInstallManager::loadStructForLanguage(const QString &lang) const
{
    for (QSharedPointer<ComponentInfo> i : m_packageList) {
        if (i->Id == lang) {
            return GetAvailablePackages()
                .toSet()
                .intersect(i->PackageList.toSet())
                .toList();
        }
    }

    return QStringList();
}

QStringList ComponentInstallManager::findAllDeb(const QString& path) const {
    QDir d(path);
    if (!d.exists()) {
        return QStringList();
    }

    d.setFilter(QDir::Dirs | QDir::Files);
    d.setSorting(QDir::DirsFirst);

    QFileInfoList list = d.entryInfoList();
    int           i    = 0;
    QStringList   packageList;

    for (const QFileInfo& fileInfo : list) {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") {
            i++;
            continue;
        }

        if (fileInfo.isDir()) {
            packageList << findAllDeb(fileInfo.filePath());
        }
        else {
            packageList << fileInfo.filePath();
        }
    }

    return packageList;
}

QPair<QString, QString> ComponentInstallManager::updateTs(const QString& id) const {
    return QMap<QString, QPair<QString, QString>> {
        {"minimal-install", {tr("Minimal Install"), tr("Basic functionality.")}},
        {"compute-node", {tr("Compute Node"), tr("Installation for performing computation and processing.")}},
        {"infrastructure-server", {tr("Infrastructure Server"), tr("Server for operating network infrastructure services.")}},
        {"file-and-print-server", {tr("File and Print Server"), tr("File, print, and storage server for enterprises.")}},
        {"basic-web-server", {tr("Basic Web Server"), tr("Server for serving static and dynamic internet content.")}},
        {"virtualization-host", {tr("Virtualization Host"), tr("Minimal virtualization host.")}},
        {"server-with-gui", {tr("Server with GUI"), tr("Server for operating network infrastructure services, with a GUI.")}},
        {"dde-desktop", {tr("DDE Desktop"), tr("DDE is a highly intuitive and user friendly desktop environment.")}},
        {"development-and-creative-workstation", {tr("Development and Creative Workstation"), tr("Workstation for software, hardware, graphics, or content development.")}},
        {"debugging-tools", {tr("Debugging Tools"), tr("Tools for debugging misbehaving applications and diagnosing performance problems.")}},
        {"directory-client", {tr("Directory Client"), tr("Clients for integration into a network managed by a directory service.")}},
        {"security-tools", {tr("Security Tools"), tr("Security tools for integrity and trust verification.")}},
        {"development-tools", {tr("Development Tools"), tr("A basic development environment.")}},
        {"performance-tools", {tr("Performance Tools"), tr("Tools for diagnosing system and application-level performance problems.")}},
        {"hardware-monitoring", {tr("Hardware Monitoring Utilities"), tr("A set of tools to monitor server hardware.")}},
        {"virtualization-hypervisor", {tr("Virtualization Hypervisor"), tr("Smallest possible virtualization host installation.")}},
        {"virtualization-platform", {tr("Virtualization Platform"), tr("Provide an interface for accessing and controlling virtualized guests and containers.")}},
        {"virtualization-client", {tr("Virtualization Client"), tr("Clients for installing and managing virtualization instances.")}},
        {"backup-client", {tr("Backup Client"), tr("Client tools for connecting to a backup server and doing backups.")}},
        {"backup-server", {tr("Backup Server"), tr("Software to centralize your infrastructure's backups.")}},
        {"file-server", {tr("File and Storage Server"), tr("CIFS, SMB, NFS, iSCSI, iSER, and iSNS network storage server.")}},
        {"dns-server", {tr("DNS Name Server"), tr("This package group allows you to run a DNS name server (BIND) on the system.")}},
        {"mail-server", {tr("E-mail Server"), tr("Allows the system to act as a SMTP and/or IMAP e-mail server.")}},
        {"ftp-server", {tr("FTP Server"), tr("Allows the system to act as an FTP server.")}},
        {"print-server", {tr("Print Server"), tr("Allows the system to act as a print server.")}},
        {"mainframe-access", {tr("Mainframe Access"), tr("Tools for accessing mainframe computing resources.")}},
        {"infiniband", {tr("Infiniband Support"), tr("Software designed for supporting clustering and grid connectivity using RDMA-based InfiniBand and iWARP fabrics.")}},
        {"ha", {tr("High Availability"), tr("Infrastructure for highly available services and/or shared storage.")}},
        {"resilient-storage", {tr("Resilient Storage"), tr("Clustered storage, including the GFS2 file system.")}},
        {"identity-management-server", {tr("ldentity Management Server"), tr("Centralized management of users, servers and authentication policies.")}},
        {"large-systems", {tr("Large Systems Performance"), tr("Performance support tools for large systems.")}},
        {"load-balancer", {tr("Load Balancer"), tr("Load balancing support for network traffic.")}},
        {"mariadb-client", {tr("MariaDB Database Client"), tr("The MariaDB SQL database client, and associated packages.")}},
        {"mariadb-server", {tr("MariaDB Database Server"), tr("The MariaDB SQL database server, and associated packages.")}},
        {"postgresql-client", {tr("PostgreSQL Database Client"), tr("The PostgreSQL SQL database client, and associated packages.")}},
        {"postgresql-server", {tr("PostgreSQL Database Server"), tr("The PostgreSQL SQL database server, and associated packages.")}},
        {"java-platform", {tr("Java Platform"), tr("Java support for deepin.")}},
        {"php", {tr("PHP Support"), tr("PHP web application framework.")}},
        {"python-web", {tr("Python"), tr("Basic Python web application support.")}},
        {"perl-web", {tr("Perl for Web"), tr("Basic Perl web application support.")}},
        {"internet-applications", {tr("Internet Applications"), tr("Email, chat, and video conferencing software.")}},
        {"web-servlet", {tr("Web Servlet Engine"), tr("Allows the system to host Java servlets.")}},
        {"legacy-x", {tr("Legacy x Window System Compatibility"), tr("Compatibility programs for migration from or working with legacy X Window System environments.")}},
        {"office-suite", {tr("Office Suite and Productivity"), tr("A full-purpose office suite, and other productivity tools.")}},
        {"additional-devel", {tr("Additional Development"), tr("Additional development headers and libraries for building open-source applications.")}},
        {"emacs", {tr("Emacs"), tr("The GNU Emacs extensible, customizable text editor.")}},
        {"graphics", {tr("Graphics Creation Tools"), tr("Software for creation and manipulation of still images.")}},
        {"platform-devel", {tr("Platform Development"), tr("Recommended development headers and libraries for developing applications to run on deepin.")}},
        {"technical-writing", {tr("Technical Writing"), tr("Tools for writing technical documentation.")}},
        {"virtualization-tools", {tr("Virtualization Tools"), tr("Tools for offline virtual image management.")}},
        {"network-file-system-client", {tr("Network File System Client"), tr("Enables the system to attach to network storage.")}},
        {"guest-agents", {tr("Guest Agents"), tr("Agents used when running under a hypervisor.")}},
        {"deepin", {tr("DDE Applications"), tr("A set of commonly used DDE Applications.")}},
    }[id];
}

QPair<QString, QString> ComponentInstallManager::updateTs(QSharedPointer<ComponentStruct> componentStruct) const {
    return updateTs(componentStruct->id());
}

QPair<QString, QString> ComponentInstallManager::updateTs(QSharedPointer<ComponentInfo> info) const {
    return updateTs(info->Id);
}
