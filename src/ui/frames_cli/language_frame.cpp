#include "language_frame.h"
#include "cursesp.h"
#include "cursesm.h"
#include <QScopedPointer>

#include "service/settings_manager.h"
#include "service/settings_name.h"


namespace installer {


void LanguageFramePrivate::initUI()
{
    FrameInterfacePrivate::initUI();
    setTitle(tr("select language"));

    m_instructions = new NcursesLabel(this, 2, width() - 4, begy() + 1, begx() + 1);
    m_instructions->setText(tr("    Choose the language to be used for the installation process. \
The selected language will also be the default language for the installed system."));
    m_instructions->setFocusEnabled(false);

    int languageViewH = height() - 10;
    int languageViewW = 50;


    m_languageView = new NcursesListView(this, languageViewH, languageViewW, begy() + m_instructions->height() + 2,  begx() + (width() - languageViewW) / 2);
    m_languageView->setFocus(true);
}

void LanguageFramePrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    setTitle(tr("select language"));
    m_instructions->erase();
    m_instructions->setText(tr("    Choose the language to be used for the installation process. \
The selected language will also be the default language for the installed system."));

    FrameInterfacePrivate::updateTs();
}

void LanguageFramePrivate::initConnection()
{
    connect(m_languageView, &NcursesListView::selectChanged, this, [=](int index){
        m_index = index;
    });

    connect(m_languageView, &NcursesListView::selectd, this, [=](int index){
        m_index = index;
        emit next();
    });

}

bool LanguageFramePrivate::validate()
{
    writeConf();
    emit languageChange();
    return true;
}

void LanguageFramePrivate::show()
{
    if(!m_isshow) {
        NCursesWindowBase::show();
        m_isshow = true;
        m_pBackButton->hide();
        m_pBackButton->setFocusEnabled(false);
    }
}

void LanguageFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void LanguageFramePrivate::readConf()
{
    //Q_D(LanguageFrame);
    const QString di_locale = GetSettingsString("DI_LOCALE");
    const QString default_locale = GetSettingsString(kSelectLanguageDefaultLocale);
    const QString locale = di_locale.isEmpty() ? default_locale : di_locale;

    m_languageList = GetLanguageList();
    QStringList list;
    int index = 0;

    int maxLength = 0;
    foreach (LanguageItem item, m_languageList) {
        if (item.name.length() > maxLength) {
            maxLength = item.name.length();
        }
    }


    for (int i = 0; i < m_languageList.size(); i++) {
        QString name = m_languageList.at(i).name;
        QString tmp;
        for (int j = 0; j < maxLength - name.length(); j++) {
            tmp += " ";
        }
        name += tmp;
        list.push_back(QString("%1      -%2").arg(name).arg(m_languageList.at(i).local_name));
        if (locale == m_languageList.at(i).locale) {
            index = i;
        }
    }

    m_languageView->setList(list);

    if (index >= 0) {
        m_index = index;
        m_languageView->setCurrentIndex(index);
    }
}

void LanguageFramePrivate::writeConf()
{
    //Q_D(LanguageFrame);
    WriteLocale(m_languageList.at(m_index).locale);
}


LanguageFrame::LanguageFrame(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new LanguageFramePrivate (parent->getPrivate(), h, w, beginY, beginX);

    Q_D(LanguageFrame);
    connect(d, &LanguageFramePrivate::languageChange, this, &LanguageFrame::languageChangeed);
}

LanguageFrame::~LanguageFrame()
{

}


bool LanguageFrame::init()
{
    Q_D(LanguageFrame);
    if (m_currState == FRAME_STATE_NOT_START) {
        d->readConf();
        m_currState = FRAME_STATE_RUNNING;
    }
    return true;
}

QString LanguageFrame::getFrameName()
{
    return "LanguageFrame";
}

bool LanguageFrame::handle()
{
    return true;
}

}




