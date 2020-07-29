#include "install_success_frame.h"

#include "ui/ncurses_widgets/ncurses_text_brower.h"
#include "ui/frames_cli/inner/savelog_frame.h"

#include "service/settings_manager.h"
#include "service/settings_name.h"

namespace installer {

InstallSuccessFramePrivate::InstallSuccessFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      m_installresultTextBrower(nullptr),
      m_isshow(false),
      m_currentchoicetype(-1),
      m_installSuccessTitle(::QObject::tr("Successfully Installed")),
      m_installSuccessInfoTitle(::QObject::tr("Done")),
      m_installSuccessInfoDes(::QObject::tr("Successfully Installed")),
      m_installSuccessInfoTodo(::QObject::tr("Click the button below and then remove the installation media immediately")),
      m_installFailedInfoTitle(::QObject::tr("Installation Failed")),
      m_installFailedInfoDes(::QObject::tr("Sorry for the trouble. Please take a photo to send us the error log, or save the log to an external disk. We will help solve the issue."))
{
    initUI();
    initConnection();
}

InstallSuccessFramePrivate::~InstallSuccessFramePrivate()
{
    if(m_installresultTextBrower != nullptr) {
        delete m_installresultTextBrower;
        m_installresultTextBrower = nullptr;
    }
}

void InstallSuccessFramePrivate::initUI()
{
    setBackground(NcursesUtil::getInstance()->dialog_attr());
    this->drawShadow(true);
    this->box();

    m_installresultTextBrower = new NcursesTextBrower(this, height() - 10, width() - 2 , begy() + 1, begx() + 1);
    m_installresultTextBrower->setBackground(this->background());
    m_installresultTextBrower->setFocusEnabled(false);

    QString strBack = ::QObject::tr("Save Log");
    QString strNext = ::QObject::tr("Reboot Now");

    int buttonHeight = 3;
    int buttonWidth_backbtn = strBack.length() + 4;
    int buttonWidth_nextbtn = strNext.length() + 4;

    m_pBackButton = new NcursesButton(this, strNext, buttonHeight,
                                    buttonWidth_backbtn, begy() + height() - buttonHeight - 2, begx() + (width() / 2) + 2);
    m_pBackButton->drawShadow(true);
    m_pBackButton->box();
    m_pBackButton->setObjectName(strNext);

    m_pNextButton = new NcursesButton(this, strNext, buttonHeight,
                                    buttonWidth_nextbtn, begy() + height() - buttonHeight - 2, begx() + (width() / 2) - buttonWidth_nextbtn -2);
    m_pNextButton->drawShadow(true);
    m_pNextButton->box();
    m_pNextButton->setObjectName(strNext);
}

void InstallSuccessFramePrivate::updateTs()
{
    box(ACS_VLINE, ACS_HLINE);

    m_installSuccessTitle     = ::QObject::tr("Successfully Installed");
    m_installSuccessInfoTitle = ::QObject::tr("Done");
    m_installSuccessInfoDes   = ::QObject::tr("Successfully Installed");
    m_installSuccessInfoTodo  = ::QObject::tr("Click the button below and then remove the installation media immediately");
    m_installFailedTitle      = ::QObject::tr("Installation Failed");
    m_installFailedInfoTitle  = ::QObject::tr("Installation Failed");
    m_installFailedInfoDes    = ::QObject::tr("Sorry for the trouble. Please take a photo to send us the error log, or save the log to an external disk. We will help solve the issue.");

#ifdef QT_DEBUG_test
    bool testissuccess = true;
#else
    bool testissuccess = GetSettingsBool("DI_INSTALL_SUCCESSED");
#endif // QT_DEBUG

    if(testissuccess) {
       printTitle(m_installSuccessTitle, width());
       m_installresultTextBrower->clearText();
       //m_installresultTextBrower->appendItemText(m_installSuccessInfoTitle);
       //m_installresultTextBrower->appendItemText(m_installSuccessInfoDes);
       m_installresultTextBrower->appendItemText(m_installSuccessInfoTodo);
       QString strNext = ::QObject::tr("Reboot Now");
       m_pNextButton->setText(strNext);
    } else {
       m_installresultTextBrower->clearText();
       printTitle(m_installFailedTitle, width());
       //m_installresultTextBrower->appendItemText(m_installFailedInfoTitle);
       m_installresultTextBrower->appendItemText(m_installFailedInfoDes);
       //QString strNext = ::QObject::tr("Shut Down");
       QString strSaveLog = ::QObject::tr("Save Log");
       QString strNext = ::QObject::tr("Exit");
       m_pBackButton->setText(strSaveLog);
       m_pNextButton->setText(strNext);
    }

    layout();
}

void InstallSuccessFramePrivate::initConnection()
{
    connect(m_pBackButton, SIGNAL(clicked()), this, SLOT(doBackBtnClicked()));
    connect(m_pNextButton, SIGNAL(clicked()), this, SLOT(doNextBtnClicked()));
}

bool InstallSuccessFramePrivate::validate()
{
    return true;
}

void InstallSuccessFramePrivate::show()
{
    if(!m_isshow) {
        NCursesWindowBase::show();
        m_isshow = true;
    }
}

void InstallSuccessFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void InstallSuccessFramePrivate::onKeyPress(int keyCode)
{

}

void InstallSuccessFramePrivate::keyEventTriger(int keycode)
{
    if (m_currentchoicetype != -1) {
        emit keyEventTrigerSignal(keycode);
    } else {
        FrameInterfacePrivate::keyEventTriger(keycode);
    }
}

void InstallSuccessFramePrivate::setValue()
{
    m_currentchoicetype = -1;
    m_pBackButton->setFocusEnabled(true);
    m_pNextButton->setFocusEnabled(true);
    m_pNextButton->setFocus(true);
}

void InstallSuccessFramePrivate::doBackBtnClicked()
{
    m_currentchoicetype = 0;
    emit showChildSignal(0);
}

void InstallSuccessFramePrivate::doNextBtnClicked()
{
    bool testissuccess = GetSettingsBool("DI_INSTALL_SUCCESSED");
    if (testissuccess) {
        emit successFinished();
    } else {
        emit failFinished();
    }
}

void InstallSuccessFramePrivate::layout()
{
    QString strSaveLog = ::QObject::tr("Save Log");
    QString strNext = ::QObject::tr("Exit");
    int buttonHeight = 3;
    int buttonWidth_savelogbtn = 2;
    int buttonWidth_nextbtn = 2;

    if (installer::ReadLocale() == "zh_CN") {
        buttonWidth_savelogbtn = strSaveLog.length() * 2 + 4 * 2;
        buttonWidth_nextbtn = strNext.length() * 2 + 4 * 2;
    } else {
        buttonWidth_savelogbtn = strSaveLog.length() + 4;
        buttonWidth_nextbtn = strNext.length() + 4;
    }

    bool testissuccess = GetSettingsBool("DI_INSTALL_SUCCESSED");

    if(!testissuccess) {
        m_pBackButton->erase();
        m_pBackButton->resizew(buttonHeight, buttonWidth_savelogbtn);
        m_pBackButton->resetBackground();
        m_pBackButton->box(ACS_VLINE,ACS_HLINE);
        m_pBackButton->setText(strSaveLog);
        m_pBackButton->moveWidowTo(begy() + height() - buttonHeight - 2, begx() + (width() / 2) + 2);
        m_pBackButton->show();
    }

    m_pNextButton->erase();
    m_pNextButton->resizew(buttonHeight, buttonWidth_nextbtn);
    m_pNextButton->resetBackground();
    m_pNextButton->box(ACS_VLINE,ACS_HLINE);
    m_pNextButton->setText(strNext);
    m_pNextButton->moveWidowTo(begy() + height() - buttonHeight - 2, begx() + (width() / 2) - buttonWidth_nextbtn - 2);
    m_pNextButton->show();

    m_pNextButton->setFocus(true);
    m_installresultTextBrower->show();
    m_installresultTextBrower->refresh();
}


InstallSuccessFrame::InstallSuccessFrame(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new InstallSuccessFramePrivate (parent->getPrivate(), h, w, beginY, beginX);

    m_savelogframe = new SaveLogFrame(this);
    m_savelogframe->hide();

    addChildFrame(m_savelogframe);

    Q_D(InstallSuccessFrame);
    connect(d, &InstallSuccessFramePrivate::showChildSignal, this, &InstallSuccessFrame::showChildSlot);
}

InstallSuccessFrame::~InstallSuccessFrame()
{

}


bool InstallSuccessFrame::init()
{
    Q_D(InstallSuccessFrame);
    if (m_currState == FRAME_STATE_NOT_START) {
        m_private->layout();
        m_currState = FRAME_STATE_RUNNING;
    }

    d->setValue();

    return true;
}

QString InstallSuccessFrame::getFrameName()
{
    return "InstallSuccessFrame";
}

void InstallSuccessFrame::showChildSlot(int index)
{
    hide();
    showChild(index);
}

bool InstallSuccessFrame::handle()
{
    return true;
}

}
