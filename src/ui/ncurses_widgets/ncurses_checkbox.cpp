#include "ncurses_checkbox.h"
#include "ncurses_text_brower.h"

namespace installer {

NcursesCheckBox::NcursesCheckBox(NCursesWindowBase *parent, const QString &text, int lines, int cols, int beginY, int beginX)
    :NCursesWindowBase(parent, lines, cols, beginY, beginX),
      m_text(text),
      m_select(false),
      m_enalble(true),
      m_isusetitle(false),
      m_titlebrower(nullptr),
      m_contentbrower(nullptr),
      m_selectshowpos(0),
      m_strheight(0),
      m_isshow(false),
      m_isTitleUseCheckbox(false)
{
    if(parent != nullptr) {
        this->setBackground(parent->background());
    }
    curs_set(0);
}

NcursesCheckBox::NcursesCheckBox(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
    :NCursesWindowBase(parent, lines, cols, beginY, beginX),
      m_title(""),
      m_text(""),
      m_select(false),
      m_enalble(true),
      m_isusetitle(false),
      m_titlebrower(nullptr),
      m_contentbrower(nullptr),
      m_selectshowpos(0),
      m_strheight(0),
      m_isshow(false),
      m_isTitleUseCheckbox(false)
{
    if(parent != nullptr) {
        this->setBackground(parent->background());
    }
    curs_set(0);
}

NcursesCheckBox::~NcursesCheckBox()
{
    clearText();
}

void NcursesCheckBox::show()
{
    if (!m_isshow) {
        NCursesWindowBase::show();
        m_isshow = true;
    }
}

void NcursesCheckBox::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void NcursesCheckBox::setText(const QString &title, const QString &text)
{
    m_title = title;
    m_text = text;

    clearText();
    erase();

    QString testframestr = "";
    if(m_select) {
        testframestr = "[*] ";
    } else {
        testframestr = "[ ] ";
    }
    int testbrowerwidth = width() - testframestr.length();

    if(m_isusetitle) {
        m_titlebrower   = new NcursesTextBrower(this, 1, testbrowerwidth, begy(), begx() + width() - testbrowerwidth);
        m_titlebrower->setText(title);
        int testlines_1 = m_titlebrower->getTextHeight();
        m_titlebrower->resize(testlines_1, testbrowerwidth);

        m_contentbrower = new NcursesTextBrower(this, 1, testbrowerwidth, begy() + testlines_1, begx() + width() - testbrowerwidth);
        m_contentbrower->setText(text);
        int testlines_2 = m_contentbrower->getTextHeight();
        m_contentbrower->resize(testlines_2, testbrowerwidth);

        m_strheight = testlines_1 + testlines_2;
        resize(m_strheight, width());

        if (m_isTitleUseCheckbox) {
            m_selectshowpos = 0;
        } else {
            m_selectshowpos = testlines_1;
        }
    } else {
        m_contentbrower = new NcursesTextBrower(this, 1, testbrowerwidth, begy(), begx() + width() - testbrowerwidth);
        m_contentbrower->setText(text);

        int testlines = m_contentbrower->getTextHeight();
        m_strheight = testlines;
        m_contentbrower->resize(m_strheight, testbrowerwidth);
        resize(m_strheight, width());

        m_selectshowpos = 0;
    }
    addstr(m_selectshowpos, 0, testframestr.toUtf8().data());
}

void NcursesCheckBox::setSelect(bool select)
{
    m_select = select;
    QString testframestr = "";
    if(m_select) {
        testframestr = "[*] ";
    } else {
        testframestr = "[ ] ";
    }
    addstr(m_selectshowpos, 0, testframestr.toUtf8().data());

    refresh();
}

void NcursesCheckBox::setFocus(bool foucs) {

    //NCursesWindowBase::setFocus(foucs);
    m_foucs = foucs;

    if (foucs) {
        attron(NcursesUtil::getInstance()->item_selected_attr());
    } else {
        setBackground(m_parent->background());
    }

    if(m_titlebrower != nullptr) {
       m_titlebrower->setFocus(foucs);
    }
    if(m_contentbrower != nullptr) {
       m_contentbrower->setFocus(foucs);
    }

    setSelect(m_select);
}

void NcursesCheckBox::resizew(int newLines, int newColumns)
{
    QString testframestr = "";
    if(m_select) {
        testframestr = "[*] ";
    } else {
        testframestr = "[ ] ";
    }
    int testbrowerwidth = newColumns + testframestr.length();

    if(m_titlebrower != nullptr) {
        m_titlebrower->resize(m_titlebrower->height(), testbrowerwidth);
        if(m_contentbrower != nullptr) {
            m_contentbrower->resize(m_contentbrower->height(), testbrowerwidth);
        }
    } else if(m_contentbrower != nullptr) {
        m_contentbrower->resize(m_contentbrower->height(), testbrowerwidth);
    }

    wresize(this->height(), testbrowerwidth);
    NCursesWindowBase::show();
}

void NcursesCheckBox::setEnable(bool enable)
{

}

void NcursesCheckBox::onKeyPress(int keyCode)
{
    switch (keyCode) {
        case 32:{
            if(m_select){
                setSelect(false);
                emit signal_SelectChange(false);
            } else {
                setSelect(true);
                emit signal_SelectChange(true);
            }

        }
            break;
        default:
            break;
    }

    NCursesWindowBase::onKeyPress(keyCode);
}

void NcursesCheckBox::clearText()
{
    if(m_titlebrower != nullptr) {
        delete m_titlebrower;
        m_titlebrower = nullptr;
    }

    if(m_contentbrower != nullptr) {
        delete m_contentbrower;
        m_contentbrower = nullptr;
    }

    m_childWindows.clear();
    m_foucsWindows.clear();
}

void NcursesCheckBox::moveWindowTo(int y, int x)
{
    mvwin(y, x);

    QString testframestr = "[*] ";
    if(m_titlebrower != nullptr) {
        m_titlebrower->mvwin(y, x + testframestr.length());
        if(m_contentbrower != nullptr) {
            m_contentbrower->mvwin(y + 1, x + testframestr.length());
        }
    } else if(m_contentbrower != nullptr) {
        m_contentbrower->mvwin(y, x + testframestr.length());
    }
}

void NcursesCheckBox::setbkgd(chtype colortype)
{
    attron(colortype);

    if(m_titlebrower != nullptr) {
       m_titlebrower->attron(colortype);//bkgd(colortype);
       m_titlebrower->show();
    }
    if(m_contentbrower != nullptr) {
       m_contentbrower->attron(colortype);//bkgd(colortype);
       m_contentbrower->show();
    }

    refresh();
}

void NcursesCheckBox::setFocusStyle(chtype type)
{
    m_chtype_focus = type;
    if(m_titlebrower != nullptr) {
       m_titlebrower->setFocusStyle(type);
    }
    if(m_contentbrower != nullptr) {
       m_contentbrower->setFocusStyle(type);
    }

}


}
