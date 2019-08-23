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

#include <QDebug>
#include <QApt/DebFile>

using namespace installer;

ComponentInstallManager *ComponentInstallManager::Instance()
{
    static ComponentInstallManager manager;
    return &manager;
}

QSharedPointer<ComponentStruct> ComponentInstallManager::findComponentById(const QString &id)
{
    return *std::find_if(m_list.cbegin(), m_list.cend(), [=] (const QSharedPointer<ComponentStruct>& info) {
        return info->id() == id;
    });
}

ComponentInstallManager::ComponentInstallManager(QObject *parent) : QObject(parent)
{
    QJsonDocument doc = QJsonDocument::fromJson(GetComponentDefault().toUtf8());
    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QSharedPointer<ComponentStruct> component(new ComponentStruct(it.key(), it.value().toObject()));
        m_list << component;
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

    // 加載所有的deb包
    QDir dir("/lib/live/mount/medium/pool/main/");
    if (!dir.exists()) {
        dir.setPath("/run/live/medium/pool/main/");
    }

    if (dir.exists()) {
        const QStringList& list = findAllDeb(dir.path());
        for (auto it = m_packageList.begin(); it != m_packageList.end();) {
            const QString& id = it->data()->Id;
            if (!list.contains(id)) {
                qWarning() << "Package not exist: " << id;
                it = m_packageList.erase(it);
            }
            else {
                ++it;
            }
        }
    }
    else {
        qDebug() << "/media/cdrom not exist.";
    }
}

QStringList ComponentInstallManager::packageListByComponentStruct(QSharedPointer<ComponentStruct> componentStruct) const {
    auto integrateList = [=](QList<QSharedPointer<ComponentInfo>> list) -> QStringList {
        QStringList packageList;
        for (QSharedPointer<ComponentInfo> info : list) {
            if (!info->Selected) continue;
            for (QSharedPointer<ComponentInfo> i : m_packageList) {
                if (info->Id == i->Id) {
                    packageList << i->PackageList;
                    break;
                }
            }
        }
        return packageList;
    };

    return QStringList() << integrateList(componentStruct->defaultValue())
                         << integrateList(componentStruct->extra());
}

QStringList ComponentInstallManager::uninstallPackageListByComponentStruct(QSharedPointer<ComponentStruct> componentStruct) const {
    QStringList list;
    QList<QSharedPointer<ComponentInfo>> uninstallList = componentStruct->uninstall();

    for (QSharedPointer<ComponentInfo> info : uninstallList) {
        for (QSharedPointer<ComponentInfo> i : m_packageList) {
            if (info->Id == i->Id) {
                list << i->PackageList;
                break;
            }
        }
    }

    return list;
}

QStringList ComponentInstallManager::loadStructForLanguage(const QString &lang) const
{
    for (QSharedPointer<ComponentInfo> i : m_packageList) {
        if (i->Id == lang) {
            return i->PackageList;
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
        if (fileInfo.fileName() == "." | fileInfo.fileName() == "..") {
            i++;
            continue;
        }

        if (fileInfo.isDir()) {
            packageList << findAllDeb(fileInfo.filePath());
        }
        else {
            packageList << fileInfo.fileName();
        }
    }

    return packageList;
}
