#include "disk_space_insufficient_frame.h"
#include "ui/ncurses_widgets/ncurses_label.h"

#include <QApplication>


namespace installer {




DiskSpaceInsufficientPrivate::DiskSpaceInsufficientPrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      m_quitBtn(nullptr),
      m_isshow(true)
{
    initUI();
}

void DiskSpaceInsufficientPrivate::initUI()
{
    setTitle("Disk spece insufficient");

    //FrameInterfacePrivate::initUI();
    setBackground(NcursesUtil::getInstance()->dialog_attr());
    this->drawShadow(true);
    this->box();

    setBackground(NcursesUtil::getInstance()->dialog_attr());

    int h = 6;
    int w = this->width() - 4;
    int beginY = LINES / 4 + 2;
    int beginX = COLS  / 4 + 2;


    NcursesLabel *title = new NcursesLabel(this, h, w, beginY, beginX);
    title->setFocusEnabled(false);
    title->setBackground(NcursesUtil::getInstance()->dialog_attr());
    title->setAttr(NcursesUtil::getInstance()->dialog_attr());
    title->setText("Disk space is insufficient, need at least 64GB of space");

    QString quitStr = ::QObject::tr("Abort");
    int buttonHeight = 3;
    int buttonWidth = std::max(quitStr.length(), quitStr.length()) + 4;
    int bntBeginY = (LINES - buttonHeight - 2) / 2 + buttonHeight;
    int bntBeginX = (COLS - buttonWidth) / 2;
    m_quitBtn = new NcursesButton(this, quitStr, buttonHeight, buttonWidth, bntBeginY, bntBeginX);
    m_quitBtn->drawShadow(true);
    m_quitBtn->box();
    m_quitBtn->setFocus(true);
}

void DiskSpaceInsufficientPrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    setTitle("Disk spece insufficient");
    FrameInterfacePrivate::updateTs();
    if(m_quitBtn != nullptr) {
        m_quitBtn->erase();        
        QString quitStr = ::QObject::tr("Abort");
        if (installer::ReadLocale() == "zh_CN") {
            m_quitBtn->resizew(3, quitStr.length() * 2 + 4 * 2);
        } else {
            m_quitBtn->resizew(3, quitStr.length() + 4);
        }
        m_quitBtn->resetBackground();
        m_quitBtn->box(ACS_VLINE,ACS_HLINE);
        m_quitBtn->setText(quitStr);
    }
}

bool DiskSpaceInsufficientPrivate::validate()
{
    return true;
}

void DiskSpaceInsufficientPrivate::show()
{
    if(!m_isshow) {
        FrameInterfacePrivate::show();
        m_isshow = true;
    }
}

void DiskSpaceInsufficientPrivate::keyHandle()
{
    while (!this->hidden()) {
        int key = getKey();
        switch (key) {
            case KEY_ENTER_OTHER:
                return;
        }
    }
}

DiskSpaceInsufficient::DiskSpaceInsufficient(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = MAINWINDOW_HEIGHT;//LINES / 2;
    int w = MAINWINDOW_WIDTH;//COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new DiskSpaceInsufficientPrivate (parent->getPrivate(), h, w, beginY, beginX);
}

DiskSpaceInsufficient::~DiskSpaceInsufficient()
{

}


bool DiskSpaceInsufficient::init()
{
    //if (m_currState == FRAME_STATE_NOT_START) {
    //    m_private->initUI();
    //    m_currState = FRAME_STATE_RUNNING;
    //}
    //Q_D(DiskSpaceInsufficient);
    //d->updateTs();
    //m_private->show();
    return true;
}

QString DiskSpaceInsufficient::getFrameName()
{
    return "DiskSpaceInsufficient";
}

bool DiskSpaceInsufficient::handle()
{
    {
        //do something
    }
    m_private->keyHandle();
    emit close();
    return true;
}

}
