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
    m_titleLabel = new NcursesLabel(this, 1, 1, begy(), begx());
    m_titleLabel->setFocusEnabled(false);

    m_instructions = new NcursesLabel(this, 3, 1, begy() + 1, begx() + 1);
    m_instructions->setFocusEnabled(false);

    m_languageView = new NcursesListView(this, height() - 10, 20, begy(), begx());
    m_languageView->setFocus(true);
    updateTs();
}

void LanguageFramePrivate::layout()
{
    try {
        m_titleLabel->adjustSizeByContext();
        m_titleLabel->mvwin(begy(), begx() + (width() - m_titleLabel->width()) / 2);
        m_instructions->adjustSizeByContext();
        m_instructions->mvwin(begy() + m_titleLabel->height(), begx() + 2);
        m_languageView->adjustSizeByContext();
        m_languageView->resize(height() - 10,  m_languageView->width());
        m_languageView->mvwin(begy() + m_instructions->height() + 2,  begx() + (width() - m_languageView->width()) / 2);
    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }
}

void LanguageFramePrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    m_titleLabel->setText(tr("select language"));
    m_instructions->erase();
    m_instructions->setText(tr("    Choose the language to be used for the installation process. \
The selected language will also be the default language for the installed system."));
    FrameInterfacePrivate::updateTs();
    layout();
}

void LanguageFramePrivate::initConnection()
{
    connect(m_languageView, &NcursesListView::selectChanged, this, [=](int index){
        m_index = index;
        writeConf();
        emit languageChange();
        updateTs();
        m_isshow = false;
        this->show();
    });

    connect(m_languageView, &NcursesListView::selectd, this, [=](int index){
        m_index = index;
        emit next();
    });

    readConf();
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
    connect(d, &LanguageFramePrivate::languageChange, this, &LanguageFrame::languageChanged);
}

LanguageFrame::~LanguageFrame()
{

}


bool LanguageFrame::init()
{
    Q_D(LanguageFrame);
    if (m_currState == FRAME_STATE_NOT_START) {
        m_private->layout();
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




