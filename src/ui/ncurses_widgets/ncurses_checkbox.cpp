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
      m_isshow(false)
{
    if(parent != nullptr) {
        this->setBackground(parent->background());
    }
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
      m_isshow(false)
{
    if(parent != nullptr) {
        this->setBackground(parent->background());
    }
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

void NcursesCheckBox::setText(const QString &title, const QString &text, bool iswchar)
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

    int browerwidthtest = testbrowerwidth;
    if(iswchar) {
        browerwidthtest = browerwidthtest / 2;
    }

    if(m_isusetitle) {
        int testlines_1 = title.length() / browerwidthtest;
        if((title.length() % browerwidthtest) > 0){
            testlines_1++;
        }

        int testlines_2 = text.length() / browerwidthtest;
        if((text.length() % browerwidthtest) > 0) {
            testlines_2++;
        }
        m_strheight = testlines_1 + testlines_2;
        wresize(m_strheight, width());

        m_titlebrower   = new NcursesTextBrower(this, testlines_1, testbrowerwidth, begy(), begx() + width() - testbrowerwidth);
        m_titlebrower->setText(title, iswchar);
        m_titlebrower->show();
        m_titlebrower->refresh();

        m_contentbrower = new NcursesTextBrower(this, testlines_2, testbrowerwidth, begy() + testlines_1, begx() + width() - testbrowerwidth);
        m_contentbrower->setText(text, iswchar);
        m_contentbrower->show();
        m_contentbrower->refresh();
        m_selectshowpos = testlines_1;
    } else {
        int testlines = 0;
        testlines = text.length() / testbrowerwidth;
        if((text.length() % browerwidthtest) > 0) {
            testlines++;
        }
        m_strheight = testlines;
        wresize(m_strheight, width());

        m_contentbrower = new NcursesTextBrower(this, testlines, testbrowerwidth, begy(), begx() + width() - testbrowerwidth);
        m_contentbrower->setText(text, iswchar);
        m_contentbrower->show();
        m_contentbrower->refresh();
        m_selectshowpos = 0;
    }
    addstr(m_selectshowpos, 0, testframestr.toUtf8().data());
    this->refresh();
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
    if (foucs) {
        attron(NcursesUtil::getInstance()->item_selected_attr());
    } else {
        //attrset(NcursesUtil::getInstance()->item_attr());
        setBackground(m_parent->background());
    }

    if(m_titlebrower != nullptr) {
       m_titlebrower->setFocus(foucs);
    }
    if(m_contentbrower != nullptr) {
       m_contentbrower->setFocus(foucs);
    }
    setSelect(m_select);
    NCursesWindowBase::setFocus(foucs);
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

    NCursesWindowBase::resizew(this->height(), testbrowerwidth);
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


}
