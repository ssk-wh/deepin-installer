#include "virtual_machine_frame.h"


namespace installer {

VirtualMachineFramePrivate::VirtualMachineFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
{

}

void VirtualMachineFramePrivate::initUI()
{
    FrameInterfacePrivate::initUI();
    printTitle("Virtual Machine", width());
}

void VirtualMachineFramePrivate::updateTs()
{
    box(ACS_VLINE, ACS_HLINE);
    printTitle("Virtual Machine", width());
    FrameInterfacePrivate::updateTs();
}


VirtualMachineFrame::VirtualMachineFrame(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new VirtualMachineFramePrivate(parent->getPrivate(), h, w, beginY, beginX);
}

VirtualMachineFrame::~VirtualMachineFrame()
{

}


bool VirtualMachineFrame::init()
{
    if (m_currState == FRAME_STATE_NOT_START) {
        m_private->initUI();
        m_currState = FRAME_STATE_RUNNING;
    }
    Q_D(VirtualMachineFrame);
    d->updateTs();
    m_private->show();
    return true;
}

QString VirtualMachineFrame::getFrameName()
{
    return "VirtualMachineFrame";
}

bool VirtualMachineFrame::handle()
{
    {
        //do something
    }
    m_private->keyHandle();
    return true;
}

}
