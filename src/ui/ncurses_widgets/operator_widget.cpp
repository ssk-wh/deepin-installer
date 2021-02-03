#include "operator_widget.h"

#include "ui/ncurses_widgets/ncurses_label.h"

installer::OperatorWidget::OperatorWidget(installer::NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX, bool shadow, bool box):
    NCursesWindowBase(parent, lines, cols, beginY, beginX, shadow, box)
{
    m_titleLab = new NcursesLabel(this, 1, cols - 2, beginY, beginX);
    m_titleLab->setFocusEnabled(false);

    m_descLab = new NcursesLabel(this, 2, cols - 2, beginY, beginX);
    //m_descLab->setBold(true);
    m_descLab->setFocusEnabled(false);
}

void installer::OperatorWidget::setTitle(const QString &text)
{
    m_titleLab->setText(text);
}

void installer::OperatorWidget::setDesc(const QString &text)
{
    m_descLab->setText(text);
}

void installer::OperatorWidget::move(int y, int x)
{
    m_titleLab->mvwin(y, x);
    y += m_titleLab->height();
    m_descLab->mvwin(y, x);
    NCursesWindowBase::mvwin(y, x);
}

void installer::OperatorWidget::drawFoucs()
{
    if (isOnFoucs()) {
        m_titleLab->setBackground(NcursesUtil::getInstance()->button_key_active_attr());
        m_descLab->setBackground(NcursesUtil::getInstance()->button_key_active_attr());
    } else {
        m_titleLab->setBackground(this->background());
        m_descLab->setBackground(this->background());
    }
}


