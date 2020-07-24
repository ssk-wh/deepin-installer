#ifndef CLI_INSTALL_NETWORK_FRAME_H
#define CLI_INSTALL_NETWORK_FRAME_H


#include "ui/interfaces_cli/frameinterface.h"
#include "ui/delegates/network_operate.h"

namespace dde{
namespace network {
    class NetworkModel;
    class NetworkWorker;
    class NetworkDevice;
}
}

namespace installer {

class NcursesListView;
class NcursesTextBrower;
class NcursesLabel;
class NCursesLineEdit;
class NcursesButton;
class NetworkContronlThread;


struct NetwrokFrameItem{
    NetwrokFrameItem() {
        m_NcursesLabel    = nullptr;
        m_NCursesLineEdit = nullptr;
        m_ErrorinfoLabel  = nullptr;
        m_begy = 0;
        m_begx = 0;
        m_IsOK = false;
        m_IsErrorTextChange = false;
    }

    NcursesLabel* m_NcursesLabel;
    NCursesLineEdit* m_NCursesLineEdit;
    NcursesLabel* m_ErrorinfoLabel;
    int m_begy;
    int m_begx;
    bool m_IsOK;
    bool m_IsErrorTextChange;
};

class NetwrokFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    NetwrokFramePrivate(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX);

    // FrameInterfacePrivate interface
public:
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();
    bool validate() override;
    void show() override;
    void hide() override;
    void initIPConfig();
    bool writeInfoList();
    void updateChoiceType(int type);
    void setFocusEnableType(int type);
    virtual void onKeyPress(int keyCode) override;
    void resetValue();
    void manualConfigure();
    void AutoConfigure();

    void doBackBtnClicked();
    void doNextBtnClicked();
    void editTextCheck(int index, const QString &text);

private slots:
    void slot_EidtTextChange(const QString &text);

private:
    bool m_isshow;
    bool m_isallinputok;
    int m_currentchoicetype;
    int m_currentlineeditindex;
    QStringList m_titledes;
    QString m_networkconfigtypestr;
    QString m_networkconnecterrorstr;
    NcursesTextBrower* m_titledesbrower;
    NcursesLabel* m_childpagecounttext;
    NcursesLabel* m_networkconnecterrorlabel;
    NcursesListView* m_operationchoice;
    QVector<NetwrokFrameItem> m_ipconfigitems;
    NetworkManager::Device::Ptr m_ipv4Device;
    DHCPTYpe m_dhcpType;
    std::list<std::pair<std::function<void (QString)>, QString>> m_trList;
};


class NetwrokFrame : public FrameInterface
{
    Q_OBJECT
public:
    NetwrokFrame(FrameInterface* parent);
    virtual ~NetwrokFrame();

    // FrameInterface interface
public:
    bool init() override;
    QString getFrameName() override;

protected:
    bool handle() override;
    Q_DECLARE_PRIVATE_D(m_private, NetwrokFrame)
};


}


#endif
