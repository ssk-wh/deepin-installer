#include "ncurses_line_edit.h"

#include <QTimer>

installer::NCursesLineEdit::NCursesLineEdit(installer::NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX):
    NCursesWindowBase(parent, lines, cols, beginY, beginX),
    m_is_passowrd(false),
    m_isNumber(false)
{
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

    NCursesWindowBase::show();
}

void installer::NCursesLineEdit::setEchoMode(bool isPassword)
{
    m_is_passowrd = isPassword;
}

QString installer::NCursesLineEdit::text() const
{
    return m_text;
}

void installer::NCursesLineEdit::onKeyPress(int keyCode)
{
    qDebug() << "keyCode = " << keyCode;

    if (!this->isOnFoucs()) {
        return;
    }
    QString text = m_text;
    switch (keyCode) {
        case KEY_ENTER_OTHER:
        case KEY_ESC:
        case KEY_TAB:
        break;
        case KEY_BACK:
            text.remove(text.length()-1, 1);
        break;
        default:
        if (keyCode > 32 && keyCode <= 126 && text.length() < width()) {
            text += QChar(keyCode);
        } else {
            beep();
        }


    }

    this->setText(text);
    NCursesWindowBase::onKeyPress(keyCode);
}

void installer::NCursesLineEdit::drawFoucs()
{
    if (isOnFoucs()) {
        //this->changedText();
        bkgd(NcursesUtil::getInstance()->button_key_active_attr());
    } else {
        //this->changedText();
        bkgd(this->background());
    }
}

void installer::NCursesLineEdit::changedText()
{
    this->erase();
    if (m_is_passowrd) {
        for (int i = 0; i < m_text.size(); i++) {
            this->printw("*");
        }
    } else {
        if(m_text.compare(""))
            this->printw("%s", m_text.toUtf8().data());
    }

    emit textChanged(m_text);
}


