#include <wctype.h>
#include "ui/ncurses_widgets/ncurses_text_brower.h"

namespace installer {


NcursesTextBrower::NcursesTextBrower(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : NCursesWindowBase (parent, lines, cols, beginY, beginX),
      m_currLine(0),
      m_reserveY(0),
      m_reserveX(1)

{
    bkgd(parent->getbkgd());
    scrollok(true);
    setscrreg(0, lines);
}

void NcursesTextBrower::setText(const QString &text, bool iswchar)
{
    m_text.clear();
    appendItemText(text, iswchar);
}

void NcursesTextBrower::appendText(const QString &text, bool iswchar)
{
    if(text.isEmpty())
    {
        return;
    }

    int lineCharNum = 0;
    if(iswchar)
    {
        lineCharNum = width() / 2;//一行显示的字符个数

    } else {
        lineCharNum = width();//一行显示的字符个数
    }

    int delta = 0;
    if (!m_text.isEmpty()) {
        QString last = m_text.last();
        delta = lineCharNum - last.length();
        if (delta > 0) {
            m_text.removeLast();
            last += text.mid(0, delta);
            m_text.push_back(last);
        }

    }

    QString teststr = text.right(text.length() - delta);
    appendItemText(teststr, iswchar);
}

void NcursesTextBrower::appendItemText(const QString &text, bool iswchar)
{
    QString input = text;

#ifdef QT_DEBUG_test
     input = "";
     iswchar = true;
#endif // QT_DEBUG

    int lineCharNum = 0;
    if(iswchar)
    {
        lineCharNum = width() / 2;//中文一行显示的字符个数

    } else {
        lineCharNum = width();//英文一行显示的字符个数
    }

    if (!input.isEmpty()) {

        QString tmp = "";
        for (int i = 0; i < text.length();) {

            if(tmp.length() == lineCharNum){
                m_text.push_back(tmp);
                tmp = "";
            }else {
                if(input.at(i) == "\n"){
                    m_text.push_back(tmp);
                    tmp = "";
                    i++;
                } else {
                    tmp += text.at(i);
                    i++;
                }
            }
        }

        if(tmp.compare(""))
            m_text.push_back(tmp);
    }
}

bool NcursesTextBrower::setFileName(const QString)
{
    return true;
}

void NcursesTextBrower::scrollToEnd()
{
    m_currLine = m_text.size() - (height() - 2 * m_reserveY);
    if(m_currLine < 0) {
        m_currLine = 0;
    }
    show();
}

void NcursesTextBrower::clearText()
{
    erase();
    m_currLine = 0;
    m_text.clear();
    refresh();
}

void NcursesTextBrower::setFocus(bool foucs)
{
    if (foucs) {
        attron(NcursesUtil::getInstance()->item_selected_attr());
    } else {
        setBackground(this->background());
    }

    NCursesWindowBase::setFocus(foucs);
}

void NcursesTextBrower::show()
{
    erase();
    int currY  = m_reserveY;

    for (int i = m_currLine; i < m_text.size(); i++) {
        if (currY > (height() - 2 * m_reserveY)) {
            break;
        }
        addstr(currY, 0, m_text.at(i).toUtf8().data());//
        currY++;
    }

    NCursesWindowBase::show();
}

void NcursesTextBrower::hide()
{
   m_currLine = 0;
   return NCursesWindowBase::hide();
}

void NcursesTextBrower::onKeyPress(int keyCode) {
    if (m_text.size() <= (height() - 2 * m_reserveY)) {
        return;
    }

    switch (keyCode) {
        case KEY_UP:
        if (m_currLine > 0) {
            m_currLine--;
            show();
        }
            break;
        case KEY_DOWN:
        if ((m_currLine + height() - 2 * m_reserveY) < m_text.size()) {
            m_currLine++;
            show();
        }
            break;
        default:
            break;
    }

}

}
