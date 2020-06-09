#include "main_window_cli.h"
#include <QDebug>
#include <QString>
#include <QThread>
#include <QTranslator>
#include <QApplication>
#include "service/settings_manager.h"
#include "ui/ncurses_widgets/ncurses_windows_base.h"
#include "ui/delegates/language_delegate.h"
#include "ui/frames_cli/network_frame.h"
#include "ui/frames_cli/inner/partition_table_warning_frame.h"
#include "service/power_manager.h"

namespace installer {

/*void CopyLogFile(const QString& log_file) {
  const QString current_log_file(GetLogFilepath());
  if (!CopyFile(current_log_file, log_file, true)) {
    qCritical() << "Failed to copy log to:" << log_file;
  }
}*/

class KeyInput : public FrameInterfacePrivate
{
public:
    explicit KeyInput(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
        : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
    {
    }
    virtual ~KeyInput(){}

public:
    void getKeyInputRun() {
        int testcount = 0;
        while (true) {
            if(testcount < 5) {
                QThread::msleep(10);
                testcount++;
                continue;
            }
            int key = getKey();
            testcount = 0;
            if(m_FrameInterface->getCurrentChild() != nullptr)
                m_FrameInterface->getCurrentChild()->getPrivate()->keyEventTriger(key);

            while(testcount < 5) {
                QThread::msleep(10);
                testcount++;
                continue;
            }
            testcount = 0;
        }
    }
    void setFrameInterface(FrameInterface* object){ m_FrameInterface = object; }
    FrameInterface* m_FrameInterface = nullptr;
};


void MainWindowPrivate::initUI()
{
    setBackground(NcursesUtil::getInstance()->screen_attr());
    addstr(0, 1, QString(::QObject::tr("Welcome to install UOS")).toUtf8().data());
    QString keyManual = QString("<↑ ↓ ← →>移动  | <Tab>切换 |<Enter> 确定 |<Space>选中")
            .arg(::QObject::tr("Select Item"))
            .arg(::QObject::tr("Change Field"))
            .arg(::QObject::tr("Confirm"))
            .arg(::QObject::tr("Select"));
    addstr(begy() + lines() - 1,  (width() - std::min(width() - 2, keyManual.length())) / 2, keyManual.toUtf8().data());
    refresh();
}

void MainWindowPrivate::updateTs()
{
    erase();
    addstr(0, 1, QString(::QObject::tr("Welcome to install UOS")).toUtf8().data());
    QString keyManual = QString("<↑ ↓ ← →>移动  | <Tab>切换 |<Enter> 确定 |<Space>选中")
            .arg(::QObject::tr("Select Item"))
            .arg(::QObject::tr("Change Field"))
            .arg(::QObject::tr("Confirm"))
            .arg(::QObject::tr("Select"));
    addstr(begy() + lines() - 1,  (width() - std::min(width() - 2, keyManual.length())) / 2, keyManual.toUtf8().data());
}

void MainWindowPrivate::show()
{
    erase();
    addstr(0, 1, QString(::QObject::tr("Welcome to install UOS")).toUtf8().data());
    QString keyManual = QString("<↑ ↓ ← →>移动  | <Tab>切换 |<Enter> 确定 |<Space>选中")
            .arg(::QObject::tr("Select Item"))
            .arg(::QObject::tr("Change Field"))
            .arg(::QObject::tr("Confirm"))
            .arg(::QObject::tr("Select"));
    addstr(begy() + lines() - 1,  (width() - std::min(width() - 2, keyManual.length())) / 2, keyManual.toUtf8().data());
}


MainWindow::MainWindow(QObject* parent)
{
    m_keyinputcheck = new QThread(this);
    m_KeyInput = new KeyInput(nullptr, LINES, COLS, 0, 0);
    m_private = new MainWindowPrivate(nullptr, LINES, COLS, 0, 0);

    m_KeyInput->setFrameInterface(this);
    connect(m_keyinputcheck, &QThread::started, m_KeyInput, &KeyInput::getKeyInputRun);
    m_KeyInput->moveToThread(m_keyinputcheck);
    m_keyinputcheck->start();
}


QString MainWindow::getFrameName()
{
    return "MainWindow";
}

bool MainWindow::handle()
{
    m_currState = FRAME_STATE_RUNNING;
    return true;
}



void MainWindow::setEnableAutoInstall(bool auto_install)
{

}

void MainWindow::setLogFile(const QString &log_file)
{

}

void MainWindow::initConnection()
{
    connect(m_systemInfoFrame, &SystemInfoFrame::createRoot, m_systemInfoRootPaswordFrame, &CreateRootUserFrame::setShoulDispaly);

    connect(m_systemInfoFrame, &SystemInfoFrame::userName, m_systemInfoRootPaswordFrame, &CreateRootUserFrame::setUserName);
    connect(m_systemInfoFrame, &SystemInfoFrame::userPassword, m_systemInfoRootPaswordFrame, &CreateRootUserFrame::setUserPassword);

    return FrameInterface::initConnection();
}

void MainWindow::scanDevicesAndTimezone()
{

}

void MainWindow::slot_languageChange()
{
    if(m_translator == nullptr) {
        m_translator = new QTranslator(this);
    }

    if(m_translator_cli == nullptr) {
        m_translator_cli = new QTranslator(this);
    }

    qApp->removeTranslator(m_translator);
    qApp->removeTranslator(m_translator_cli);

    const QString locale(ReadLocale());
    if(!locale.compare("zh_CN")) {
        QString tmplocale = QString("cli_%1").arg(locale);
        m_translator->load(GetLocalePath(locale));
        m_translator_cli->load(GetLocalePath(tmplocale));
        qApp->installTranslator(m_translator);
        qApp->installTranslator(m_translator_cli);
    } else {
        m_translator->load(GetLocalePath(locale));
        qApp->installTranslator(m_translator);
    }


}

void MainWindow::slot_dostartInstall()
{
    if(m_installProcessFrame != nullptr){
        m_installProcessFrame->startInstall();
    }
}

void MainWindow::slot_failFinished()
{
    //this->saveLogFile();

    if (!ShutdownSystemWithMagicKey()) {
        qWarning() << "ShutdownSystemWithMagicKey() failed!";
    }

    if (!ShutdownSystem()) {
        qWarning() << "ShutdownSystem() failed!";
    }
}

void MainWindow::slot_successFinished()
{
    //this->saveLogFile();

    if (!RebootSystemWithMagicKey()) {
        qWarning() << "RebootSystemWithMagicKey() failed!";
    }

    if (!RebootSystem()) {
        qWarning() << "RebootSystem() failed!";
    }
}

// Page order:
//   * privilege error frame;
//   * select language frame;
//   * disk space insufficient page;
//   * virtual machine page;
//   * system info page;
//   * timezone page;
//   * partition page;
//   * install progress page;
//   * install success page or install failed page;
// And confirm-quit-page can be triggered at any moment except in
// install progress page.

bool MainWindow::init()
{
    slot_languageChange();
    // Set language.
    if (m_currState == FRAME_STATE_NOT_START) {
        m_private->initUI();
        //m_privilegeErrrorFrame = new PrivilegeErrorFrame(this);
        //m_privilegeErrrorFrame->hide();
        //addChildFrame(m_privilegeErrrorFrame);

        //m_virtualMachineFrame = new VirtualMachineFrame(this);
        //m_virtualMachineFrame->hide();
        //addChildFrame(m_virtualMachineFrame);

        m_languageFrame = new LanguageFrame(this);
        m_languageFrame->hide();
        connect(m_languageFrame, &LanguageFrame::languageChanged, this, &MainWindow::slot_languageChange);
        addChildFrame(m_languageFrame);

        m_licenceFrame = new LicenceFrame(this);
        m_licenceFrame->hide();
        addChildFrame(m_licenceFrame);

        m_keyboardFrame = new KeyboardFrame(this);
        m_keyboardFrame->hide();
        addChildFrame(m_keyboardFrame);

        m_networkFrame = new NetwrokFrame(this);
        m_networkFrame->hide();
        addChildFrame(m_networkFrame);

        m_timeZoneFrame = new TimeZoneFrame(this);
        m_timeZoneFrame->hide();
        addChildFrame(m_timeZoneFrame);

        m_componentFrame = new ComponentFrame(this);
        m_componentFrame->hide();
        addChildFrame(m_componentFrame);

        m_systemInfoFrame = new SystemInfoFrame(this);
        m_systemInfoFrame->hide();
        addChildFrame(m_systemInfoFrame);

        m_systemInfoRootPaswordFrame = new CreateRootUserFrame(this);
        m_systemInfoRootPaswordFrame->hide();
        addChildFrame(m_systemInfoRootPaswordFrame);

        //m_diskSpaceInsufficientFrame = new DiskSpaceInsufficient(this);
        //addChildFrame(m_diskSpaceInsufficientFrame);

        m_partitionFrame = new PartitionFrame(this);
        m_partitionFrame->hide();
        connect( static_cast<PartitionFramePrivate*>(m_partitionFrame->getPrivate()),
                 &PartitionFramePrivate::dostartInstall, this, &MainWindow::slot_dostartInstall);
        addChildFrame(m_partitionFrame);

        m_installProcessFrame = new InstallProcessFrame(this);
        m_installProcessFrame->hide();
        addChildFrame(m_installProcessFrame);

        m_installSuccessFrame = new InstallSuccessFrame(this);
        m_installSuccessFrame->hide();
        connect( static_cast<InstallSuccessFramePrivate*>(m_installSuccessFrame->getPrivate()),
                 &InstallSuccessFramePrivate::failFinished, this, &MainWindow::slot_failFinished);
        connect( static_cast<InstallSuccessFramePrivate*>(m_installSuccessFrame->getPrivate()),
                 &InstallSuccessFramePrivate::successFinished, this, &MainWindow::slot_successFinished);
        addChildFrame(m_installSuccessFrame);

        initConnection();
    }


    return true;

}



}
