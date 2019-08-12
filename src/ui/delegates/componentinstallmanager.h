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

#ifndef COMPONENTINSTALLMANAGER_H
#define COMPONENTINSTALLMANAGER_H

#include <QObject>
#include <QList>
#include <QSharedPointer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace installer {
    struct ComponentInfo {
        QString Id;
        QStringList PackageList;
    };

    class ComponentStruct {
    public:
        ComponentStruct(const QString& id, const QJsonObject& obj)
            : m_id(id)
            , m_default(new ComponentInfo)
            , m_extra(new ComponentInfo)
        {
            for (QJsonValue value : obj["default"].toArray()) {
                m_default->PackageList << value.toString();
            }

            for (QJsonValue value : obj["extra"].toArray()) {
                m_extra->PackageList << value.toString();
            }
        }

        inline QString id() const {
            return m_id;
        }

        inline QSharedPointer<ComponentInfo> defaultValue() const {
            return m_default;
        }


        inline QSharedPointer<ComponentInfo> extra() const {
            return m_extra;
        }

    private:
        QString m_id;
        QSharedPointer<ComponentInfo> m_default;
        QSharedPointer<ComponentInfo> m_extra;
    };
}

namespace installer {
class ComponentInstallManager : public QObject
{
    Q_OBJECT
public:
    static ComponentInstallManager* Instance();

    QSharedPointer<ComponentStruct> findComponentById(const QString& id);

    inline QList<QSharedPointer<ComponentStruct>> list() const {
        return m_list;
    }

private:
    explicit ComponentInstallManager(QObject *parent = nullptr);
    ComponentInstallManager(const ComponentInstallManager& manager) = delete;
    ~ComponentInstallManager() override = default;

private:
    QList<QSharedPointer<ComponentStruct>> m_list;
};
}

#endif // COMPONENTINSTALLMANAGER_H
