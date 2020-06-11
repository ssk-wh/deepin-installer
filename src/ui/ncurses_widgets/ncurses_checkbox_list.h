#ifndef INSTALL_NCURSES_COMBOX_LIST_H
#define INSTALL_NCURSES_COMBOX_LIST_H

#include "ui/ncurses_widgets/ncurses_windows_base.h"

namespace installer {

class NcursesCheckBox;

class NcursesCheckBoxList : public NCursesWindowBase
{
    Q_OBJECT
public:
    enum ListType{
        BASICENVIRONMENT = 0,
        EXTRACHOICES,
        OTHER
    };

    NcursesCheckBoxList(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);
    virtual ~NcursesCheckBoxList();
    void setListType(ListType type){ m_listtype = type; }
    void setSingleSelect(bool issingle){ m_singleselect = issingle; }
    void setRealSelect(bool isreal){ m_realselect = isreal; }
    void setList(QVector<QPair<QString, QString>>& list, bool iswchar = false, bool isusetitle = true);
    QString getCurrentTitle();
    QString getCurrentText();
    QString getCurrentSingleSelectTitle();
    QString getCurrentSingleSelectText();
    void clearList();
    void setSelectItems(QStringList& selectitems){ m_selectitems.clear(); m_selectitems = selectitems; }
    QStringList& getSelectItems(){ return m_selectitems; }
    void onKeyPress(int keyCode) override;
    void show() override;
    void drawFoucs() override;
    void selectAll(bool selectall);
    int size(){ return m_listsize; }

signals:
    void signal_KeyTriger(int keycode, int listtype, int index);

private:
    void doSingleSelect();

private:
    QStringList m_selectitems;
    QVector<NcursesCheckBox*> m_ncursesCheckBoxs_vector;
    bool m_singleselect;
    bool m_realselect;
    ListType m_listtype;
    int m_index;
    int m_showindex;
    int m_reserveX;
    int m_heightpos;
    int m_listsize;
    int testheight;
};

}

#endif // INSTALL_NCURSES_COMBOX_LIST_H
