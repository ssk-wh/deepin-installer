#include "install_process_frame.h"
#include "ui/ncurses_widgets/ncurses_progressbar.h"
#include "ui/ncurses_widgets/ncurses_text_brower.h"

#include "base/file_util.h"
#include "base/thread_util.h"
#include "service/hooks_manager.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"

#include <QTimer>
#include <QThread>

#include "cursesp.h"
#include "cursesm.h"

namespace installer {

InstallProcessFramePrivate::InstallProcessFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      m_progressvalue(0),
      m_NcursesProgressBar(nullptr),
      m_NcursesTextBrower(nullptr),
      hooks_manager_(nullptr),
      hooks_manager_thread_(nullptr),
      m_isshow(false),
      m_processTimer(new QTimer(this))
{
    initUI();
    initConnection();
}

InstallProcessFramePrivate::~InstallProcessFramePrivate()
{
    if(hooks_manager_ != nullptr) {
        delete hooks_manager_;
        hooks_manager_ = nullptr;
    }

    if(hooks_manager_thread_ != nullptr) {
        delete hooks_manager_thread_;
        hooks_manager_thread_ = nullptr;
    }
}

void InstallProcessFramePrivate::initUI()
{
    setBackground(NcursesUtil::getInstance()->dialog_attr());
    this->drawShadow(true);
    this->box();

    m_NcursesTextBrower = new NcursesTextBrower(this, height() - 10, width() - 5, begy() + 2, begx() + 5 / 2);
    //m_NcursesTextBrower->hide();

    m_NcursesProgressBar = new NcursesProgressBar(this, 3, width(), begy() + height() - 3, begx());
    m_NcursesProgressBar->setRange(100);
    //m_NcursesProgressBar->hide();
    ////connect(this, SIGNAL(signal_timeout()), this, SLOT(slot_timeout()));
}

void InstallProcessFramePrivate::layout()
{

}

void InstallProcessFramePrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    printTitle(::QObject::tr("Installing"), width());
    FrameInterfacePrivate::updateTs();
}

void InstallProcessFramePrivate::show()
{
    if (!m_isshow) {
        QString testbrowertext = ::QObject::tr("Installing") + QString(" ... ");//QString("Do parttition ...");
        m_NcursesTextBrower->appendItemText(testbrowertext, false);
        m_NcursesTextBrower->scrollToEnd();

        FrameInterfacePrivate::show();
        m_isshow = true;
    }
}

void InstallProcessFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void InstallProcessFramePrivate::initConnection()
{
}

bool InstallProcessFramePrivate::validate()
{
    return true;
}

void InstallProcessFramePrivate::onKeyPress(int keyCode)
{
    switch (keyCode) {
    case KEY_TAB:
        switchChildWindowsFoucs();
        break;
    case KEY_DOWN:
        emit signal_timeout();
        break;
    default:
        break;
    }
    NCursesWindowBase::onKeyPress(keyCode);
}

void InstallProcessFramePrivate::onHooksErrorOccurred()
{
    failed_ = true;
    WriteInstallSuccessed(false);
    m_progressvalue = 1;
    m_NcursesProgressBar->reSet();
    //tiao zhuan zhi xia yi ge jie mian
    hide();
    next();
}

void InstallProcessFramePrivate::onHooksFinished()
{
    failed_ = false;

    // Set progress value to 100 explicitly.
    onProgressUpdate(100);
    m_progressvalue = 1;
    m_NcursesProgressBar->reSet();
    WriteInstallSuccessed(true);
    hide();
    next();
}

void InstallProcessFramePrivate::onProgressUpdate(int progress)
{
    m_NcursesProgressBar->setValue(progress);
}

void InstallProcessFramePrivate::startInstall()
{
//    QString testbrowertext = QString("partion complete");
//    m_NcursesTextBrower->appendItemText(testbrowertext, false);
//    m_NcursesTextBrower->scrollToEnd();
    m_NcursesProgressBar->setValue(5);

    hooks_manager_ = new HooksManager();
    hooks_manager_thread_ = new QThread(this);

    connect(hooks_manager_, &HooksManager::errorOccurred, this, &InstallProcessFramePrivate::onHooksErrorOccurred);
    connect(hooks_manager_, &HooksManager::finished, this, &InstallProcessFramePrivate::onHooksFinished);
    connect(hooks_manager_, &HooksManager::processUpdate, this, &InstallProcessFramePrivate::onProgressUpdate);
    connect(hooks_manager_thread_, &QThread::finished, hooks_manager_, &HooksManager::deleteLater);

#ifdef NDEBUG
    hooks_manager_->moveToThread(hooks_manager_thread_);
    hooks_manager_thread_->start();
    emit hooks_manager_->runHooks();
#endif

}

void InstallProcessFramePrivate::slot_timeout()
{
    if(m_progressvalue > 98)
    {
        m_progressvalue = 1;
        m_NcursesProgressBar->reSet();
        next();
    } else {
        m_progressvalue++;
        QString testbrowertext = QString("").arg(m_progressvalue);
        m_NcursesTextBrower->appendItemText(testbrowertext, false);
        m_NcursesTextBrower->scrollToEnd();
    }
    m_NcursesProgressBar->setValue(m_progressvalue);
}


InstallProcessFrame::InstallProcessFrame(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = MAINWINDOW_HEIGHT;//LINES / 2;
    int w = MAINWINDOW_WIDTH;//COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new InstallProcessFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
    //m_private->hide();
}

InstallProcessFrame::~InstallProcessFrame()
{

}


bool InstallProcessFrame::init()
{
    Q_D(InstallProcessFrame);
    if (m_currState == FRAME_STATE_NOT_START) {
        m_private->layout();
        m_currState = FRAME_STATE_RUNNING;
    }
    return true;
}

QString InstallProcessFrame::getFrameName()
{
    return "InstallProcessFrame";
}

void InstallProcessFrame::startInstall()
{
    Q_D(InstallProcessFrame);
    d->startInstall();
}

bool InstallProcessFrame::handle()
{
    return true;
}

}
