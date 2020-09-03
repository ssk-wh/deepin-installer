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

class KeyInput : public FrameInterfacePrivate
{
public:
    explicit KeyInput(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
        : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
    {
        fcntl(0,F_SETFL, O_NONBLOCK);
    }
    virtual ~KeyInput(){}

public:
    void getKeyInputRun() {
        int testcount = 0;
        while (true) {
            //int key = getKey();
            int key = getch();

            if (key != -1) {
                if (m_keys.size() > 0) {
                    if (key != m_keys.last()) {
                        m_keys.push_back(key);
                    }
                } else {
                    m_keys.push_back(key);
                }
            }


            if ((m_keys.size() > 0) && (m_FrameInterface->getCurrentChild() != nullptr)) {
                m_FrameInterface->getCurrentChild()->getPrivate()->keyEventTriger(m_keys.front());
                m_keys.pop_front();
            }

            QThread::msleep(50);
        }
    }
    void setFrameInterface(FrameInterface* object){ m_FrameInterface = object; }

private:
    QQueue<int> m_keys;
    FrameInterface* m_FrameInterface = nullptr;
};


void MainWindowPrivate::initUI()
{
    setBackground(NcursesUtil::getInstance()->screen_attr());
}

void MainWindowPrivate::updateTs()
{
    erase();
    addstr(0, 1, QString(::QObject::tr("Welcome to install UOS")).toUtf8().data());

    QString text = QString("<↑ ↓>%1  | <Tab>%2 |<Enter>%3")
                   .arg(::QObject::tr("Select Item"))
                   .arg(::QObject::tr("Change Field"))
                   .arg(::QObject::tr("Confirm"));
    update(text);
}

void MainWindowPrivate::show()
{
}

void MainWindowPrivate::update(const QString &text)
{
    erase();
    addstr(begy() + lines() - 1,  (width() - std::min(width() - 2, text.length())) / 2, text.toUtf8().data());
    refresh();
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

    WriteNecuresCliInstallMode(true);
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
    Q_D(MainWindow);

    connect(m_systemInfoFrame, &SystemInfoFrame::createRoot, m_systemInfoRootPaswordFrame, &CreateRootUserFrame::setShoulDispaly);

    connect(m_systemInfoFrame, &SystemInfoFrame::userName, m_systemInfoRootPaswordFrame, &CreateRootUserFrame::setUserName);
    connect(m_systemInfoFrame, &SystemInfoFrame::userPassword, m_systemInfoRootPaswordFrame, &CreateRootUserFrame::setUserPassword);

    connect(this, &MainWindow::update, this, [=](const QString txet) {
        d->update(txet);
    });

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
        QString tmplocale = QString("%1").arg(locale);
        m_translator->load(GetLocalePath(locale));
        m_translator_cli->load(GetLocalePath(tmplocale));
        qApp->installTranslator(m_translator);
        qApp->installTranslator(m_translator_cli);
    } else {
        if (!locale.contains("zh")) {
            m_translator->load(GetLocalePath(locale));
            qApp->installTranslator(m_translator);
        }
    }

    Q_D(MainWindow);
    d->updateTs();

    if ((m_timeZoneFrame != nullptr) && (m_languageFrame != nullptr)) {
        m_timeZoneFrame->setDefaultTimezone(m_languageFrame->getCurrentLanguageTimezone());
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
        //addChildFrame(m_privilegeErrrorFrame);

        //m_virtualMachineFrame = new VirtualMachineFrame(this);
        //addChildFrame(m_virtualMachineFrame);

        m_languageFrame = new LanguageFrame(this);
        connect(m_languageFrame, &LanguageFrame::languageChanged, this, &MainWindow::slot_languageChange);
        addChildFrame(m_languageFrame);

        m_licenceFrame = new LicenceFrame(this);
        addChildFrame(m_licenceFrame);

        m_keyboardFrame = new KeyboardFrame(this);
        addChildFrame(m_keyboardFrame);

        m_networkFrame = new NetwrokFrame(this);
        addChildFrame(m_networkFrame);

        m_timeZoneFrame = new TimeZoneFrame(this);
        addChildFrame(m_timeZoneFrame);

        m_componentFrame = new ComponentFrame(this);
        addChildFrame(m_componentFrame);

        m_systemInfoFrame = new SystemInfoFrame(this);
        addChildFrame(m_systemInfoFrame);

        m_systemInfoRootPaswordFrame = new CreateRootUserFrame(this);
        addChildFrame(m_systemInfoRootPaswordFrame);

        //m_diskSpaceInsufficientFrame = new DiskSpaceInsufficient(this);
        //addChildFrame(m_diskSpaceInsufficientFrame);

        m_partitionFrame = new PartitionFrame(this);
        connect( static_cast<PartitionFramePrivate*>(m_partitionFrame->getPrivate()),
                 &PartitionFramePrivate::dostartInstall, this, &MainWindow::slot_dostartInstall);
        addChildFrame(m_partitionFrame);

        m_installProcessFrame = new InstallProcessFrame(this);
        addChildFrame(m_installProcessFrame);

        m_installSuccessFrame = new InstallSuccessFrame(this);
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
