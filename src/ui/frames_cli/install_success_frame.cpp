#include "install_success_frame.h"

#include "ui/ncurses_widgets/ncurses_text_brower.h"

#include "service/settings_manager.h"
#include "service/settings_name.h"

namespace installer {

InstallSuccessFramePrivate::InstallSuccessFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      m_installresultTextBrower(nullptr),
      m_isshow(false),
      m_installSuccessTitle(QObject::tr("Install sucess")),
      m_installSuccessInfoTitle(QObject::tr("[Done]")),
      m_installSuccessInfoDes(QObject::tr("[Successfully Installed]")),
      m_installSuccessInfoTodo(QObject::tr("[Clik the button below and then remove the installation media immediately]")),
      m_installFailedTitle(QObject::tr("Install failed")),
      m_installFailedInfoTitle(QObject::tr("[Installation Failed]")),
      m_installFailedInfoDes(QObject::tr("[Sorry for the trouble. Please photo or scan the QR code to send us the error log, or save the log to an external disk. We will help solve the issue]"))
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

    m_installresultTextBrower = new NcursesTextBrower(this, height() - 10, width() - 5, begy() + 1, begx() + 1);
    m_installresultTextBrower->setBackground(this->background());

    //QString strBack = QObject::tr("back");
    QString strNext = QObject::tr("Reboot Now");

    int buttonHeight = 3;
    int buttonWidth = strNext.length() + 4;//std::max(strNext.length(), strBack.length()) + 4;

    m_pNextButton = new NcursesButton(this, strNext, buttonHeight,
                                    buttonWidth, begy() + height() - buttonHeight - 2, begx() + (width() - buttonWidth) / 2);
    m_pNextButton->drawShadow(true);
    m_pNextButton->box();
    m_pNextButton->setObjectName(strNext);
}

void InstallSuccessFramePrivate::updateTs()
{
    box(ACS_VLINE, ACS_HLINE);
    m_installSuccessTitle     = QObject::tr("Successfully Installed");
    m_installSuccessInfoTitle = QObject::tr("[Done]");
    m_installSuccessInfoDes   = QObject::tr("[Successfully Installed]");
    m_installSuccessInfoTodo  = QObject::tr("[Clik the button below and then remove the installation media immediately]");
    m_installFailedTitle      = QObject::tr("Installation Failed");
    m_installFailedInfoTitle  = QObject::tr("[Installation Failed]");
    m_installFailedInfoDes    = QObject::tr("[Sorry for the trouble. Please photo or scan the QR code to send us the error log, or save the log to an external disk. We will help solve the issue]");

    bool testissuccess = GetSettingsBool("DI_INSTALL_SUCCESSED");

    if(testissuccess) {
       printTitle(m_installSuccessTitle, width());
       m_installresultTextBrower->clearText();
       m_installresultTextBrower->appendItemText(m_installSuccessInfoTitle);
       m_installresultTextBrower->appendItemText(m_installSuccessInfoDes);
       m_installresultTextBrower->appendItemText(m_installSuccessInfoTodo);
       QString strNext = QObject::tr("Reboot Now");
       m_pNextButton->setText(strNext);
    } else {
       m_installresultTextBrower->clearText();
       printTitle(m_installFailedTitle, width());
       m_installresultTextBrower->appendItemText(m_installFailedInfoTitle);
       m_installresultTextBrower->appendItemText(m_installFailedInfoDes);
       QString strNext = QObject::tr("Shut down");
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

void InstallSuccessFramePrivate::doBackBtnClicked()
{

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
    QString strNext = m_pNextButton->text();
    int buttonHeight = 3;
    int buttonWidth  = 2;

    if (installer::ReadLocale() == "zh_CN") {
        buttonWidth = strNext.length() * 2 + 4 * 2;
    } else {
        buttonWidth = strNext.length() + 4;
    }

    m_pNextButton->erase();
    m_pNextButton->resizew(buttonHeight, buttonWidth);
    m_pNextButton->resetBackground();
    m_pNextButton->box(ACS_VLINE,ACS_HLINE);
    m_pNextButton->setText(strNext);
    //_pNextButton->mvwin(begy() + height() - buttonHeight - 2, begx() + (width() - buttonWidth) / 2);
    m_pNextButton->show();

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
    return true;
}

QString InstallSuccessFrame::getFrameName()
{
    return QObject::tr("InstallSuccessFrame");
}

bool InstallSuccessFrame::handle()
{
    return true;
}

}
