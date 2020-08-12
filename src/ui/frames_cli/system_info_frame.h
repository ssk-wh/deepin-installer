#ifndef CLI_INSTALL_SYSTEM_INFO_FRAME_H
#define CLI_INSTALL_SYSTEM_INFO_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_line_edit.h"
#include "ui/ncurses_widgets/ncurses_checkbox.h"

namespace installer {

class SystemInfoFrame;

class SystemInfoFramePrivate : public FrameInterfacePrivate
{
    friend SystemInfoFrame;
    Q_OBJECT
public:
    SystemInfoFramePrivate(SystemInfoFrame* parent, int lines, int cols, int beginY, int beginX);
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;
    void onKeyPress(int keyCode) override;

public:
    void readConf();
    void writeConf();

protected:
    void downHandle() override;
    void upHandle() override;
    void switchChildWindowsFoucs() override;

private:
    bool validateUsername(QString& msg);
    bool validateHostname(QString& msg);
    bool validatePassword(NCursesLineEdit *passwordEdit, QString& msg);
    bool validatePassword2(NCursesLineEdit* passwordEdit, NCursesLineEdit* passwordCheckEdit, QString& msg);
    void showError(const QString &msg);

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
    NcursesCheckBox* m_NcursesCheckBox = nullptr;
    QString m_localeString = "";
    bool m_isshow = false;
    bool m_isHostEdited = false;

    SystemInfoFrame *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(SystemInfoFrame)
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

signals:
    void createRoot(bool isCreate);
    void userName(const QString &);
    void userPassword(const QString &);

protected:
    bool handle() override;
    Q_DECLARE_PRIVATE_D(m_private, SystemInfoFrame)
};


}


#endif
