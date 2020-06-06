#include "partition_frame.h"
#include <QThread>

#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/frames_cli/inner/full_disk_frame.h"
#include "ui/frames_cli/inner/advanced_partition_frame.h"

#include "ui/models/partition_model.h"

namespace installer {



void PartitionFramePrivate::initUI()
{
    try {
        //FrameInterfacePrivate::initUI();
        setBackground(NcursesUtil::getInstance()->dialog_attr());
        this->drawShadow(true);
        this->box();

        //重构建下一步和返回按钮，用于控制子界面的显示
        QString strBack = QObject::tr("back");
        QString strNext = QObject::tr("next");

        m_pBackButton = new NcursesButton(this, strBack, 3, 14, begy() + height() - 5, begx() + 5);
        m_pBackButton->drawShadow(true);
        m_pBackButton->box();
        m_pBackButton->setObjectName(strBack);

        m_pNextButton = new NcursesButton(this, strNext, 3, 14, begy() + height() - 5, begx() + width() - 20);
        m_pNextButton->drawShadow(true);
        m_pNextButton->box();
        m_pNextButton->setObjectName(strNext);

        //界面控件
        //m_label_title = new NcursesLabel(this, 1, 1, begy(), begx());
        //m_label_title->setFocusEnabled(false);

        m_label_comment1 = new NcursesLabel(this, 3, 2, begy(), begx());
        m_label_comment1->setFocusEnabled(false);
        m_label_comment1->setBackground(NcursesUtil::getInstance()->comment_attr());

        m_label_comment2 = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_comment2->setFocusEnabled(false);
        //m_label_comment2->setBackground(NcursesUtil::getInstance()->comment_attr());

        m_partitionmodelist = new NcursesListView(this, 2, width() / 2, begy(), begx());
        m_partitionmodelist->setFocus(true);

    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }
}

void PartitionFramePrivate::layout()
{
    try {
        int beginY = begy();
        //m_label_title->adjustSizeByContext();
        //m_label_title->mvwin(beginY, begx() + (width() - m_label_title->width()) / 2);
        //beginY += m_label_title->height() + 1;

        beginY = beginY + 2;
        m_label_comment1->adjustSizeByContext();
        m_label_comment1->mvwin(beginY, begx() + 1);
        beginY += m_label_comment1->height();

        m_label_comment2->adjustSizeByContext();
        m_label_comment2->mvwin(beginY, begx() + (width() - m_label_comment2->width()) / 2);
        beginY += m_label_comment2->height();

        m_partitionmodelist->adjustSizeByContext();
        m_partitionmodelist->mvwin(beginY, begx() + (width() - m_partitionmodelist->width()) / 2);
    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }


}

void PartitionFramePrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    //m_label_title->setText(tr("Create Partitions"));
    printTitle(QObject::tr("Create Partitions"), width());
    //m_label_comment1->setText(tr("   The setup program can guide you to use various standard schemes "
    // "for disk partition. If you like, you can do it manually. If you choose the Partition Wizard,"
    // "you will have the opportunity to check and modify the partition settings later."));
    m_label_comment1->setText(tr("    Make sure you have backed up important data, then select the partition mode."));

    m_label_comment2->setText(QString(tr("Partition mode")).append(" :"));

    QStringList strList;
    strList << tr("Full disk");
    strList << tr("Advanced");

    m_partitionmodelist->setList(strList);

    FrameInterfacePrivate::updateTs();
    layout();
}

bool PartitionFramePrivate::validate()
{
    return true;
}

void PartitionFramePrivate::show()
{
    if(m_parMode == ePartitionMode::PAR_MOD_FULL_DISK){
        m_partitionmodelist->setCurrentIndex(0);

    } else if (m_parMode == ePartitionMode::PAR_MOD_MANUAL) {
        m_partitionmodelist->setCurrentIndex(1);
    }

    if(!m_isshow) {
        NCursesWindowBase::show();
        m_isshow = true;
        m_pNextButton->setFocus(true);
    }
}

void PartitionFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void PartitionFramePrivate::initConnection()
{
    connect(m_pBackButton, &NcursesButton::clicked, this, &PartitionFramePrivate::doBackBtnClicked);
    connect(m_pNextButton, &NcursesButton::clicked, this, &PartitionFramePrivate::doNextBtnClicked);
    connect(m_partitionmodelist, &NcursesListView::selectChanged, this, [=](int index){
       m_parMode = static_cast<ePartitionMode>(index);
    });

}

void PartitionFramePrivate::onKeyPress(int keyCode)
{
    //NCursesWindowBase::onKeyPress(keyCode);
}

void PartitionFramePrivate::keyEventTriger(int keycode)
{
    if(m_currentchoicetype != -1){
        emit keyEventTrigerSignal(keycode);
    } else {
        FrameInterfacePrivate::keyEventTriger(keycode);
        //emit keyEventTrigerSignal(keycode);
    }
}

void PartitionFramePrivate::doBackBtnClicked()
{
    emit back();
}

void PartitionFramePrivate::doNextBtnClicked()
{
    if(m_currentchoicetype == -1){
        if(m_parMode == ePartitionMode::PAR_MOD_FULL_DISK){
            m_currentchoicetype = 0;

        } else if (m_parMode == ePartitionMode::PAR_MOD_MANUAL) {
            m_currentchoicetype = 1;
        }

        if(m_currentchoicetype == -1)
            return;
        //m_label_title->hide();
        m_label_comment1->hide();
        m_label_comment2->hide();
        m_pBackButton->setFocus(false);
        m_pNextButton->setFocus(false);
        //m_label_title->setFocus(false);
        m_partitionmodelist->setFocus(false);
        m_pBackButton->setFocusEnabled(false);
        m_pNextButton->setFocusEnabled(false);
        //m_label_title->setFocusEnabled(false);
        m_partitionmodelist->setFocusEnabled(false);
        m_partitionmodelist->hide();

        showChildSignal(m_currentchoicetype);
    }
}

void PartitionFramePrivate::setValue()
{
    m_currentchoicetype = -1;
    m_pBackButton->setFocusEnabled(true);
    m_pNextButton->setFocusEnabled(true);
    m_partitionmodelist->setFocusEnabled(true);
}


PartitionFrame::PartitionFrame(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_partition_model = new PartitionModel(this);
    m_private = new PartitionFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
    m_fullDiskFrame = new FullDiskFrame(this, m_partition_model);
    m_advancedPartitionFrame = new AdvancedPartitionFrame(this, m_partition_model);

    addChildFrame(m_fullDiskFrame);
    addChildFrame(m_advancedPartitionFrame);
    m_fullDiskFrame->hide();
    m_advancedPartitionFrame->hide();

    Q_D(PartitionFrame);
    connect( static_cast<AdvancedPartitionFramePrivate*>(m_advancedPartitionFrame->getPrivate()),
             &AdvancedPartitionFramePrivate::startInstall, d, &PartitionFramePrivate::dostartInstall);
    connect( static_cast<FullDiskFramePrivate*>(m_fullDiskFrame->getPrivate()),
             &FullDiskFramePrivate::startInstall, d, &PartitionFramePrivate::dostartInstall);
    connect(d, &PartitionFramePrivate::showChildSignal, this, &PartitionFrame::showChildSlot);
}

PartitionFrame::~PartitionFrame()
{

}


bool PartitionFrame::init()
{
    static bool isFirst = true;
    if (isFirst) {
        scanDevices();
        isFirst = false;
    }
    if (m_currState == FRAME_STATE_NOT_START) {        
        //m_private->layout();
        m_currState = FRAME_STATE_RUNNING;
    }

    Q_D(PartitionFrame);
    d->setValue();

    return true;
}

QString PartitionFrame::getFrameName()
{
    return "PartitionFrame";
}

void PartitionFrame::scanDevices()
{
    m_partition_model->scanDevices();
}

bool PartitionFrame::handle()
{
    {
        //do something
    }
    //m_private->keyHandle();
    //writeConf();
    return true;
}

void PartitionFrame::writeConf()
{

}

void PartitionFrame::showChildSlot(int index)
{
    if ( index == 0) {
        m_fullDiskFrame->setShowEnable(true);
        m_advancedPartitionFrame->setShowEnable(false);
    } else if(index == 1) {
        m_fullDiskFrame->setShowEnable(false);
        m_advancedPartitionFrame->setShowEnable(true);
    }

    hide();
    showChild(index);
}

//void PartitionFrame::FullDiskPartitionComplete()
//{
//    Q_D(PartitionFrame);
//    emit d->dostartInstall();
//}

}
