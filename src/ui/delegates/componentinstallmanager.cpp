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

#include <QDebug>

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
#ifdef QT_DEBUG
    QJsonDocument doc = QJsonDocument::fromJson("{\"最小系统\":{\"default\":[\"a\"],\"extra\":[\"1\",\"2\"]},\"计算机节点\":{\"default\":[\"b\"],\"extra\":[\"2\",\"1\"]},\"基础设施服务器\":{\"default\":[\"c\"],\"extra\":[\"1\",\"2\",\"3,\"]},\"文件及打印服务器\":{\"default\":[\"d\"],\"extra\":[\"1\",\"3\"]},\"基本网页服务器\":{\"default\":[\"e\"],\"extra\":[\"4\",\"1\"]},\"虚拟化主机\":{\"default\":[\"f\"],\"extra\":[\"1\",\"2\"]},\"带GUI的服务器\":{\"default\":[\"f\"],\"extra\":[\"3\",\"2\"]},\"DDE桌面\":{\"default\":[\"a\"],\"extra\":[\"4\",\"1\"]},\"开发及生成工作站\":{\"default\":[\"e\"],\"extra\":[\"1\",\"3\"]}}");
    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QSharedPointer<ComponentStruct> component(new ComponentStruct(it.key(), it.value().toObject()));
        m_list << component;
    }

    for (auto it = m_list.cbegin(); it != m_list.cend(); ++it) {
        qDebug() << it->get()->id();
        QList<QSharedPointer<ComponentInfo>> defaultList = it->get()->defaultValue();
        for (QSharedPointer<ComponentInfo> info : defaultList) {
            qDebug() << info->Id << info->PackageList;
        }

        QList<QSharedPointer<ComponentInfo>> extraList = it->get()->extra();
        for (QSharedPointer<ComponentInfo> info : extraList) {
            qDebug() << info->Id << info->PackageList;
        }
        qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>";
    }
#endif
}
