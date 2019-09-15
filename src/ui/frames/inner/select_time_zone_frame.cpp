#include "select_time_zone_frame.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/nav_button.h"
#include "base/file_util.h"
#include "ui/views/disk_installation_view.h"
#include "ui/models/disk_installation_model.h"
#include "ui/delegates/disk_installation_delegate.h"
#include "ui/delegates/frameless_list_delegate.h"
#include "ui/models/continent_model.h"
#include "sysinfo/timezone.h"
#include "service/settings_manager.h"

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
    m_continentListView->setStyleSheet(ReadFile(":/styles/select_time_zone_frame.css"));
    m_continentListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_continentModel = new ContinentModel;
    QStringList strList;
    const QString& locale = ReadLocale();
    for (auto it = m_allTimeZone.begin(); it != m_allTimeZone.end(); ++it) {
        m_continentList << it.key();
        Q_ASSERT(it.value().count() > 0);
        strList << GetLocalTimezoneName(it.key() + "/" + it.value().at(0), locale).first;
    }
    m_continentModel->setStringList(strList);
    m_continentListView->setModel(m_continentModel);
    DiskInstallationItemDelegate* continentDelegate = new DiskInstallationItemDelegate;
    m_continentListView->setItemDelegate(continentDelegate);

    m_timeZoneListView = new DiskInstallationView;
    m_timeZoneListView->setFixedSize(QSize(kTimeZoneListViewWidth, kListViewHeight));
    m_timeZoneListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_timeZoneListView->setStyleSheet(ReadFile(":/styles/select_time_zone_frame.css"));

    m_timeZoneModel = new QStringListModel;
    m_timeZoneListView->setModel(m_timeZoneModel);

    FramelessItemDelegate* delegate = new FramelessItemDelegate(this);
    m_timeZoneListView->setItemDelegate(delegate);

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

    m_currentContinentIndex = curIndex;
    const QString& locale = ReadLocale();
    QStringList timezoneList;
    m_currentTimeZone = m_allTimeZone[m_continentList.at(m_currentContinentIndex.row())];
    for (const QString& timezone : m_currentTimeZone) {
        timezoneList << GetLocalTimezoneName(m_continentList
            .at(m_currentContinentIndex.row()) + "/" + timezone, locale).second;
    }
    m_timeZoneModel->setStringList(timezoneList);

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

    QString timezone = m_continentList.at(m_currentContinentIndex.row()) + "/"
            + m_currentTimeZone.at(curIndex.row());
    emit timezoneUpdated(timezone);
}

void SelectTimeZoneFrame::onUpdateTimezoneList(const QString &timezone)
{
    m_timeZoneListView->setMouseTracking(true);

    const QStringList list = timezone.split("/");

    Q_ASSERT(list.count() > 1);
    m_currentContinentIndex = m_continentModel->index(m_continentList.indexOf(list.first()));
    m_continentListView->blockSignals(true);
    m_continentListView->setCurrentIndex(m_currentContinentIndex);
    m_continentListView->blockSignals(false);

    m_currentTimeZone = m_allTimeZone[list.first()];
    const QModelIndex& timezoneIndex = m_timeZoneModel->index(m_currentTimeZone.indexOf(list.last()));
    m_timeZoneListView->blockSignals(true);
    m_timeZoneListView->scrollTo(timezoneIndex, QAbstractItemView::PositionAtTop);
    m_timeZoneListView->setCurrentIndex(timezoneIndex);
    m_timeZoneListView->blockSignals(false);
}

}
