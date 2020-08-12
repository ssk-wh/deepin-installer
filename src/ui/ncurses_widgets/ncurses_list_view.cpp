#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_checkbox.h"
#include "service/settings_manager.h"

namespace installer {

NcursesListView::NcursesListView(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : NCursesWindowBase (parent, lines, cols, beginY, beginX),
      m_index(0),
      m_currLine(0),
      m_reserveX(0),
      m_height(lines)

{
     bkgd(parent->getbkgd());
}

NcursesListView::~NcursesListView()
{

}

void NcursesListView::setList(QStringList &list)
{
    m_list.clear();
    foreach (NCursesWindowBase* child, m_childWindows) {
        if (child) {
            delete child;
            child = nullptr;
        }
    }
    m_childWindows.clear();
    m_foucsWindows.clear();
    m_index = 0;
    m_currLine = 0;
    m_currentIndex = 0;
    m_list = list;
    int i = 0;

    if (m_list.size() < m_height) {
        resize(m_list.size(), width());
    } else {
        resize(m_height, width());
    }

    foreach (QString text, m_list) {
        NcursesLabel* label = new NcursesLabel(this, text, 1, width() - 2 * m_reserveX, begy() + i % height(), begx() + m_reserveX);
        label->setFocusStyle(NcursesUtil::getInstance()->list_view_item_select());
        //label->setAlignment(Qt::AlignCenter);
        label->setFocusEnabled(true);
        i += label->height();
        label->hide();
    }
}


void NcursesListView::setCurrentIndex(int index)
{
    if (index < m_list.size()) {
        m_index = index / height();
        m_currLine = index % height();

        if (m_currentIndex != index) {
            m_currentIndex = index;
            emit selectChanged(index);
        }
    }

}

int NcursesListView::getCurrentIndex()
{
    return m_currentIndex;
}

QString NcursesListView::getCurrenItem()
{
   return  m_list.at(getCurrentIndex());
}

QStringList NcursesListView::getList()
{
 return m_list;
}

void NcursesListView::append(QString &text)
{

}

void NcursesListView::setSeelectMode(bool isSelect)
{
    m_is_select_mode = isSelect;
}

void NcursesListView::onKeyPress(int keyCode)
{
    switch (keyCode) {
        case KEY_UP:
            if (m_currentIndex) {
                m_childWindows[m_currentIndex]->setFocus(false);
                m_currentIndex--;
                m_currLine--;
                if (m_currLine < 0) {
                    show();
                } else {
                    m_childWindows[m_currentIndex]->setFocus(true);
                }
                emit selectChanged(m_currentIndex);
            }
            break;
        case KEY_DOWN:
            if (m_currentIndex < m_childWindows.size()-1) {
                m_childWindows[m_currentIndex]->setFocus(false);
                m_currentIndex++;
                m_currLine++;
                if (m_currLine > height()) {
                    show();
                } else {
                    m_childWindows[m_currentIndex]->setFocus(true);
                }
                emit selectChanged(m_currentIndex);
            }

            break;
        case KEY_ENTER_OTHER:
            emit selectd(m_currentIndex);
            break;
        default:
            break;
    }

    NCursesWindowBase::onKeyPress(keyCode);
}

void NcursesListView::show()
{
    for (int i = 0; i < m_childWindows.size(); i++) {
        m_childWindows[i]->setFocus(false);
        m_childWindows[i]->hide();
    }

    m_page = m_currentIndex / height();
    for (int currY = 0; currY <= height() && currY + (m_page * height()) < m_childWindows.size(); currY++) {
        if (m_is_select_mode) {
            if(isOnFoucs()){
                m_childWindows[m_currentIndex]->setFocus(true);
            } else {
                chtype testchtype = m_childWindows[m_currentIndex]->getFocusStyle();
                m_childWindows[m_currentIndex]->bkgd(testchtype);
                m_childWindows[m_currentIndex]->show();
            }

        }
        m_childWindows[currY + (m_page * height())]->adjustSizeByContext();
        m_childWindows[currY + (m_page * height())]->mvwin(begy() + currY, begx() + m_reserveX);
        m_childWindows[currY + (m_page * height())]->show();
    }

    m_currLine = m_currentIndex % height();

    refresh();
}


void NcursesListView::drawFoucs()
{

}

void NcursesListView::adjustSizeByContext()
{
    if (m_list.empty()) {
        return;
    }
    auto iter = std::max_element(std::begin(m_list), std::end(m_list), [&](const QString& a, const QString& b) -> bool {
        return  a.length() < b.length();
    });
    int maxLength = iter->length();
    if(installer::ReadLocale() == "zh_CN") {
        maxLength = iter->length() * 2;

        if(maxLength >((m_parent->width() / 2) - 3)) {
            maxLength = iter->length();
        }
    }

    resize(height(), maxLength);
    foreach (NCursesWindowBase* child, m_childWindows) {
        if (child) {
            child->resize(child->height(), maxLength);
        }
    }
}

int NcursesListView::size()
{
    return m_list.size();
}

void NcursesListView::setFocus(bool foucs)
{
    m_foucs = foucs;
    //NCursesWindowBase::setFocus(foucs);
    if (m_childWindows.size() > 0) {
        if (foucs) {
            m_childWindows[m_currentIndex]->setFocus(true);
        } else {
            //m_childWindows[m_currentIndex]->setFocus(false);
            chtype testchtype = m_childWindows[m_currentIndex]->getFocusStyle();
            m_childWindows[m_currentIndex]->bkgd(testchtype);
            m_childWindows[m_currentIndex]->show();
        }
    }

}

void NcursesListView::clearFoucs()
{
    for (int i = 0; i < m_childWindows.size(); i++) {
        m_childWindows[i]->setFocus(false);
    }
}


}
