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
#include <QCollator>
#include <QDebug>

namespace installer {

namespace {
    int kContinentListViewWidth = 240;
    int kTimeZoneListViewWidth = 450;
}

SelectTimeZoneFrame::SelectTimeZoneFrame(QWidget *parent)
    : QFrame (parent)
    , m_allTimeZone(GetContinentZoneInfo())
    , m_currentContinentIndex(QModelIndex())
    , m_currentTimezoneIndex(QModelIndex())
{
    initUI();
    initConnections();
}

void SelectTimeZoneFrame::updateContinentModelData()
{
    const QString& locale = ReadLocale();
    QCollator collator(locale);
    QString left;
    QString right;
    QString continent;
    QMap<QString, QString>::const_iterator internationIt;

    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(m_allTimeZone.begin(), m_allTimeZone.end()
        , [&](const QPair<QString, QStringList>& a, const QPair<QString, QStringList>& b) -> bool {
            if((internationIt = m_mapEnglishToInternation.find(a.first)) == m_mapEnglishToInternation.end()){
                Q_ASSERT(a.second.count() > 0);
                left = GetLocalTimezoneName(a.first + "/" + a.second.first(), locale).first;
                m_mapEnglishToInternation[a.first] = left;
            }
            else {
                left = internationIt.value();
            }

            if((internationIt = m_mapEnglishToInternation.find(b.first)) == m_mapEnglishToInternation.end()){
                Q_ASSERT(b.second.count() > 0);
                right = GetLocalTimezoneName(b.first + "/" + b.second.first(), locale).first;
                m_mapEnglishToInternation[b.first] = right;
            }
            else {
                right = internationIt.value();
            }

            return collator.compare(left, right) < 0;
        }
    );

    for (auto& it : m_allTimeZone) {
        std::sort(it.second.begin(), it.second.end()
            , [&](const QString& a, const QString& b) -> bool {
            if((internationIt = m_mapEnglishToInternation.find(a)) == m_mapEnglishToInternation.end()){
                left = GetLocalTimezoneName(it.first + "/" + a, locale).second;
                m_mapEnglishToInternation[a] = left;
            }
            else {
                left = internationIt.value();
            }

            if((internationIt = m_mapEnglishToInternation.find(b)) == m_mapEnglishToInternation.end()){
                right = GetLocalTimezoneName(it.first + "/" + b, locale).second;
                m_mapEnglishToInternation[b] = right;
            }
            else {
                right = internationIt.value();
            }

            return collator.compare(left, right) < 0;
            }
        );
    }

    // backup current continent name
    if(m_currentContinentIndex.isValid()){
        Q_ASSERT(m_currentContinentList.count() > 0);
        continent = m_currentContinentList.at(m_currentContinentIndex.row());
    }

    QStringList strList;
    for (auto it : m_allTimeZone) {
        m_currentContinentList << it.first;
        Q_ASSERT(it.second.count() > 0);
        strList << m_mapEnglishToInternation[it.first];
    }
    m_continentModel->setStringList(strList);

    // after continent list sort, update current continent index in list
    if(m_currentContinentIndex.isValid()){
        m_currentContinentIndex = m_continentModel->index(m_currentContinentList.indexOf(continent));
    }
}

void SelectTimeZoneFrame::updateTimezoneModelData()
{
    if (!m_currentContinentIndex.isValid()){
        return;
    }

    const QString& locale = ReadLocale();
    QStringList timezoneList;
    QString continent = m_currentContinentList.at(m_currentContinentIndex.row());
    QString timezone;

    // backup current timezone name
    if(m_currentTimezoneIndex.isValid()){
        timezone = m_currentTimeZone.at(m_currentTimezoneIndex.row());
    }

    for (auto it : m_allTimeZone) {
        if(it.first == continent){
            m_currentTimeZone = it.second;
            break;
        }
    }
    for (const QString& timezone : m_currentTimeZone) {
        timezoneList << m_mapEnglishToInternation[timezone];
    }
    m_timeZoneModel->setStringList(timezoneList);

    // after timezone list sort, update current continent index in list
    if(m_currentTimezoneIndex.isValid()){
        m_currentTimezoneIndex = m_timeZoneModel->index(m_currentTimeZone.indexOf(timezone));
    }
}

void SelectTimeZoneFrame::initUI()
{
    m_continentListView = new DiskInstallationView;
    m_continentListView->setFixedWidth(kContinentListViewWidth);
    m_continentListView->setStyleSheet(ReadFile(":/styles/select_time_zone_frame.css"));
    m_continentListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_continentModel = new ContinentModel;
    m_continentListView->setModel(m_continentModel);
    DiskInstallationItemDelegate* continentDelegate = new DiskInstallationItemDelegate;
    m_continentListView->setItemDelegate(continentDelegate);

    m_timeZoneListView = new DiskInstallationView;
    m_timeZoneListView->setFixedWidth(kTimeZoneListViewWidth);
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

    updateContinentModelData();
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
    updateTimezoneModelData();

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

    m_currentTimezoneIndex = curIndex;
    QString timezone = m_currentContinentList.at(m_currentContinentIndex.row()) + "/"
            + m_currentTimeZone.at(curIndex.row());
    emit timezoneUpdated(timezone);
}

void SelectTimeZoneFrame::onUpdateTimezoneList(const QString &timezone)
{
    if (isVisible()) {
        m_timeZoneListView->setMouseTracking(true);
    }

    const QStringList list = timezone.split("/");

    Q_ASSERT(list.count() > 1);
    m_currentContinentIndex = m_continentModel->index(m_currentContinentList.indexOf(list.first()));
    m_continentListView->blockSignals(true);
    m_continentListView->setCurrentIndex(m_currentContinentIndex);
    m_continentListView->blockSignals(false);

    for (auto it : m_allTimeZone) {
        if(it.first == list.first()){
            m_currentTimeZone = it.second;
            break;
        }
    }
    m_currentTimezoneIndex = m_timeZoneModel->index(m_currentTimeZone.indexOf(list.last()));
    m_timeZoneListView->blockSignals(true);
    m_timeZoneListView->scrollTo(m_currentTimezoneIndex, QAbstractItemView::PositionAtTop);
    m_timeZoneListView->setCurrentIndex(m_currentTimezoneIndex);
    m_timeZoneListView->blockSignals(false);
}

void SelectTimeZoneFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange){
        // clear EnglishToInternation translate map first
        m_mapEnglishToInternation.clear();

        updateContinentModelData();
        if(m_currentContinentIndex.isValid()){
            m_continentListView->setCurrentIndex(m_currentContinentIndex);
        }
        updateTimezoneModelData();
        if(m_currentTimezoneIndex.isValid()){
            m_timeZoneListView->setCurrentIndex(m_currentTimezoneIndex);
        }
    }
    else {
        QFrame::changeEvent(event);
    }
}

}
