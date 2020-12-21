#pragma once

#include "sysinfo/timezone.h"

#include <QFrame>
#include <QStringListModel>
#include <QStandardItemModel>

#include <DListView>
#include <DStandardItem>
#include <DFrame>

DWIDGET_USE_NAMESPACE

namespace installer {

class TimeZoneModel;
class ContinentModel;

class SelectTimeZoneFrame : public DFrame
{
    Q_OBJECT

public:
    explicit SelectTimeZoneFrame(QWidget* parent = nullptr);

    void updateContinentModelData();
    void updateTimezoneModelData();
    void onContinentViewSelectedChanged(QModelIndex preIndex/*, QModelIndex curIndex*/);
    void onTimeZoneViewSelectedChanged(QModelIndex preIndex/*, QModelIndex curIndex*/);
    void onUpdateTimezoneList(const QString& timezone);

    void setWidgetfocus(bool isfocus = true);
    bool focusSwitch();
    bool doSpace();
    bool doSelect();
    bool directionKey(int keyvalue);

signals:
    void timezoneUpdated(const QString& timezone);

private:
    void initUI();
    void initConnections();
    void setSelectItem(QModelIndex index);

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
    QModelIndex m_oldTimezoneIndex;
    QString m_listViewWrapOldStyleSheet = "";
};

}
