#include "ui/ncurses_widgets/ncurses_label.h"
#include "service/settings_manager.h"

namespace installer {

NcursesLabel::NcursesLabel(NCursesWindowBase *parent, const QString &text, int lines, int cols, int beginY, int beginX)
    : NCursesWindowBase (parent, lines, cols, beginY, beginX),
      m_text(text),
      m_alignment(Qt::AlignLeft)
{
    curs_set(0);
}

NcursesLabel::NcursesLabel(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : NCursesWindowBase(parent, lines, cols, beginY, beginX),
      m_alignment(Qt::AlignLeft)

{
    curs_set(0);
}

void NcursesLabel::setText(const QString &text)
{
    m_text = text;
}

void NcursesLabel::setFocus(bool foucs)
{
    if (foucs) {
        bkgd(NcursesUtil::getInstance()->button_key_active_attr());
    } else {
        //attron(NcursesUtil::getInstance()->item_attr());
        setBackground(this->background());
    }
    NCursesWindowBase::setFocus(foucs);
}

void NcursesLabel::show()
{
    this->erase();
    if (m_isBold) {
        attron(A_UNDERLINE);
    }
    switch (m_alignment) {
    case Qt::AlignLeft:
        addstr(m_text.toUtf8().data());
        break;
    case Qt::AlignCenter:
        if (width() > m_text.length()) {
            addstr(0, (width() - m_text.length()) / 2, m_text.toUtf8().data());

        } else {
            addstr(m_text.toUtf8().data());
        }
        break;
    default:
        addstr(m_text.toUtf8().data());
    }
    if (m_isBold) {
        attroff(A_UNDERLINE);
    }

    NCursesWindowBase::show();
}

void NcursesLabel::hide()
{
    NCursesWindowBase::hide();
    m_isShow = false;
}

void NcursesLabel::setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
}

void NcursesLabel::adjustSizeByContext()
{
    int maxLength = 0;
    QString chineseStr_zh = "";
    QString chineseStr_en = "";
    int nCount = m_text.count();
    for(int i = 0 ; i < nCount ; i++) {
        QChar cha = m_text.at(i);
        ushort uni = cha.unicode();
        if((uni >= 0x4E00 && uni <= 0x9FA5)
                || (uni >= 0x3130 && uni <= 0x318F) || (uni >= 0xAC00 && uni <= 0xD7A3)
                || (cha == "（") || (cha == "）") || (cha == "，") || (cha == "。")|| (cha == "：")|| (cha == "；")|| (cha == "“") || (cha == "”") || (cha == "《") || (cha == "》") || (cha == "【") || (cha == "】") || (cha == "、")) {
            chineseStr_zh.append(uni);
        } else {
            chineseStr_en.append(uni);
        }
    }

    int testlength = chineseStr_en.length() + chineseStr_zh.length() * 2;
    if (testlength > maxLength) {
        maxLength = testlength;
    }

    resize(height(), testlength);
}

void NcursesLabel::drawFoucs()
{

}

QString NcursesLabel::text() const
{
    return m_text;
}

}
