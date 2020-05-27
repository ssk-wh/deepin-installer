#ifndef NCURSES_LINE_EDIT_H
#define NCURSES_LINE_EDIT_H

#include "ui/ncurses_widgets/ncurses_windows_base.h"

namespace installer {

class NcursesLabel;

class NCursesLineEdit : public NCursesWindowBase
{
    Q_OBJECT
public:
    NCursesLineEdit(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    void setIsNumber(bool isnumber);
public slots:
    void setText(const QString &);
    void setEchoMode(bool isPassword);
    QString text() const;
    void setFocus(bool foucs) override;

protected:
    void onKeyPress(int keyCode) override;
    void drawFoucs() override;

private:
    void changedText();

signals:
    void textChanged(const QString &);
    void editChanged(const QString &);
    void editingFinished();

private:
    NcursesLabel * m_showlabel = nullptr;
    QString        m_text;
    bool           m_is_passowrd;
    bool m_isNumber = false;
    bool m_isFinished = false;
};

}

#endif // NCURSES_LINE_EDIT_H
