#include "select_time_zone_frame.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/nav_button.h"
#include "base/file_util.h"
#include "ui/views/disk_installation_view.h"
#include "ui/delegates/disk_installation_delegate.h"
#include "ui/delegates/frameless_list_delegate.h"
#include "ui/models/continent_model.h"
#include "sysinfo/timezone.h"
#include "service/settings_manager.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QCollator>
#include <QDebug>
#include <QEvent>
#include <DFrame>

DWIDGET_USE_NAMESPACE

namespace installer {

namespace {
    const int kContinentListViewWidth = 255;
    const int kTimeZoneListViewWidth = 255;
}

SelectTimeZoneFrame::SelectTimeZoneFrame(QWidget *parent)
    : DFrame (parent)
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
    QString continent;

    for (const QPair<QString, QStringList>& continentTimezonePair : m_allTimeZone) {
        Q_ASSERT(continentTimezonePair.second.count() > 0);
        m_mapEnglishToInternation[continentTimezonePair.first] = GetLocalTimezoneName(
            QString("%1/%2").arg(continentTimezonePair.first)
            .arg(continentTimezonePair.second.first()), locale).first;

        const QStringList& timezoneList = continentTimezonePair.second;
        for (const QString& timezone : timezoneList) {
            m_mapEnglishToInternation[timezone] = GetLocalTimezoneName(
                QString("%1/%2").arg(continentTimezonePair.first)
                .arg(timezone), locale).second;
        }
    }

    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(m_allTimeZone.begin(), m_allTimeZone.end()
        , [&](const QPair<QString, QStringList>& a, const QPair<QString, QStringList>& b) {
            return collator.compare(m_mapEnglishToInternation[a.first], m_mapEnglishToInternation[b.first]) < 0;
        }
    );

    for (auto& it : m_allTimeZone) {
        std::sort(it.second.begin(), it.second.end(), [&](const QString& a, const QString& b) {
                return collator.compare(m_mapEnglishToInternation[a], m_mapEnglishToInternation[b]) < 0;
            }
        );
    }

    // backup current continent name
    if(m_currentContinentIndex.isValid()){
        Q_ASSERT(m_currentContinentList.count() > 0);
        continent = m_currentContinentList.at(m_currentContinentIndex.row());
    }

    QStringList strList;
    m_currentContinentList.clear();

    for (auto it : m_allTimeZone) {
        m_currentContinentList << it.first;
        Q_ASSERT(it.second.count() > 0);
        strList << m_mapEnglishToInternation[it.first];
    }

    m_continentListView->selectionModel()->blockSignals(true);
    m_continentModel->setStringList(strList);
    m_continentListView->selectionModel()->blockSignals(false);

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

    QStringList timezoneList;
    QString continent = m_currentContinentList.at(m_currentContinentIndex.row());
    QString timezone;

    // backup current timezone name
    if(m_currentTimezoneIndex.isValid()){
        timezone = m_currentTimeZoneList.at(m_currentTimezoneIndex.row());
    }

    for (auto it : m_allTimeZone) {
        if(it.first == continent){
            m_currentTimeZoneList = it.second;
            break;
        }
    }

    for (const QString& tz : m_currentTimeZoneList) {
        timezoneList << m_mapEnglishToInternation[tz];
    }

    m_timeZoneListView->selectionModel()->blockSignals(true);

    m_timeZoneModel->clear();
    for (auto data : timezoneList) {
        DStandardItem* item = new DStandardItem(data);
        m_timeZoneModel->appendRow(item);
    }

    m_timeZoneListView->selectionModel()->blockSignals(false);

    // after timezone list sort, update current continent index in list
    if(m_currentTimezoneIndex.isValid()){
        m_currentTimezoneIndex = m_timeZoneModel->index(m_currentTimeZoneList.indexOf(timezone), 0);
    }
}

void SelectTimeZoneFrame::initUI()
{
    m_continentListView = new DListView;
    m_continentListView->setMinimumWidth(kContinentListViewWidth);
    m_continentListView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    m_continentListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_continentListView->setContextMenuPolicy(Qt::NoContextMenu);
    m_continentListView->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_continentListView->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_continentListView->setEditTriggers(QListView::NoEditTriggers);

    m_continentModel = new ContinentModel;
    m_continentListView->setModel(m_continentModel);

    m_timeZoneListView = new DListView;

    m_timeZoneListView->setFocusPolicy(Qt::TabFocus);
    m_timeZoneListView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_timeZoneListView->setEditTriggers(QListView::NoEditTriggers);
    m_timeZoneListView->setIconSize(QSize(32, 32));
    m_timeZoneListView->setResizeMode(QListView::Adjust);
    m_timeZoneListView->setMovement(QListView::Static);
    m_timeZoneListView->setSelectionMode(QListView::NoSelection);
    m_timeZoneListView->setFrameShape(QFrame::NoFrame);
    m_timeZoneListView->setMinimumWidth(kTimeZoneListViewWidth);
    m_timeZoneListView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    m_timeZoneListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_timeZoneListView->setContextMenuPolicy(Qt::NoContextMenu);
    m_timeZoneListView->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_timeZoneListView->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    m_timeZoneModel = new QStandardItemModel;
    m_timeZoneListView->setModel(m_timeZoneModel);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(10, 10, 10, 0);
    leftLayout->setSpacing(0);
    leftLayout->addWidget(m_continentListView);
    QFrame *leftListViewWrap = new QFrame;
    leftListViewWrap->setContentsMargins(0, 0, 0, 0);
    leftListViewWrap->setLayout(leftLayout);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setContentsMargins(10, 10, 10, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(m_timeZoneListView);
    QFrame *rightListViewWrap =  new QFrame;
    rightListViewWrap->setContentsMargins(0, 0, 0, 0);
    rightListViewWrap->setLayout(rightLayout);

    DVerticalLine* dVerticalLine = new DVerticalLine;
    QHBoxLayout* hLineBoxLayout = new QHBoxLayout;
    hLineBoxLayout->addWidget(dVerticalLine);

    QHBoxLayout* listViewLayout = new QHBoxLayout;
    listViewLayout->setContentsMargins(0, 0, 0, 0);
    listViewLayout->setSpacing(0);
    listViewLayout->addStretch();
    listViewLayout->addWidget(leftListViewWrap, 0, Qt::AlignLeft);
    listViewLayout->addLayout(hLineBoxLayout);
    listViewLayout->addWidget(rightListViewWrap, 0, Qt::AlignRight);
    listViewLayout->addStretch();

    DFrame *frame = new DFrame;
    frame->setFrameRounded(true);
    frame->setContentsMargins(1, 1, 1, 1);
    frame->setLayout(listViewLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addStretch();
    mainLayout->addWidget(frame);
    mainLayout->addStretch();

    setFrameRounded(false);
    setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    updateContinentModelData();
}

void SelectTimeZoneFrame::initConnections()
{
    connect(m_continentListView->selectionModel(), &QItemSelectionModel::currentChanged
            , this, &SelectTimeZoneFrame::onContinentViewSelectedChanged);
    connect(m_timeZoneListView->selectionModel(), &QItemSelectionModel::currentChanged
            , this, &SelectTimeZoneFrame::onTimeZoneViewSelectedChanged);
}

void SelectTimeZoneFrame::setSelectItem(QModelIndex index)
{
    if (!index.isValid()) {
        qCritical() << "Invalid model index";
        return;
    }

    if (index.row() >= m_timeZoneModel->rowCount()) {
        qCritical() << "Index row isn't less than model count";
        return;
    }

    if (m_oldTimezoneIndex.isValid())
    {
        DStandardItem *tmpItem = dynamic_cast<DStandardItem*>(m_timeZoneModel->item(m_oldTimezoneIndex.row()));
        if (tmpItem != nullptr) {
            tmpItem->setCheckState(Qt::Unchecked);
        }
    }

    m_oldTimezoneIndex = index;

    DStandardItem *lastItem = dynamic_cast<DStandardItem*>(m_timeZoneModel->item(index.row()));
    if (lastItem != nullptr) {
        lastItem->setCheckState(Qt::Checked);
    }
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

    QStringList timezoneEnglishList;
    for (auto it : m_allTimeZone) {
        if(it.first == m_currentContinentList.at(curIndex.row())){
            timezoneEnglishList = it.second;
            break;
        }
    }

    QStringList timezoneList;
    for (const QString& timezone : timezoneEnglishList) {
        timezoneList << m_mapEnglishToInternation[timezone];
    }

    m_timeZoneListView->selectionModel()->blockSignals(true);

    m_timeZoneModel->clear();
    for(auto data : timezoneList) {
        DStandardItem* item = new DStandardItem(data);
        m_timeZoneModel->appendRow(item);
    }

    m_timeZoneListView->selectionModel()->blockSignals(false);

    if(curIndex == m_currentContinentIndex){
        m_timeZoneListView->selectionModel()->blockSignals(true);
        setSelectItem(m_currentTimezoneIndex);
        m_timeZoneListView->setCurrentIndex(
                    m_timeZoneModel->index(m_currentTimezoneIndex.row(), 0));
        m_timeZoneListView->selectionModel()->blockSignals(false);
        m_timeZoneListView->scrollTo(m_timeZoneModel->index(m_currentTimezoneIndex.row(), 0)
                                     , QAbstractItemView::PositionAtTop);
    }
    else{
        m_timeZoneListView->scrollToTop();
    }
}

void SelectTimeZoneFrame::onTimeZoneViewSelectedChanged(QModelIndex curIndex, QModelIndex preIndex)
{
    Q_UNUSED(preIndex);

    if(!curIndex.isValid()){
        return;
    }

    if(!(curIndex.row() < m_timeZoneModel->rowCount())){
        return;
    }

    setSelectItem(curIndex);

    m_currentContinentIndex = m_continentListView->currentIndex();

    for (auto it : m_allTimeZone) {
        if(it.first == m_currentContinentList.at(m_currentContinentIndex.row())){
            m_currentTimeZoneList = it.second;
            break;
        }
    }

    m_currentTimezoneIndex = curIndex;

    QString timezone = QString("%1/%2").arg(m_currentContinentList.at(m_currentContinentIndex.row()))
        .arg(m_currentTimeZoneList.at(curIndex.row()));
    emit timezoneUpdated(timezone);
}

void SelectTimeZoneFrame::onUpdateTimezoneList(const QString &timezone)
{
    m_mapEnglishToInternation.clear();

    updateContinentModelData();
    if(m_currentContinentIndex.isValid()){
        m_continentListView->selectionModel()->blockSignals(true);
        m_continentListView->setCurrentIndex(m_currentContinentIndex);
        m_continentListView->selectionModel()->blockSignals(false);
    }

    updateTimezoneModelData();
    if(m_currentTimezoneIndex.isValid()){
        m_timeZoneListView->selectionModel()->blockSignals(true);
        setSelectItem(m_currentTimezoneIndex);
        m_timeZoneListView->setCurrentIndex(m_currentTimezoneIndex);
        m_timeZoneListView->selectionModel()->blockSignals(false);
        m_timeZoneListView->scrollTo(m_currentTimezoneIndex, QAbstractItemView::PositionAtTop);
    }

    if (isVisible()) {
        m_timeZoneListView->setMouseTracking(true);
    }

    int index = timezone.indexOf('/');
    if (index <= 0){
        qWarning() << "invalid timezone:" << timezone;
        return;
    }

    m_currentContinentIndex = m_continentModel->index(m_currentContinentList.indexOf(timezone.left(index)));
    if (!m_currentContinentIndex.isValid()) {
        qCritical() << QString("Can't find continent %1 in continent list %2").arg(timezone.left(index))
                       .arg(m_currentContinentList.join(","));
        return;
    }

    m_continentListView->selectionModel()->blockSignals(true);
    m_continentListView->setCurrentIndex(m_currentContinentIndex);
    m_continentListView->selectionModel()->blockSignals(false);

    for (auto it : m_allTimeZone) {
        if(it.first == timezone.left(index)){
            m_currentTimeZoneList = it.second;
            break;
        }
    }

    QStringList timezoneList;
    for (const QString& tz : m_currentTimeZoneList) {
        timezoneList << m_mapEnglishToInternation[tz];
    }

    m_timeZoneListView->selectionModel()->blockSignals(true);

    m_timeZoneModel->clear();
    for (auto data : timezoneList) {
        DStandardItem* item = new DStandardItem(data);
        m_timeZoneModel->appendRow(item);
    }

    m_currentTimezoneIndex = m_timeZoneModel->index(
                m_currentTimeZoneList.indexOf(timezone.mid(index + 1)), 0);
    if (!m_currentTimezoneIndex.isValid()) {
        qCritical() << QString("Can't find timezone %1 in timezone list %2").arg(timezone.mid(index + 1))
                       .arg(m_currentTimeZoneList.join(","));
        return;
    }

    setSelectItem(m_currentTimezoneIndex);
    m_timeZoneListView->setCurrentIndex(m_currentTimezoneIndex);
    m_timeZoneListView->selectionModel()->blockSignals(false);
    m_timeZoneListView->scrollTo(m_currentTimezoneIndex, QAbstractItemView::PositionAtTop);
}

}
