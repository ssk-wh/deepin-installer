#include "ncurses_line_edit.h"

#include <QTimer>

installer::NCursesLineEdit::NCursesLineEdit(installer::NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX):
    NCursesWindowBase(parent, lines, cols, beginY, beginX),
    m_is_passowrd(false),
    m_isNumber(false)
{
    curs_set(0);
}

void installer::NCursesLineEdit::setIsNumber(bool isnumber)
{
    m_isNumber = isnumber;
}

void installer::NCursesLineEdit::setText(const QString &text)
{
    if (text.size() >0 && m_isNumber && !text.contains(QRegExp("^\\d+$"))) return;

    m_text = text;

    this->changedText();

    Q_EMIT textChanged(m_text);

    this->refresh();
}

void installer::NCursesLineEdit::setEchoMode(bool isPassword)
{
    m_is_passowrd = isPassword;
}

QString installer::NCursesLineEdit::text() const
{
    return m_text;
}

void installer::NCursesLineEdit::setFocus(bool foucs)
{
    if (foucs) {
        m_isFinished = true;
    }
    if (m_isFinished && !foucs) {
        Q_EMIT editingFinished();
    }

    return NCursesWindowBase::setFocus(foucs);
}

void installer::NCursesLineEdit::onKeyPress(int keyCode)
{
    if (!this->isOnFoucs()) {
        return;
    }

    QString text = m_text;
    switch (keyCode) {
        case KEY_ENTER_OTHER:
        case KEY_ESC:
        case KEY_TAB:
        return;
        case KEY_BACK:
            text.remove(text.length()-1, 1);
        break;
        default:
        {
            if (m_EditModle == IPEDIT) {
                if (((keyCode >= 48 && keyCode <= 57) || (keyCode == 46)) && text.length() < width()) {
                    text += QChar(keyCode);
                }
            } else {
                if (keyCode > 32 && keyCode <= 126 && text.length() < width()) {
                    text += QChar(keyCode);
                }
            }
        }
        break;
    }

    this->setText(text);
    Q_EMIT editChanged(text);
    NCursesWindowBase::onKeyPress(keyCode);
}

void installer::NCursesLineEdit::drawFoucs()
{
    if (isOnFoucs()) {
        bkgd(NcursesUtil::getInstance()->button_key_active_attr());
        this->changedText();
    } else {
        bkgd(this->background());
        this->changedText();
    }
}

void installer::NCursesLineEdit::changedText()
{
    this->erase();
    if (m_is_passowrd) {
        for (int i = 0; i < m_text.size(); i++) {
            addstr("*");
        }
    } else {
        if(m_text.compare(""))
            addstr(m_text.toUtf8().data());
    }
}


