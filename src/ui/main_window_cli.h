#ifndef INSTALLER_UI_CHAR_MAIN_WINDOW_CLI_H
#define INSTALLER_UI_CHAR_MAIN_WINDOW_CLI_H


#include "ui/interfaces_cli/frameinterface.h"
#include "ui/frames_cli/privilege_error_frame.h"
#include "ui/frames_cli/language_frame.h"
#include "ui/frames_cli/install_component_frame.h"
#include "ui/frames_cli/disk_space_insufficient_frame.h"
#include "ui/frames_cli/virtual_machine_frame.h"
#include "ui/frames_cli/system_info_frame.h"
#include "ui/frames_cli/time_zone_frame.h"
#include "ui/frames_cli/partition_frame.h"
#include "ui/frames_cli/install_process_frame.h"
#include "ui/frames_cli/install_success_frame.h"
#include "ui/frames_cli/licence_frame.h"
#include "ui/frames_cli/network_frame.h"
#include "ui/frames_cli/keyboard_frame.h"
#include "ui/frames_cli/inner/create_root_user_frame.h"

#include <QThread>
#include <QTranslator>

namespace installer {

class NetwrokFrame;
class KeyInput;

class MainWindowPrivate : public FrameInterfacePrivate
{
public:
    MainWindowPrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX) : FrameInterfacePrivate(parent, lines, cols, beginY, beginX) {
        setlocale(LC_ALL, "");
    }
    void initUI() override;
    void updateTs() override;
    void show() override;

    void update(const QString &);
};

class MainWindow : public FrameInterface
{
    Q_OBJECT
public:
    explicit MainWindow(QObject* parent = nullptr);
    QString getFrameName() override;
    bool init() override;
    void setEnableAutoInstall(bool auto_install);
    // Set filepath to which log file will be backup.
    void setLogFile(const QString& log_file);
    void initConnection() override;

    // Notify background thread to scan disk devices if needed.
    // And read current timezone.
    void scanDevicesAndTimezone();

private slots:
    void slot_languageChange();
    void slot_dostartInstall();
    void slot_failFinished();
    void slot_successFinished();

protected:
    bool handle() override;
private:
    QThread* m_keyinputcheck = nullptr;
    KeyInput* m_KeyInput = nullptr;
    QTranslator* m_translator = nullptr;
    QTranslator* m_translator_cli = nullptr;
    PrivilegeErrorFrame* m_privilegeErrrorFrame = nullptr;
    LanguageFrame* m_languageFrame = nullptr;
    ComponentFrame *m_componentFrame = nullptr;
    LicenceFrame* m_licenceFrame = nullptr;
    DiskSpaceInsufficient* m_diskSpaceInsufficientFrame = nullptr;
    VirtualMachineFrame* m_virtualMachineFrame = nullptr;
    SystemInfoFrame* m_systemInfoFrame = nullptr;
    CreateRootUserFrame* m_systemInfoRootPaswordFrame = nullptr;
    TimeZoneFrame* m_timeZoneFrame = nullptr;
    PartitionFrame* m_partitionFrame = nullptr;
    NetwrokFrame* m_networkFrame = nullptr;
    InstallProcessFrame* m_installProcessFrame = nullptr;
    InstallSuccessFrame* m_installSuccessFrame = nullptr;
    KeyboardFrame* m_keyboardFrame = nullptr;

    Q_DECLARE_PRIVATE_D(m_private, MainWindow)
};



}



#endif
