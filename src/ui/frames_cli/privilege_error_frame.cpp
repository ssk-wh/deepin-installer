#include "language_frame.h"
#include "cursesp.h"

#include "privilege_error_frame.h"

namespace installer {

PrivilegeErrorFramePrivate::PrivilegeErrorFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
{
    initUI();
}

void PrivilegeErrorFramePrivate::initUI()
{
    bkgd(NcursesUtil::getInstance()->dialog_attr());
    FrameInterfacePrivate::initUI();
    setTitle("Privilege Error");
}

void PrivilegeErrorFramePrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    setTitle("Privilege Error");
    FrameInterfacePrivate::updateTs();
}

bool PrivilegeErrorFramePrivate::validate()
{
    return true;
}

PrivilegeErrorFrame::PrivilegeErrorFrame(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = MAINWINDOW_HEIGHT;//LINES / 2;
    int w = MAINWINDOW_WIDTH;//COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new PrivilegeErrorFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
}

bool PrivilegeErrorFrame::init()
{
    if (m_currState == FRAME_STATE_NOT_START) {
        m_currState = FRAME_STATE_RUNNING;
    }
    return true;
}

QString PrivilegeErrorFrame::getFrameName()
{
    return "PrivilegeErrorFrame";
}

bool PrivilegeErrorFrame::handle()
{
    return true;
}

}
