#pragma once

#include "sysinfo/timezone.h"

#include <QFrame>
#include <QListView>
#include <QStringListModel>

namespace installer {

class ContinentModel;
class TimeZoneModel;
class FramelessListView;
class DiskInstallationView;
class ContinentModel;

class SelectTimeZoneFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SelectTimeZoneFrame(QWidget* parent = nullptr);

    void onContinentViewSelectedChanged(QModelIndex preIndex, QModelIndex curIndex);
    void onTimeZoneViewSelectedChanged(QModelIndex preIndex, QModelIndex curIndex);

signals:
    void timezoneUpdated(const QString& timezone);

private:
    void initUI();
    void initConnections();

    ContinentZoneInfoMap m_allTimeZone;
    QString m_currentTimezone;

    DiskInstallationView* m_continentListView = nullptr;
    ContinentModel* m_continentModel = nullptr;

    FramelessListView* m_timeZoneListView = nullptr;
    QStringListModel* m_timeZoneModel = nullptr;
};

}
