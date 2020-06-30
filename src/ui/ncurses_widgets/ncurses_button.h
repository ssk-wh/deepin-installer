#ifndef INSTALL_NCURSES_BUTTON_H
#define INSTALL_NCURSES_BUTTON_H

#include <QObject>
#include "ui/ncurses_widgets/ncurses_windows_base.h"

namespace installer {

class NcursesButton : public NCursesWindowBase
{
    Q_OBJECT
public:
    NcursesButton(NCursesWindowBase* parent, QString text, int lines, int cols, int beginY, int beginX, bool box = true, bool shadow = true);
    virtual ~NcursesButton(){}
    void show() override;
    void setText(const QString& text);
    void setEnable(bool enable);
    void onKeyPress(int keyCode) override;
    QString text() const;
    void resetBackground();
    void drawFoucs();

signals:
    void clicked();
private:
    QString m_text;
    bool m_enalble;
};

}


#endif
