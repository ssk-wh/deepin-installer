#ifndef INSTALL_NCURSES_PROGRESSBAR_H
#define INSTALL_NCURSES_PROGRESSBAR_H

#include <QObject>
#include "ui/ncurses_widgets/ncurses_windows_base.h"

namespace installer {

class NcursesProgressBar : public NCursesWindowBase
{
    Q_OBJECT

public:
    NcursesProgressBar(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    virtual ~NcursesProgressBar();
    void show() override;
    void setRange(int rangevalue);
    void setValue(int value);
    //void onKeyPress(int keyCode) override;
    int getValue() { return m_value;}
    void reSet();
    void start();
    void stop();

//signals:

private:
    int m_rangevalue;
    int m_value;
    NCursesPanel* m_progressSlidmonitor;
};

}
#endif // INSTALL_NCURSES_PROGRESSBAR_H
