#include "ui/ncurses_widgets/ncurses_label.h"
#include "service/settings_manager.h"

namespace installer {

NcursesLabel::NcursesLabel(NCursesWindowBase *parent, const QString &text, int lines, int cols, int beginY, int beginX)
    : NCursesWindowBase (parent, lines, cols, beginY, beginX),
      m_text(text),
      m_alignment(Qt::AlignLeft)
{
}

NcursesLabel::NcursesLabel(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : NCursesWindowBase(parent, lines, cols, beginY, beginX),
      m_alignment(Qt::AlignLeft)

{
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
    if (m_text.length() > m_parent->width()) {
        resize(height(), m_parent->width() - 2);
    } else {
        if(installer::ReadLocale() == "zh_CN"){
            if((m_text.length() * 2) <= m_parent->width()) {
                resize(height(), m_text.length() * 2);
            } else {
                resize(height(), m_text.length());
            }

        } else {
            resize(height(), m_text.length());
        }
    }
}

void NcursesLabel::drawFoucs()
{

}

QString NcursesLabel::text() const
{
    return m_text;
}

}
