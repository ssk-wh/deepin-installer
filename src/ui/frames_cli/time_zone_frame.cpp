#include "time_zone_frame.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include <QCollator>


namespace installer {

const char kCliContinentDefault[] = "Asia";
const char kCliTimezoneDefault[] = "Shanghai";

TimeZoneFramePrivate::TimeZoneFramePrivate(TimeZoneFrame *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(nullptr, lines, cols, beginY, beginX),
      q_ptr(qobject_cast<TimeZoneFrame*>(parent)),
      m_currentContinentIndex(0),
      m_currentTimezoneIndex(0)
{
    initUI();
    initConnection();
}

void TimeZoneFramePrivate::initUI()
{
    FrameInterfacePrivate::initUI();
    m_title = ::QObject::tr("Select Timezone");
    setTitle(m_title);

    m_instructions = new NcursesLabel(this, 1, width() - 4, begy() + 1, begx() + 1);
    m_instructions->setText(::QObject::tr("Is it the right timezone? You can change it as well."));
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
    Q_Q(TimeZoneFrame);

    box(ACS_VLINE, ACS_HLINE);
    m_title = ::QObject::tr("Select Timezone");
    setTitle(m_title);
    m_instructions->setText(::QObject::tr("Is it the right timezone? You can change it as well."));

    FrameInterfacePrivate::updateTs();
}

bool TimeZoneFramePrivate::validate()
{
    Q_Q(TimeZoneFrame);

    q->writeConf();

    return FrameInterfacePrivate::validate();
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
    m_alias_map(GetTimezoneAliasMap()),
    m_localeString("")
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new TimeZoneFramePrivate (this, h, w, beginY, beginX);
}

TimeZoneFrame::~TimeZoneFrame()
{

}


bool TimeZoneFrame::init()
{
    if (m_localeString.compare(installer::ReadLocale())) {
        readConf();
        m_localeString = installer::ReadLocale();
    }

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
    return "TimeZoneFrame";
}

void TimeZoneFrame::readConf()
{
    Q_D(TimeZoneFrame);
//    m_timezone = GetSettingsString(kTimezoneDefault);
//    m_timezone = parseTimezoneAlias(m_timezone);
    m_allTimeZone = GetContinentZoneInfo();

    QString locale = ReadLocale();
    updateContinentData(locale);

    d->m_currentContinentIndex = m_currentContinentList.indexOf(kCliContinentDefault);
    if (d->m_currentContinentIndex == -1) {
        d->m_currentContinentIndex = 0;
    }
    d->m_continentView->setCurrentIndex(d->m_currentContinentIndex);

    updateTimezoneData();

    d->m_currentTimezoneIndex = m_currentTimeZoneList.indexOf(kCliTimezoneDefault);
    if (d->m_currentTimezoneIndex == -1) {
        d->m_currentTimezoneIndex = 0;
    }
    d->m_timeZoneView->setCurrentIndex(d->m_currentTimezoneIndex);
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
    return true;}
}
