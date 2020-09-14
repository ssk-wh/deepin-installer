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
//    scrollok(true);
//    setscrreg(0, lines);
    curs_set(0);
}

void NcursesTextBrower::setText(const QString &text)
{
    m_text.clear();
    appendItemText(text);
}

void NcursesTextBrower::appendText(const QString &text)
{
    if(text.isEmpty())
    {
        return;
    }

    int lineCharNum = width();

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
    appendItemText(teststr);
}

void NcursesTextBrower::appendItemText(const QString &text)
{
    QString input = text;

#ifdef QT_DEBUG_test
     input = "";
     iswchar = true;
#endif // QT_DEBUG

    int lineCharNum = lineCharNum = width();

    if (!input.isEmpty()) {

        QString tmp = "";
        QString chineseStr_zh = "";
        QString chineseStr_en = "";

        for (int i = 0; i < text.length();) {
            QChar cha = input.at(i);
            ushort uni = cha.unicode();
            if ((uni >= 0x4E00 && uni <= 0x9FA5)
                    || (uni >= 0x3130 && uni <= 0x318F) || (uni >= 0xAC00 && uni <= 0xD7A3)
                    || (cha == "（") || (cha == "）") || (cha == "，") || (cha == "。")|| (cha == "：")|| (cha == "；")|| (cha == "“") || (cha == "”") || (cha == "《") || (cha == "》") || (cha == "【") || (cha == "】") || (cha == "、") || (cha == "●")) {
                chineseStr_zh.append(uni);
            } else {
                chineseStr_en.append(uni);
            }

            int maxLength = chineseStr_en.length() + chineseStr_zh.length() * 2;

            if (maxLength >= lineCharNum) {
                m_text.push_back(tmp);
                tmp = "";
                chineseStr_zh = "";
                chineseStr_en = "";
            } else {
                if (input.at(i) == "\n") {
                    m_text.push_back(tmp);
                    tmp = "";
                    chineseStr_zh = "";
                    chineseStr_en = "";
                    i++;
                } else {
                    tmp += input.at(i);
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
