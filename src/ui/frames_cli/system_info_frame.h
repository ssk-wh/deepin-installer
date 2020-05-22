#ifndef CLI_INSTALL_SYSTEM_INFO_FRAME_H
#define CLI_INSTALL_SYSTEM_INFO_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_line_edit.h"

namespace installer {

class SystemInfoFrame;

class SystemInfoFramePrivate : public FrameInterfacePrivate
{
    friend SystemInfoFrame;
    Q_OBJECT
public:
    SystemInfoFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX)
        : FrameInterfacePrivate(parent, lines, cols, beginY, beginX)
    {
        initUI();
        initConnection();
        updateTs();
    }

    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;

public:
    void readConf();
    void writeConf();

private:
    bool validateUsername(QString& msg);
    bool validateHostname(QString& msg);
    bool validatePassword(NCursesLineEdit *passwordEdit, QString& msg);
    bool validatePassword2(NCursesLineEdit* passwordEdit, NCursesLineEdit* passwordCheckEdit, QString& msg);


private:

private:
    NcursesLabel* m_label_title = nullptr;
    NcursesLabel* m_label_instructions = nullptr;
    NcursesLabel* m_label_hostname = nullptr;
    NcursesLabel* m_label_username = nullptr;
    NcursesLabel* m_label_password = nullptr;
    NcursesLabel* m_label_password_confirm = nullptr;
    NcursesLabel* m_label_root_password = nullptr;
    NcursesLabel* m_label_root_password_confirm = nullptr;
    NCursesLineEdit* m_le_hostname = nullptr;
    NCursesLineEdit* m_le_username = nullptr;
    NCursesLineEdit* m_le_password = nullptr;
    NCursesLineEdit* m_le_password_confirm = nullptr;
    NCursesLineEdit* m_le_root_password = nullptr;
    NCursesLineEdit* m_le_root_password_confirm = nullptr;
    NcursesLabel*    m_label_error_info = nullptr;
    bool m_isshow = false;
};

class SystemInfoFrame : public FrameInterface
{
    Q_OBJECT

public:
    SystemInfoFrame(FrameInterface* parent);
    virtual ~SystemInfoFrame();

public:
    bool init() override;
    QString getFrameName() override;

protected:
    bool handle() override;
    Q_DECLARE_PRIVATE_D(m_private, SystemInfoFrame)
};


}


#endif