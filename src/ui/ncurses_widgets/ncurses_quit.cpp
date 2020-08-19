#include "ncurses_quit.h"
#include "ncurses_button.h"
#include "ncurses_label.h"

#include "service/power_manager.h"

installer::NcursesQuit::NcursesQuit(installer::NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX, bool shadow, bool box) :
    NCursesWindowBase(parent, lines, cols, beginY, beginX, shadow, box)
{
    m_title_lab = new NcursesLabel(this, 1, cols - 2, beginY, beginX);
    m_title_lab->setFocusEnabled(false);

    m_desc_lab = new NcursesLabel(this, 1, cols - 2, beginY, beginX);
    m_desc_lab->setFocusEnabled(false);

    m_cancel_bnt = new NcursesButton(this, ::QObject::tr("Continue"), 3, 14, begy() + height() - 5, begx() + 5);
    m_cancel_bnt->drawShadow(true);
    m_cancel_bnt->box();
    m_cancel_bnt->setFocus(true);
    connect(m_cancel_bnt, &NcursesButton::clicked, this, &NcursesQuit::cancel);

    m_quit_bnt = new NcursesButton(this, ::QObject::tr("Abort"), 3, 14, begy() + height() - 5, begx() + width() - 20);
    m_quit_bnt->drawShadow(true);
    m_quit_bnt->box();
    m_quit_bnt->setFocus(false);
    connect(m_quit_bnt, &NcursesButton::clicked, this, [=] {
#ifdef QT_DEBUG
        qDebug() << "ShutdownSystem";
#else
        if (!ShutdownSystem()) {
            qWarning() << "ShutdownSystem() failed!";
        }

        if (!ShutdownSystemWithMagicKey()) {
            qWarning() << "ShutdownSystemWithMagicKey() failed!";
        }
#endif // QT_DEBUG
    });
}

void installer::NcursesQuit::show()
{
    int curY = begy() + 6;
    int curX = begx();

    this->drawShadow(true);
    this->box();

    printTitle(::QObject::tr("Abort Installation"), width());

    m_title_lab->adjustSizeByContext();
    m_title_lab->mvwin(curY, curX + (width() / 2) - (m_title_lab->width() / 2));

    m_desc_lab->setText(::QObject::tr("Relevant operations you made in the installation process will not take effect, abort or continue installation?"));
    m_desc_lab->adjustSizeByContext();
    m_desc_lab->mvwin(curY, curX + (width() / 2) - (m_desc_lab->width() / 2));

    m_cancel_bnt->erase();
    m_cancel_bnt->resetBackground();
    m_cancel_bnt->box(ACS_VLINE,ACS_HLINE);
    m_cancel_bnt->setText(::QObject::tr("Continue"));
    m_cancel_bnt->setFocus(true);

    m_quit_bnt->erase();
    m_quit_bnt->resetBackground();
    m_quit_bnt->box(ACS_VLINE,ACS_HLINE);
    m_quit_bnt->setText(::QObject::tr("Abort"));

    return NCursesWindowBase::show();
}

void installer::NcursesQuit::hide()
{
    this->setFocus(false);
    m_quit_bnt->setFocus(false);
    m_cancel_bnt->setFocus(false);

    return NCursesWindowBase::hide();
}


void installer::NcursesQuit::keyHandle(int key)
{
    switch (key) {
        case KEY_TAB:
            qDebug() << "keyHandle = " << KEY_TAB;
            switchChildWindowsFoucs();
            show();
        break;
        default:
        for (NCursesWindowBase* childWindow : m_childWindows) {
            if (childWindow->isOnFoucs()) {
                 childWindow->onKeyPress(key);
            }
        }
        break;
    }
}




