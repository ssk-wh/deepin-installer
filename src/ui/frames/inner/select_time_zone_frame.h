#pragma once

#include "sysinfo/timezone.h"

#include <QFrame>
#include <QListView>
#include <QStringListModel>

namespace installer {

class TimeZoneModel;
class DiskInstallationView;
class ContinentModel;

class SelectTimeZoneFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SelectTimeZoneFrame(QWidget* parent = nullptr);

    void onContinentViewSelectedChanged(QModelIndex preIndex, QModelIndex curIndex);
    void onTimeZoneViewSelectedChanged(QModelIndex preIndex, QModelIndex curIndex);
    void onUpdateTimezoneList(const QString& timezone);

signals:
    void timezoneUpdated(const QString& timezone);

private:
    void initUI();
    void initConnections();

    ContinentZoneInfoMap m_allTimeZone;
    QStringList m_continentList;
    QStringList m_currentTimeZone;
    QModelIndex m_currentContinentIndex;

    DiskInstallationView* m_continentListView = nullptr;
    ContinentModel* m_continentModel = nullptr;

    DiskInstallationView* m_timeZoneListView = nullptr;
    QStringListModel* m_timeZoneModel = nullptr;
};

}
