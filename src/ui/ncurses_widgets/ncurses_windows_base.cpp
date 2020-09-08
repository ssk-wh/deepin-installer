#include "ncurses_windows_base.h"
#include <algorithm>
#include "service/settings_manager.h"


namespace installer {

NCursesWindowBase::NCursesWindowBase(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, bool shadow, bool box)
    : QObject(parent),
      NCursesPanel(lines, cols, beginY, beginX),
      m_parent(parent),
      m_shabow(shadow),
      m_box(box),
      m_foucs(false),
      m_foucs_enabled(true)
{

//    Q_ASSERT(lines > 0);
//    Q_ASSERT(cols > 0);
//    Q_ASSERT(beginX >= 0);
//    Q_ASSERT(beginY >= 0);
    if(has_colors())
    {
        useColors();
    }

    curs_set(0);


    m_shadowWindow = new NCursesPanel(lines, cols, beginY + 1, beginX + 2);
    m_shadowWindow->hide();

    if (parent != nullptr) {
        parent->addChildWindows(this);
        setBackground(parent->background());
    }

    //drawShadow

}


void NCursesWindowBase::printTitle(QString title, int width)
{
    if (title.length() != 0) {
            int tlen = 0;
            if (installer::ReadLocale() == "zh_CN") {
                tlen = std::min(width - 2, title.length() * 4);
                addch(0, (width - tlen / 2) / 2 - 1, '|');
                addstr(0, (width - tlen / 2) / 2, title.toUtf8().data(), tlen);
                addch('|');
            } else {
                tlen = std::min(width - 2, title.length());
                //setAttr(NcursesUtil::getInstance()->title_attr());
                addch(0, (width - tlen) / 2 - 1, '|');
                addstr(0, (width - tlen) / 2, title.toUtf8().data(), tlen);
                addch('|');
            }
        }
}

void NCursesWindowBase::setTitle(QString title) {
    m_title = "|" + title + "|";
}


void NCursesWindowBase::drawShadow(bool isshow)
{
    /*if(isshow){
        m_shadowWindow->show();
    }else{
        m_shadowWindow->hide();
    }
    m_shadowWindow->refresh();*/
    m_isshadowWindow = isshow;
}


void NCursesWindowBase::addChildWindows(NCursesWindowBase *childWindows) {
    if (childWindows == nullptr) {
        return;
    }
    if (!m_childWindows.contains(childWindows)) {
        m_childWindows.push_back(childWindows);
        if (childWindows->m_foucs_enabled)
        {
            addFoucsWindows(childWindows);
        }
    }
}

void NCursesWindowBase::removeChildWindows(NCursesWindowBase *childWindows)
{
    if (childWindows == nullptr) {
        return;
    }
    if (m_childWindows.contains(childWindows)) {
        m_childWindows.removeOne(childWindows);
        if (childWindows->m_foucs_enabled)
        {
            removeFoucsWindows(childWindows);
        }
        disconnect(childWindows, nullptr, nullptr, nullptr);
        childWindows->deleteLater();
    }
}

void NCursesWindowBase::clearFoucs()
{
    if (m_parent != nullptr) {
        for (NCursesWindowBase *win : m_parent->m_childWindows) {
            win->updateFoucs(false);
        }
    }
}

void NCursesWindowBase::updateFoucs(bool foucs)
{
    bool old = m_foucs;
    m_foucs = foucs;

    if (old != m_foucs) {
        emit selectionChanged();
        m_foucs ? Q_EMIT inFoucs() : Q_EMIT outFoucs();
    }
}

void NCursesWindowBase::setFocus(bool foucs) {
    if (hidden()) {
        return;
    }

    if (foucs) {
        this->clearFoucs();
    }
    updateFoucs(foucs);

    this->show();
    /*if (m_parent != nullptr)
        m_parent->refresh();*/
}

void NCursesWindowBase::setFocusEnabled(bool enabled)
{
    m_foucs_enabled = enabled;
    if (m_parent != nullptr) {
        if (m_foucs_enabled) {
            m_parent->addFoucsWindows(this);
        } else {
            m_parent->removeFoucsWindows(this);
        }
    }
}

bool NCursesWindowBase::isOnFoucs() {
    return m_foucs;
}

void NCursesWindowBase::show() {

    //show shadowWindow
    if(m_isshadowWindow){
        m_shadowWindow->show();
    }

    this->drawCurs();
    this->drawFoucs();
    NCursesPanel::show();
    
    /*if (!m_title.isEmpty()) {
        label(m_title.toUtf8().data(), "");
    }*/

    foreach (NCursesWindowBase *childWindows, m_childWindows) {
            childWindows->show();  
    }

    refresh();
}

void NCursesWindowBase::hide() {
    m_shadowWindow->hide();
    NCursesPanel::hide();
//    if(m_parent != nullptr)
//        m_parent->noutrefresh();
    foreach (NCursesWindowBase *childWindows, m_childWindows) {
        childWindows->hide();
        //childWindows->refresh();
    }
//    if(m_parent != nullptr)
//        m_parent->refresh();
    refresh();
}


void NCursesWindowBase::switchChildWindowsFoucs() {

    if (m_foucsWindows.empty()) {
        return;
    }

    int index = m_foucsWindows.size();
    int i = 0;
    for (; i < index; i++) {
        if (m_foucsWindows[i]->isOnFoucs()) {
            if ((i + 1) == index) {
                m_foucsWindows[i]->setFocus(false);
                m_foucsWindows[0]->setFocus(true);
            } else {
                m_foucsWindows[i]->setFocus(false);
                m_foucsWindows[i + 1]->setFocus(true);
            }
            break;
        } else {
            if ((i + 1) == index) {
                m_foucsWindows[i]->setFocus(false);
                m_foucsWindows[0]->setFocus(true);
            }
        }
    }
}

void NCursesWindowBase::onKeyPress(int keyCode) {
    Q_UNUSED(keyCode)

    if (!this->isOnFoucs()) {
        return;
    }
    switch (keyCode) {
        case KEY_ENTER_OTHER:
            emit enter();
        break;
        case KEY_ESC:
            emit esc();
        break;
    }
}

void NCursesWindowBase::resize(int lines, int cols)
{
    this->wresize(lines, cols);
}

void NCursesWindowBase::drawFoucs()
{

}

void NCursesWindowBase::drawCurs()
{
//    curs_set(0);
}

void NCursesWindowBase::adjustSizeByContext()
{

}

void NCursesWindowBase::resizew(int newLines, int newColumns)
{
    wresize(newLines, newColumns);
    if(m_isshadowWindow) {
        m_shadowWindow->wresize(newLines, newColumns);
    }
}

void NCursesWindowBase::setIsShow(bool isshow)
{

}

void NCursesWindowBase::moveWidowTo(int y, int x)
{
    mvwin(y, x);
    if(m_isshadowWindow)
    {
        m_shadowWindow->mvwin(y+1, x+1);
    }
}

void NCursesWindowBase::setBackground(chtype box)
{
    bkgd(box);
    m_background = box;
    //this->refresh();
}

void NCursesWindowBase::setAttr(chtype attr)
{
    attrset(attr);
    m_attr = attr;
    this->refresh();
}

chtype NCursesWindowBase::background()
{
    return m_background;
}

void NCursesWindowBase::setFocusStyle(chtype type)
{
    m_chtype_focus = type;
}

void NCursesWindowBase::addFoucsWindows(NCursesWindowBase *childWindows)
{
    if (childWindows == nullptr) {
        return;
    }
    if (!m_foucsWindows.contains(childWindows)) {
        m_foucsWindows.push_back(childWindows);
    }
}

void NCursesWindowBase::removeFoucsWindows(NCursesWindowBase *childWindows)
{
    if (childWindows == nullptr) {
        return;
    }
    if (m_foucsWindows.contains(childWindows)) {
        m_foucsWindows.removeOne(childWindows);
    }
}

}

