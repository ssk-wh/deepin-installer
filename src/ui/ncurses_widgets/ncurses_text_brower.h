#ifndef INSTALL_NCURSES_TEXT_BROWER_H
#define INSTALL_NCURSES_TEXT_BROWER_H

#include "ui/ncurses_widgets/ncurses_windows_base.h"
#include <QScopedPointer>

namespace installer {

class NcursesTextBrower : public NCursesWindowBase
{
public:
    NcursesTextBrower(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    virtual ~NcursesTextBrower(){}
    void setText(const QString &text, bool iswchar = false);
    void appendText(const QString& text, bool iswchar = false);
    void appendItemText(const QString& text, bool iswchar = false);
    bool setFileName(const QString);
    void scrollToEnd();
    void clearText();
    int getTextHeight(){ return m_text.size(); }
    void setFocus(bool foucs) override;
    void show() override;
    void onKeyPress(int keyCode) override;

private:
    QStringList m_text;
    QScopedPointer<NCursesFramedPad> m_pad;
    int m_currLine;
    int m_reserveY;
    int m_reserveX;
};


}

#endif
