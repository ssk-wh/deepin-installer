#include "partition_table_warning_frame.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/operator_widget.h"

installer::PartitionTableWarningFrame::PartitionTableWarningFrame(installer::FrameInterface *parent):
    FrameInterface(parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new PartitionTableWarningFramePrivate(this, h - 8, w - 2, beginY + 1, beginX + 1);

#ifdef QT_DEBUG
    connect(this, &PartitionTableWarningFrame::reboot, this, [=]{qInfo() << "test reboot";});
    connect(this, &PartitionTableWarningFrame::cancel, this, [=]{qInfo() << "test cancel";});
    connect(this, &PartitionTableWarningFrame::formatting, this, [=]{qInfo() << "test formatting";});
#endif // QT_DEBUG
}

bool installer::PartitionTableWarningFrame::handle()
{
    m_private->keyHandle();
    return true;
}

installer::PartitionTableWarningFramePrivate::PartitionTableWarningFramePrivate(FrameInterface *parent, int lines, int cols, int beginY, int beginX):
    FrameInterfacePrivate(nullptr, lines, cols, beginY, beginX),
    q_ptr(qobject_cast<PartitionTableWarningFrame*>(parent))
{
    initUI();
    updateTs();
    layout();
}

void installer::PartitionTableWarningFramePrivate::initUI()
{
    //FrameInterfacePrivate::initUI();
    setBackground(NcursesUtil::getInstance()->dialog_attr());

    m_titleLab = new NcursesLabel(this, 1, 1, begy(), begx());
    m_titleLab->setBackground(NcursesUtil::getInstance()->item_attr());
    m_titleLab->setFocusEnabled(false);

    m_commentLab = new NcursesLabel(this, 2, width() - 20, begy(), begx());
    m_commentLab->setFocusEnabled(false);

    m_warning1 = new OperatorWidget(this, 3, width() - 30, begy(), begx());
    m_warning1->setFocus(true);
    m_warning1->setFocusEnabled(false);
    m_warning2 = new OperatorWidget(this, 3, width() - 30, begy(), begx());
    m_warning2->setFocusEnabled(false);
    m_warning3 = new OperatorWidget(this, 3, width() - 30, begy(), begx());
    m_warning3->setFocusEnabled(false);
}

void installer::PartitionTableWarningFramePrivate::layout()
{
    try {
        int beginY = begy() + 1;
        m_titleLab->adjustSizeByContext();
        m_titleLab->mvwin(beginY, begx() + (width() - m_titleLab->width()) / 2);

        beginY += m_titleLab->height();
        m_commentLab->mvwin(beginY, begx() + (width() - m_commentLab->width()) / 2);

        beginY += m_commentLab->height() + 1;
        m_warning1->move(beginY, begx() + (width() - m_warning1->width()) / 2);
        beginY += m_warning1->height() + 1;
        m_warning2->move(beginY, begx() + (width() - m_warning2->width()) / 2);
        beginY += m_warning2->height() + 1;
        m_warning3->move(beginY, begx() + (width() - m_warning3->width()) / 2);

    } catch (NCursesException& e) {
         qCritical() << QString(e.message);
    }
}

void installer::PartitionTableWarningFramePrivate::updateTs()
{
    m_titleLab->setText(tr("Warning"));
    m_commentLab->setText(tr("You have an EFI boot loader but an MBR disk, thus you cannot install UOS directly. "
                             "Please select one of the below solutions and continue."));
    m_warning1->setTitle(QString("%1").arg(tr("Restart")));
    m_warning1->setDesc(QString("1.%1.\n2.%3")
                        .arg(tr("Restart the BIOS Settings, and close the UEFI startup"))
                        .arg(tr("Quit the BIOS, back into the UOS installation")));
    m_warning2->setTitle(QString("%1").arg(tr("Format the entire disk")));
    m_warning2->setDesc(QString("1.%1.\n2.%3")
                        .arg(tr("Please backup all your data, in order to avoid data loss"))
                        .arg(tr("Please double check whether or not you backup the data, and then restart to this interface")));
    m_warning3->setTitle(QString("%1").arg(tr("To select a disk")));
    m_warning3->setDesc(QString("%1.")
                        .arg(tr("Nothing to do")));
}

void installer::PartitionTableWarningFramePrivate::keyEventTriger(int key)
{
    Q_Q(PartitionTableWarningFrame);

    switch (key) {
        case KEY_UP:
        if (m_warning3->isOnFoucs()) {
            m_warning3->setFocus(false);
            m_warning2->setFocus(true);
        } else if (m_warning2->isOnFoucs()) {
            m_warning2->setFocus(false);
            m_warning1->setFocus(true);
        }
        break;
        case KEY_DOWN:
        if (m_warning1->isOnFoucs()) {
            m_warning1->setFocus(false);
            m_warning2->setFocus(true);
        } else if (m_warning2->isOnFoucs()) {
            m_warning2->setFocus(false);
            m_warning3->setFocus(true);
        }
        break;
        case KEY_ENTER_OTHER:
        if (m_warning1->isOnFoucs()) {
            Q_EMIT q->reboot();
        } else if (m_warning2->isOnFoucs()) {
            Q_EMIT q->formatting();
        }else if (m_warning3->isOnFoucs()) {
            Q_EMIT q->cancel();
        }
        break;
    }
}


