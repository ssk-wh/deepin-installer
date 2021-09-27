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
        bool        Selected = false;
        QString     Id;
        QStringList PackageList;
    };

    class ComponentStruct {
    public:
        ComponentStruct(const QString& id, const QJsonObject& obj)
            : m_id(id)
        {
            QJsonArray array = obj["default"].toArray();
            for (QJsonValue value : array) {
                m_default << QSharedPointer<ComponentInfo>(new ComponentInfo);
                m_default.last()->Id = value.toString();
            }

            array = obj["choice"].toArray();
            for (QJsonValue value : array) {
                m_extra << QSharedPointer<ComponentInfo>(new ComponentInfo);
                m_extra.last()->Id = value.toString();
            }

            array = obj["uninstall"].toArray();
            for (QJsonValue value : array) {
                m_uninstall << QSharedPointer<ComponentInfo>(new ComponentInfo);
                m_uninstall.last()->Id = value.toString();
            }

            array = obj["choice-uninstall"].toArray();
            for (QJsonValue value : array) {
                m_choiceUninstall << QSharedPointer<ComponentInfo>(new ComponentInfo);
                m_choiceUninstall.last()->Id = value.toString();
            }
        }

        inline QString id() const {
            return m_id;
        }

        inline QList<QSharedPointer<ComponentInfo>> defaultValue() const {
            return m_default;
        }


        inline QList<QSharedPointer<ComponentInfo>> extra() const {
            return m_extra;
        }

        inline QList<QSharedPointer<ComponentInfo>> uninstall() const {
            return m_uninstall;
        }

        inline QList<QSharedPointer<ComponentInfo>> choiceUninstall() const {
            return m_choiceUninstall;
        }

    private:
        QString m_id;
        QList<QSharedPointer<ComponentInfo>> m_default;
        QList<QSharedPointer<ComponentInfo>> m_extra;
        QList<QSharedPointer<ComponentInfo>> m_uninstall;
        QList<QSharedPointer<ComponentInfo>> m_choiceUninstall;
    };
}

namespace installer {
class ComponentInstallManager : public QObject
{
    Q_OBJECT
public:
    static ComponentInstallManager* Instance(bool showWarning = true);

    QSharedPointer<ComponentStruct> findComponentById(const QString& id);

    inline QList<QSharedPointer<ComponentStruct>> list() const {
        return m_list;
    }

    inline QList<QSharedPointer<ComponentInfo>> packageList() const {
        return m_packageList;
    }

    QStringList GetAvailablePackages() const;
    QStringList packageListByComponentStruct(QSharedPointer<ComponentStruct> componentStruct) const;
    QStringList uninstallPackageListByComponentStruct(QSharedPointer<ComponentStruct> componentStruct
                                                      , const bool isMinimalGhaphicInstall) const;

    QStringList loadStructForLanguage(const QString& lang) const;
    QPair<QString, QString> updateTs(const QString& id) const;
    QPair<QString, QString> updateTs(QSharedPointer<ComponentStruct> componentStruct) const;
    QPair<QString, QString> updateTs(QSharedPointer<ComponentInfo> info) const;

    QStringList integrateList(QList<QSharedPointer<ComponentInfo>> list, const QStringList& packageList) const;
    // Take the intersection of two lists, keep the order according to the first list.
    QStringList integrateList(const QStringList& list1, const QStringList& list2) const;

    void togglePackageWarning();

private:
    explicit ComponentInstallManager(bool showWarning, QObject *parent = nullptr);
    ComponentInstallManager(const ComponentInstallManager& manager) = delete;
    ~ComponentInstallManager() override = default;

    void readStandartSortFile();
    QStringList getComponentSortList(QSharedPointer<ComponentStruct> componentStruct);

private:
    QList<QSharedPointer<ComponentStruct>> m_list;
    QList<QSharedPointer<ComponentInfo>> m_packageList;
    QList<QPair<QString, QStringList>> m_standartSort;
};
}

#endif // COMPONENTINSTALLMANAGER_H
