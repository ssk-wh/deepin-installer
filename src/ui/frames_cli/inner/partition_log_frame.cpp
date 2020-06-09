#include "partition_log_frame.h"

namespace installer {

void PartitionLogFramePrivate::initUI()
{
    try {
        FrameInterfacePrivate::initUI();
        setBackground(NcursesUtil::getInstance()->dialog_attr());

        m_label_title = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_title->setFocusEnabled(false);

    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }
}

void PartitionLogFramePrivate::layout()
{
    try {
        int beginY = begy();
        m_label_title->adjustSizeByContext();
        m_label_title->mvwin(beginY, begx() + (width() - m_label_title->width()) / 2);

    } catch (NCursesException& e) {
         qCritical() << QString(e.message);
    }

}

void PartitionLogFramePrivate::updateTs()
{
    m_label_title->setText("Partiton operate log");

}

void PartitionLogFramePrivate::initConnection()
{

}

PartitionLogFrame::PartitionLogFrame(FrameInterface *parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new PartitionLogFramePrivate(nullptr, h - 8, w - 2, beginY + 1, beginX + 1);

}

PartitionLogFrame::~PartitionLogFrame()
{

}

bool PartitionLogFrame::init()
{
    if (m_currState == FRAME_STATE_NOT_START) {
        m_private->layout();
    }
    return true;
}

QString PartitionLogFrame::getFrameName()
{
      return "PartitionLogFrame";
}

bool PartitionLogFrame::handle()
{
    m_private->keyHandle();
    return true;
}

}
