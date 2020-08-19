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

    m_instructions = new NcursesLabel(this, 3, 1, begy() + 2, begx() + 1);
    //m_instructions->hide();
    m_instructions->setFocusEnabled(false);

    m_languageView = new NcursesListView(this, height() - 10, 20, begy(), begx());
    //m_languageView->hide();
}

void LanguageFramePrivate::layout()
{
    try {
        m_instructions->adjustSizeByContext();
        m_instructions->mvwin(begy() + 2, begx() + 2);
        m_languageView->adjustSizeByContext();
        m_languageView->resize(height() - 10,  m_languageView->width());
        m_languageView->mvwin(begy() + m_instructions->height() + 2,  begx() + (width() - m_languageView->width()) / 2);
    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }
}

void LanguageFramePrivate::updateTs()
{
    updateText();
    layout();
}

void LanguageFramePrivate::initConnection()
{
    connect(m_languageView, &NcursesListView::selectChanged, this, [=](int index){
        m_index = index;
        writeConf();
        emit languageChange();
        updateText();
        //m_isshow = false;
        //this->show();
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
        FrameInterfacePrivate::show();
        m_isshow = true;
        m_pNextButton->setFocus(false);
        m_languageView->setFocus(true);
    }
}

void LanguageFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void LanguageFramePrivate::updateText()
{
    box(ACS_VLINE,ACS_HLINE);
    printTitle(::QObject::tr("Select Language"), width());
    m_instructions->erase();
    m_instructions->setText("  " + ::QObject::tr("Choose a language used in the installation process, which will also be the default system language."));
    m_instructions->adjustSizeByContext();
    m_instructions->show();
    FrameInterfacePrivate::updateTs();
}

QString LanguageFramePrivate::getCurrentLanguageTimezone()
{
    return m_languageList.at(m_index).timezone;
}

void LanguageFramePrivate::onKeyPress(int keyCode)
{
    switch (keyCode) {
    case KEY_TAB:
        switchChildWindowsFoucs();
        break;
    }

    qDebug()<< keyCode;
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
    int h = MAINWINDOW_HEIGHT;//LINES / 2;
    int w = MAINWINDOW_WIDTH;//COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new LanguageFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
    //m_private->hide();

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
        d->readConf();
        m_private->layout();
        m_currState = FRAME_STATE_RUNNING;
    }
    return true;
}

QString LanguageFrame::getFrameName()
{
    return "LanguageFrame";
}

QString LanguageFrame::getCurrentLanguageTimezone()
{
    Q_D(LanguageFrame);
    return d->getCurrentLanguageTimezone();
}

bool LanguageFrame::handle()
{
    return true;
}

}




