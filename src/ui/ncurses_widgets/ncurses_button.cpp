#include "ncurses_button.h"

namespace installer {



NcursesButton::NcursesButton(NCursesWindowBase* parent, QString text, int lines, int cols, int beginY, int beginX, bool box, bool shadow)
    : NCursesWindowBase (parent, lines, cols, beginY, beginX),
      m_text(text),
      m_enalble(true)
{
    setBackground(m_enalble ? NcursesUtil::getInstance()->button_active_attr() : NcursesUtil::getInstance()->button_inactive_attr());
    Q_ASSERT(lines < parent->lines());
    Q_ASSERT(cols < parent->cols());
}


void NcursesButton::show()
{
    if (m_box) {
        //box('|', '-');
    }

    move(height() / 2 , (width() - m_text.length()) / 2);
    attron(m_foucs ? NcursesUtil::getInstance()->button_label_active_attr() : NcursesUtil::getInstance()->button_active_attr());
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

}
