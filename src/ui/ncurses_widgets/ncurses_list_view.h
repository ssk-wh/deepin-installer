#ifndef INSTALL_NCURSES_LIST_VIEW_H
#define INSTALL_NCURSES_LIST_VIEW_H

#include "ui/ncurses_widgets/ncurses_windows_base.h"

namespace installer {

class NcursesListView : public NCursesWindowBase
{
    Q_OBJECT
public:
    NcursesListView(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    virtual ~NcursesListView();
    void setList(QStringList& list);
    void setCurrentIndex(int index);
    int getCurrentIndex();
    QString getCurrenItem();
    QStringList getList();
    void append(QString& text);
    void onKeyPress(int keyCode) override;
    void show() override;
    void drawFoucs() override;
    void adjustSizeByContext() override;
    int size();
    void clearFoucs();
signals:
    void selectChanged(int index);
    void selectd(int index);

private:
    QStringList m_list;
    int m_index;
    int m_currLine;
    int m_reserveX;
    int m_height;
    int m_currentIndex = 0;
    int m_page = 0;
};


}




#endif
