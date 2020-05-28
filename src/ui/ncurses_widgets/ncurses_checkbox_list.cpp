#include "ncurses_checkbox_list.h"
#include "ncurses_checkbox.h"

namespace installer {

NcursesCheckBoxList::NcursesCheckBoxList(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
    :NCursesWindowBase(parent, lines, cols, beginY, beginX),
      m_singleselect(false),
      m_realselect(false),
      m_listtype(EXTRACHOICES),
      m_index(0),
      m_showindex(0),
      m_reserveX(1),
      m_heightpos(0)
{
    this->setBackground(parent->background());
}

NcursesCheckBoxList::~NcursesCheckBoxList()
{
    clearList();
}

void NcursesCheckBoxList::setList(QVector<QPair<QString, QString>>& list,  bool iswchar)
{
    if(list.size() == 0) {
        return;
    }

    clearList();

    foreach (auto text, list) {
        NcursesCheckBox* testcheckbox = new NcursesCheckBox(this, 1, width() - 2, begy(), begx() + 2);
        testcheckbox->setIsUseTitle(true);
        testcheckbox->setText(text.first, text.second, iswchar);
        testcheckbox->setBackground(this->background());
        testcheckbox->hide();

        if(m_selectitems.indexOf(text.second) != -1) {
            testcheckbox->setSelect(true);
        }
        m_ncursesCheckBoxs_vector.push_back(testcheckbox);
    }

    m_index     = 0;
    m_showindex = m_index;
    m_heightpos = m_ncursesCheckBoxs_vector.at(m_index)->getStrHeight();
}

QString NcursesCheckBoxList::getCurrentTitle()
{
    return m_ncursesCheckBoxs_vector.at(m_index)->title();
}

QString NcursesCheckBoxList::getCurrentText()
{
    return m_ncursesCheckBoxs_vector.at(m_index)->text();
}


void NcursesCheckBoxList::clearList()
{
    foreach(NcursesCheckBox* testitem, m_ncursesCheckBoxs_vector) {
        if(testitem->isSelect()) {
            if(m_selectitems.indexOf(testitem->text()) == -1) {
                m_selectitems.append(testitem->text());
            }
        } else {
            if(m_selectitems.indexOf(testitem->text()) != -1) {
                m_selectitems.removeOne(testitem->text());
            }
        }

        delete testitem;
        testitem = nullptr;
    }

    m_childWindows.clear();
    m_foucsWindows.clear();
    m_ncursesCheckBoxs_vector.clear();
}

void NcursesCheckBoxList::onKeyPress(int keyCode)
{
    switch (keyCode) {
    case KEY_UP:
        m_ncursesCheckBoxs_vector.at(m_index)->setFocus(false);
        m_index--;
        if(m_index < 0){
            m_index = 0;
            m_heightpos = m_ncursesCheckBoxs_vector.at(m_index)->getStrHeight();
        } else {
            m_heightpos -= m_ncursesCheckBoxs_vector.at(m_index+1)->getStrHeight();
        }

        if((m_heightpos == 0) && (m_index > 0)) {
            int oldheightpos = 0;
            for(int i = m_index; ; i--) {
                if((m_heightpos >= height()) || (i < 0)) {
                    oldheightpos = m_heightpos;
                    if(i < 0)
                        m_showindex = 0;
                    else
                        m_showindex = i;

                    show();
                    m_ncursesCheckBoxs_vector.at(m_index)->setFocus(false);
                    m_heightpos = oldheightpos;
                    break;
                } else {
                    m_heightpos += m_ncursesCheckBoxs_vector.at(i)->getStrHeight();
                }
            }
        }
        m_ncursesCheckBoxs_vector.at(m_index)->setFocus(true);

        if(m_singleselect) {
            doSingleSelect();
        }
        this->refresh();
        break;

    case KEY_DOWN:
        m_ncursesCheckBoxs_vector.at(m_index)->setFocus(false);
        m_index++;
        if(m_index >= m_ncursesCheckBoxs_vector.size()){
            m_index = m_ncursesCheckBoxs_vector.size() - 1;
        } else {
            m_heightpos += m_ncursesCheckBoxs_vector.at(m_index)->getStrHeight();
        }

        if(m_heightpos > height()) {
            m_showindex = m_index;
            show();
            m_heightpos = m_ncursesCheckBoxs_vector.at(m_index)->getStrHeight();
        }

        m_ncursesCheckBoxs_vector.at(m_index)->setFocus(true);

        if(m_singleselect) {
            doSingleSelect();
        }
        this->refresh();
        break;

    case 32:
        if(m_listtype == BASICENVIRONMENT) {

        } else if(m_listtype == EXTRACHOICES){
             m_ncursesCheckBoxs_vector.at(m_index)->onKeyPress(keyCode);
             if(m_ncursesCheckBoxs_vector.at(m_index)->isSelect()){
                 if(m_selectitems.indexOf(m_ncursesCheckBoxs_vector.at(m_index)->text()) == -1) {
                     m_selectitems.append(m_ncursesCheckBoxs_vector.at(m_index)->text());
                 }
             } else {
                 if(m_selectitems.indexOf(m_ncursesCheckBoxs_vector.at(m_index)->text()) != -1) {
                     m_selectitems.removeOne(m_ncursesCheckBoxs_vector.at(m_index)->text());
                 }
             }
        }
        break;
    default:
        break;
    }

    emit signal_KeyTriger(keyCode, m_listtype);
    NCursesWindowBase::onKeyPress(keyCode);
}

void NcursesCheckBoxList::show()
{
    NCursesWindowBase::show();

    erase();

    if(m_index >= m_ncursesCheckBoxs_vector.size()) {
        return;
    }

    foreach(NcursesCheckBox* testitem, m_ncursesCheckBoxs_vector) {
        testitem->hide();
    }

    int testheightpos = 0;
    for(int i = m_showindex; i < m_ncursesCheckBoxs_vector.size(); i++) {
        int testtempheightpos = testheightpos + m_ncursesCheckBoxs_vector.at(i)->getStrHeight();
        if(testtempheightpos > height()) {
            break;
        }
        m_ncursesCheckBoxs_vector.at(i)->moveWindowTo(begy() + testheightpos, begx() + 1);
        m_ncursesCheckBoxs_vector.at(i)->show();
        m_ncursesCheckBoxs_vector.at(i)->refresh();

        testheightpos += m_ncursesCheckBoxs_vector.at(i)->getStrHeight();
    }

    m_ncursesCheckBoxs_vector.at(m_index)->setFocus(true);

    if(m_listtype == BASICENVIRONMENT) {
        doSingleSelect();
    }
}

void NcursesCheckBoxList::drawFoucs()
{
    //m_foucs ? attron(A_REVERSE) : attroff(A_REVERSE);
}

void NcursesCheckBoxList::doSingleSelect()
{
    foreach(NcursesCheckBox* testitem, m_ncursesCheckBoxs_vector) {
        testitem->setSelect(false);
    }

    m_ncursesCheckBoxs_vector.at(m_index)->setSelect(true);

    this->refresh();
}

}
