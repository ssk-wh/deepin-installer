#include "time_zone_frame.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include <QCollator>


namespace installer {



void TimeZoneFramePrivate::initUI()
{
    FrameInterfacePrivate::initUI();
    m_title = QObject::tr("select timezone");
    setTitle(m_title);

    m_instructions = new NcursesLabel(this, 1, width() - 4, begy() + 1, begx() + 1);
    m_instructions->setText(tr("Plese select timezone"));
    m_instructions->setFocusEnabled(false);
    m_instructions->setAlignment(Qt::AlignCenter);

    int listViewH = height() - 10;
    int listviewW = 30;
    m_instructions->setFocusEnabled(false);

    m_continentView = new NcursesListView(this, listViewH, listviewW, begy() + m_instructions->height() + 2,  begx() + (width() - listviewW * 2) / 2);
    m_continentView->setFocus(true);

    m_timeZoneView = new NcursesListView(this, listViewH, listviewW, begy() + m_instructions->height() + 2, begx() + (width() - listviewW * 2) / 2 + listviewW);



}

void TimeZoneFramePrivate::updateTs()
{
    box(ACS_VLINE, ACS_HLINE);
    m_title = QObject::tr("select timezone");
    setTitle(m_title);
    m_instructions->setText(tr("Plese select timezone"));

    FrameInterfacePrivate::updateTs();
}

bool TimeZoneFramePrivate::validate()
{
    return true;
}

void TimeZoneFramePrivate::initConnection()
{
    connect(m_continentView, &NcursesListView::selectChanged, this, [=](int index){
       m_currentContinentIndex = index;
       m_currentTimezoneIndex = 0;
       emit continentChanged(index);
    });
    connect(m_timeZoneView, &NcursesListView::selectChanged, this, [=](int index){
       m_currentTimezoneIndex = index;
       emit timezoneChanged(index);
    });
}


TimeZoneFrame::TimeZoneFrame(FrameInterface* parent) :
    FrameInterface (parent),
    m_alias_map(GetTimezoneAliasMap())
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new TimeZoneFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
}

TimeZoneFrame::~TimeZoneFrame()
{

}


bool TimeZoneFrame::init()
{
    readConf();
    if (m_currState == FRAME_STATE_NOT_START) {

        connect(dynamic_cast<TimeZoneFramePrivate*>(m_private), &TimeZoneFramePrivate::continentChanged, this, [=]{
            updateTimezoneData();
            Q_D(TimeZoneFrame);
            d->m_timeZoneView->show();
        });

        connect(dynamic_cast<TimeZoneFramePrivate*>(m_private), &TimeZoneFramePrivate::timezoneChanged, this, [=]{
            Q_D(TimeZoneFrame);
            m_timezone = m_currentTimeZoneList.at(d->m_currentTimezoneIndex);
            qDebug() << m_timezone;
        });

        m_currState = FRAME_STATE_RUNNING;
    }

    Q_D(TimeZoneFrame);
    d->updateTs();
    m_private->show();

    return true;
}

QString TimeZoneFrame::getFrameName()
{
    return "LanguageFrame";
}

void TimeZoneFrame::readConf()
{
//    m_timezone = GetSettingsString(kTimezoneDefault);
//    m_timezone = parseTimezoneAlias(m_timezone);
    m_allTimeZone = GetContinentZoneInfo();

    QString locale = ReadLocale();
    updateContinentData(locale);
    updateTimezoneData();
}

void TimeZoneFrame::writeConf()
{
    WriteTimezone(m_timezone);
}

void TimeZoneFrame::updateContinentData(QString &locale)
{
    QCollator collator(locale);
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

    QStringList strList;
    m_currentContinentList.clear();

    for (auto it : m_allTimeZone) {
        m_currentContinentList << it.first;
        Q_ASSERT(it.second.count() > 0);
        strList << m_mapEnglishToInternation[it.first];
    }

    Q_D(TimeZoneFrame);
    d->m_continentView->setList(strList);
    d->m_continentView->setCurrentIndex(d->m_currentContinentIndex);
}


void TimeZoneFrame::updateTimezoneData()
{
    Q_D(TimeZoneFrame);

    QStringList timezoneList;
    QString continent = m_currentContinentList.at(d->m_currentContinentIndex);

    m_currentTimeZoneList.clear();

    for (auto it : m_allTimeZone) {
        if(it.first == continent){
            m_currentTimeZoneList = it.second;
            break;
        }
    }

    for (const QString& tz : m_currentTimeZoneList) {
        timezoneList << m_mapEnglishToInternation[tz];
    }

    d->m_timeZoneView->setList(timezoneList);
    d->m_timeZoneView->setCurrentIndex(d->m_currentTimezoneIndex);
}

QString TimeZoneFrame::parseTimezoneAlias(const QString &timezone)
{
    return m_alias_map.value(timezone, timezone);
}
bool TimeZoneFrame::handle()
{

    m_private->keyHandle();
    writeConf();
    return true;}

}
