#ifndef INSTALL_NCURSES_WINDOWS_h
#define INSTALL_NCURSES_WINDOWS_h

#include <cursesw.h>
#include "ncurses_util.h"
#include <QScopedPointer>
#include <QString>
#include <QDebug>
#include <QObject>
#include <cursesp.h>

namespace installer {
const int KEY_ESC = 27;
const int KEY_TAB  = 9;
const int KEY_SPACE = 32;
const int KEY_ENTER_OTHER = 10;
const int KEY_BACK = 263;

class NCursesWindowBase : public QObject, public NCursesPanel
{
    Q_OBJECT
public:
    NCursesWindowBase(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, bool shadow = false, bool box = false);
    virtual ~NCursesWindowBase(){}
    void printTitle(QString title, int width);
    void setTitle(QString title);
    void drawShadow(bool isshow);
    void addChildWindows(NCursesWindowBase* childWindows);
    void removeChildWindows(NCursesWindowBase* childWindows);
    virtual void setFocus(bool foucs);
    virtual void setFocusEnabled(bool enabled);        
    virtual bool isOnFoucs();
    virtual void show();
    virtual void hide();
    virtual void switchChildWindowsFoucs();
    virtual void onKeyPress(int keyCode);
    virtual void resize(int lines, int cols);
    virtual void drawFoucs();
    virtual void drawCurs();
    virtual void adjustSizeByContext();
    virtual void resizew(int newLines, int newColumns);
    virtual void setIsShow(bool isshow);
    void setBackground(chtype box);
    void setAttr(chtype attr);
    chtype background();
    void setBold(bool bold);
    void setFocusStyle(chtype type);
    chtype getFocusStyle(){ return m_chtype_focus; }
signals:
    void selectionChanged();
    void enter();
    void esc();
    void outFoucs();
    void inFoucs();

protected:
    void addFoucsWindows(NCursesWindowBase* childWindows);
    void removeFoucsWindows(NCursesWindowBase* childWindows);

private:
    void clearFoucs();
    void updateFoucs(bool foucs);

protected:
    NCursesWindowBase* m_parent;
    QVector<NCursesWindowBase*> m_childWindows;
    QVector<NCursesWindowBase*> m_foucsWindows;
    bool m_shabow;
    bool m_box;
    bool m_foucs;

    bool m_isshadowWindow        = false;
    NCursesPanel* m_shadowWindow = nullptr;

    bool m_foucs_enabled;
    chtype m_background;
    chtype m_attr;

    QString m_title;
    bool m_isBold = false;
    chtype m_chtype_focus;
};

}



#endif
