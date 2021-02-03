#ifndef CLI_INSTALL_TIME_ZONE_FRAME_H
#define CLI_INSTALL_TIME_ZONE_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "sysinfo/timezone.h"

namespace installer {

class TimeZoneFrame;

class TimeZoneFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
    friend TimeZoneFrame;
public:
    TimeZoneFramePrivate(TimeZoneFrame* parent, int lines, int cols, int beginY, int beginX);

signals:
    void continentChanged(int index);
    void timezoneChanged(int index);
    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;
    virtual void onKeyPress(int keyCode) override;

protected:
    void leftHandle() override;
    void rightHandle() override;

private:
    TimeZoneFrame *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(TimeZoneFrame)

    //QString m_title;
    NcursesLabel* m_instructions;
    NcursesListView* m_continentView;
    NcursesListView* m_timeZoneView;
    int m_currentContinentIndex;
    int m_currentTimezoneIndex;
    //QString m_localeString;
    bool m_isshow;
};

class TimeZoneFrame : public FrameInterface
{
    Q_OBJECT
public:
    TimeZoneFrame(FrameInterface* parent);
    virtual ~TimeZoneFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;
    void setDefaultTimezone(QString timezone);

    QString getAbout() override{
        return QString("<↑ ↓ ← →>%1 | <Tab>%2 | <Enter>%3")
                .arg(::QObject::tr("Select Item"))
                .arg(::QObject::tr("Change Field"))
                .arg(::QObject::tr("Confirm"))
                .arg(::QObject::tr("Select"));
    }
private:
    void readConf();
    void writeConf();
    void updateContinentData(QString& locale);
    void updateTimezoneData();
    QString parseTimezoneAlias(const QString& timezone);
protected:
    bool handle() override;
private:
    QString m_timezone;
    ContinentZoneInfoList m_allTimeZone;
    QMap<QString, QString> m_mapEnglishToInternation;
    QStringList m_currentContinentList;
    QStringList m_currentTimeZoneList;
    TimezoneAliasMap m_alias_map;
    QString m_localeString;
    QString m_kCliContinentDefault;
    QString m_kCliTimezoneDefault;
    Q_DECLARE_PRIVATE_D(m_private, TimeZoneFrame)
};


}


#endif
