#pragma once

#include "sysinfo/timezone.h"

#include <QFrame>
#include <QStringListModel>
#include <QStandardItemModel>

#include <DListView>
#include <DStandardItem>

DWIDGET_USE_NAMESPACE

namespace installer {

class TimeZoneModel;
class ContinentModel;

class SelectTimeZoneFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SelectTimeZoneFrame(QWidget* parent = nullptr);

    void updateContinentModelData();
    void updateTimezoneModelData();
    void onContinentViewSelectedChanged(QModelIndex preIndex, QModelIndex curIndex);
    void onTimeZoneViewSelectedChanged(QModelIndex preIndex, QModelIndex curIndex);
    void onUpdateTimezoneList(const QString& timezone);

signals:
    void timezoneUpdated(const QString& timezone);

protected:
    void changeEvent(QEvent* event) override;

private:
    void initUI();
    void initConnections();

    ContinentZoneInfoList m_allTimeZone;
    QMap<QString, QString> m_mapEnglishToInternation;
    QStringList m_currentContinentList;
    QStringList m_currentTimeZoneList;
    QModelIndex m_currentContinentIndex;
    QModelIndex m_currentTimezoneIndex;

    DListView* m_continentListView = nullptr;
    QStringListModel* m_continentModel = nullptr;

    DListView* m_timeZoneListView = nullptr;
    QStandardItemModel* m_timeZoneModel = nullptr;

    DStandardItem* m_lastItem = nullptr;
};

}
