#include "ncurses_button.h"
#include "service/settings_manager.h"

namespace installer {



NcursesButton::NcursesButton(NCursesWindowBase* parent, QString text, int lines, int cols, int beginY, int beginX, bool box, bool shadow)
    : NCursesWindowBase (parent, lines, cols, beginY, beginX),
      m_text(text),
      m_enalble(true)
{
    setBackground(m_enalble ? NcursesUtil::getInstance()->button_active_attr() : NcursesUtil::getInstance()->button_inactive_attr());
    Q_ASSERT(lines < parent->lines());
    Q_ASSERT(cols < parent->cols());
    curs_set(0);
}


void NcursesButton::show()
{
    if (m_box) {
        //box('|', '-');
    }

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

    move(height() / 2 , (width() - maxLength) / 2);

    drawFoucs();
    addstr(m_text.toUtf8().data());
    NCursesWindowBase::show();
}

void NcursesButton::setText(const QString &text)
{
    m_text = text;
}

void NcursesButton::setEnable(bool enable) {
    m_enalble = enable;
    show();
    m_parent->refresh();

}


void NcursesButton::onKeyPress(int keyCode) {
    if (!m_foucs || !m_enalble) {
        return;
    }
    if (keyCode == KEY_ENTER_OTHER) {
        emit clicked();
    }

}

QString NcursesButton::text() const
{
    return m_text;
}

void NcursesButton::resetBackground()
{
    setBackground(m_enalble ? NcursesUtil::getInstance()->button_active_attr() : NcursesUtil::getInstance()->button_inactive_attr());
}

void NcursesButton::drawFoucs()
{
    if (isOnFoucs()) {
        bkgd(NcursesUtil::getInstance()->button_key_active_attr());
    } else {
        bkgd(this->background());
    }
}

}
