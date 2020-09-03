#include "ncurses_progressbar.h"

namespace installer {

NcursesProgressBar::NcursesProgressBar(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
    :NCursesWindowBase(parent, lines, cols, beginY, beginX),
     m_rangevalue(0),
     m_value(0),
     m_progressSlidmonitor(nullptr)
{
    this->setBackground(parent->background());
    this->box();
    m_progressSlidmonitor = new NCursesPanel(1, 1, beginY + 1, beginX + 1);
    m_progressSlidmonitor->bkgd(NcursesUtil::getInstance()->screen_attr());
    curs_set(0);
}

NcursesProgressBar::~NcursesProgressBar()
{
    if(m_progressSlidmonitor != nullptr)
    {
        delete m_progressSlidmonitor;
        m_progressSlidmonitor = nullptr;
    }
}

void NcursesProgressBar::show()
{
    NCursesWindowBase::show();
    m_progressSlidmonitor->show();
}

void NcursesProgressBar::setRange(int rangevalue)
{
    m_rangevalue = rangevalue;
}

void NcursesProgressBar::setValue(int value)
{
    if(m_rangevalue == 0)
    {
        return;
    }

    m_value = value;
    double testwidth = this->width() * 1.0;
    double testrate = m_value * 1.0 / m_rangevalue;
    int monitorlength = testwidth * testrate;
    QString testratestr = QString("%1%").arg(testrate * 100);

    m_progressSlidmonitor->wresize(1, monitorlength);
    m_progressSlidmonitor->erase();
    m_progressSlidmonitor->addstr(0, m_progressSlidmonitor->width() / 2, testratestr.toUtf8().data());
    m_progressSlidmonitor->show();
    m_progressSlidmonitor->refresh();
}

void NcursesProgressBar::reSet()
{
    if(m_progressSlidmonitor != nullptr)
    {
        delete m_progressSlidmonitor;
        m_progressSlidmonitor = nullptr;
    }

    m_progressSlidmonitor = new NCursesPanel(1, 1, begy() + 1, begx() + 1);
    m_progressSlidmonitor->bkgd(NcursesUtil::getInstance()->screen_attr());
}

void NcursesProgressBar::start()
{

}

void NcursesProgressBar::stop()
{

}


}
