#include "select_time_zone_frame.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/nav_button.h"
#include "base/file_util.h"
#include "ui/views/disk_installation_view.h"
#include "ui/views/frameless_list_view.h"
#include "ui/models/disk_installation_model.h"
#include "ui/delegates/disk_installation_delegate.h"
#include "ui/models/continent_model.h"

#include <QCheckBox>
#include <QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QDebug>

namespace installer {

namespace {
    int kContinentListViewWidth = 240;
    int kTimeZoneListViewWidth = 450;
    int kListViewHeight = 540;
}

SelectTimeZoneFrame::SelectTimeZoneFrame(QWidget *parent)
    : QFrame (parent)
    , m_allTimeZone(GetContinentZoneInfo())
{
    initUI();
    initConnections();
}

void SelectTimeZoneFrame::initUI()
{
    m_continentListView = new DiskInstallationView;
    m_continentListView->setFixedSize(QSize(kContinentListViewWidth, kListViewHeight));
    m_continentListView->setStyleSheet(ReadFile(":/styles/timezone_frame.css"));

    m_continentModel = new ContinentModel;
    QStringList strList;
    for (auto it = m_allTimeZone.begin(); it != m_allTimeZone.end(); ++it) {
        strList << it.key();
    }
    m_continentModel->setStringList(strList);
    m_continentListView->setModel(m_continentModel);
    DiskInstallationItemDelegate* continentDelegate = new DiskInstallationItemDelegate;
    m_continentListView->setItemDelegate(continentDelegate);

    m_timeZoneListView = new FramelessListView;
    m_timeZoneListView->setFixedSize(QSize(kTimeZoneListViewWidth, kListViewHeight));

    m_timeZoneModel = new QStringListModel;
    m_timeZoneListView->setModel(m_timeZoneModel);

    QHBoxLayout* listViewLayout = new QHBoxLayout;
    listViewLayout->setMargin(5);
    listViewLayout->setContentsMargins(10, 10, 10, 10);
    listViewLayout->setSpacing(0);
    listViewLayout->addStretch();
    listViewLayout->addWidget(m_continentListView);
    listViewLayout->addSpacing(1);
    listViewLayout->addWidget(m_timeZoneListView);
    listViewLayout->addStretch();

    setLayout(listViewLayout);

    this->setStyleSheet(ReadFile(":/styles/select_time_zone_frame.css"));
}

void SelectTimeZoneFrame::initConnections()
{
    connect(m_continentListView->selectionModel(), &QItemSelectionModel::currentChanged
            , this, &SelectTimeZoneFrame::onContinentViewSelectedChanged);
    connect(m_timeZoneListView->selectionModel(), &QItemSelectionModel::currentChanged
            , this, &SelectTimeZoneFrame::onTimeZoneViewSelectedChanged);
}

void SelectTimeZoneFrame::onContinentViewSelectedChanged(QModelIndex curIndex, QModelIndex preIndex)
{
    Q_UNUSED(preIndex);

    if(!curIndex.isValid()){
        return;
    }
    if(!(curIndex.row() < m_continentModel->stringList().count())){
        return;
    }

    m_currentContinent = m_continentModel->stringList().at(curIndex.row());
    m_timeZoneModel->setStringList(m_allTimeZone[m_currentContinent]);

    m_timeZoneListView->scrollToTop();
}

void SelectTimeZoneFrame::onTimeZoneViewSelectedChanged(QModelIndex curIndex, QModelIndex preIndex)
{
    Q_UNUSED(preIndex);

    if(!curIndex.isValid()){
        return;
    }
    if(!(curIndex.row() < m_timeZoneModel->stringList().count())){
        return;
    }

    QString timezone = m_currentContinent + "/" + m_timeZoneModel->stringList().at(curIndex.row());
    emit timezoneUpdated(timezone);
}

}
