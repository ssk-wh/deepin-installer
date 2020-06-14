#include "network_frame.h"

#include "ui/ncurses_widgets/ncurses_text_brower.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_line_edit.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_button.h"
#include "base/utils.h"

#include <QThread>
#include <QNetworkInterface>
//#include <networkworker.h>
//#include <networkmodel.h>
//#include <networkdevice.h>
//#include <wirelessdevice.h>
//#include <wireddevice.h>

#include <QRegularExpression>
#include <QRegularExpressionValidator>


namespace installer {

static uint coverMask(const QString &source)
{
    std::list<char>   mask;
    const QStringList list = source.split(".");

    for (auto it = list.constBegin(); it != list.constEnd(); ++it) {
        const QString &num   = *it;
        QByteArray     array = QString::number(num.toUInt(), 2).toUtf8();
        for (char c : array) {
            mask.push_back(c);
        }
    }

    uint m = 0;
    for (char c : mask) {
        m += QString(c).toUInt();
    }

    return m;
}

NetwrokFramePrivate::NetwrokFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      m_isshow(false),
      m_currentchoicetype(0),
      m_currentlineeditindex(0),
      m_titledes(""),
      m_titledesbrower(nullptr),
      m_networkconfigtypelabel(nullptr),
      m_childpagecounttext(nullptr),
      m_networkconnecterrorlabel(nullptr),
      m_dhcpType(DHCPTYpe::Auto)
{
    initUI();
    initConnection();
}

void NetwrokFramePrivate::initUI()
{
    setBackground(NcursesUtil::getInstance()->dialog_attr());
    //FrameInterfacePrivate::initUI();
    this->drawShadow(true);
    this->box();

    //m_titledes.append(::QObject::tr("  Do network set by auto, use dhcp to set network"));
    //m_titledes.append(::QObject::tr("  Do network set by manual, in this page you can set IP. Mask. Gateway. DNS"));
    //m_titledes.append(::QObject::tr("  Do not set network now, if you not want set the network now, you can do it with the installation complete"));
    m_titledes.append("    " + ::QObject::tr("Configure Ethernet according to your needs, but you can skip it as well."));
    m_titledes.append("    " + ::QObject::tr("Set the IP address, gateway, netmask, DNS please."));

//    m_networkconfigtypestr = ::QObject::tr("Configuration type") + ":";
    m_networkconnecterrorstr = ::QObject::tr("Network connection error, check the configuration please");

    m_titledesbrower = new NcursesTextBrower(this, 3, width() - 2, begy() + 2, begx() + 1);
    m_titledesbrower->setFocusEnabled(false);
    m_titledesbrower->setText(m_titledes[0]);

    m_networkconfigtypelabel = new NcursesLabel(this, m_networkconfigtypestr, 1, m_networkconfigtypestr.length() + 2, begy() + 6, begx() + 1);
    m_networkconfigtypelabel->setFocusEnabled(false);

    m_childpagecounttext = new NcursesLabel(this, " [1/1] ", 1, width() - 2 - m_networkconfigtypelabel->width(), begy() + 6, begx() + 1 + m_networkconfigtypelabel->width());
    m_childpagecounttext->setFocusEnabled(false);

//    NetwrokFrameItem operationchoiceautoset;
//    operationchoiceautoset.m_NcursesLabel = new NcursesLabel(this, 1, width() /2, begy(), begx());
//    operationchoiceautoset.m_NcursesLabel->setFocusEnabled(false);
//    Utils::addTransLate(m_trList, std::bind(&NcursesLabel::setText, operationchoiceautoset.m_NcursesLabel, std::placeholders::_1), QString(::QObject::tr("Network auto set")));

    NetwrokFrameItem operationchoicemanualset;
    operationchoicemanualset.m_NcursesLabel = new NcursesLabel(this, 1, width() /2, begy(), begx());
    operationchoicemanualset.m_NcursesLabel->setFocusEnabled(false);
    operationchoicemanualset.m_NcursesLabel->setFocusStyle(NcursesUtil::getInstance()->list_view_item_select());

    NetwrokFrameItem operationchoicenotset;
    operationchoicenotset.m_NcursesLabel = new NcursesLabel(this, 1, width() /2, begy(), begx());
    operationchoicenotset.m_NcursesLabel->setFocusEnabled(false);
    operationchoicenotset.m_NcursesLabel->setFocusStyle(NcursesUtil::getInstance()->list_view_item_select());

//    m_operationchoice.push_back(operationchoiceautoset);
    m_operationchoice.push_back(operationchoicemanualset);
    m_operationchoice.push_back(operationchoicenotset);

    NetwrokFrameItem ipconfigitemsipset;
    ipconfigitemsipset.m_NcursesLabel = new NcursesLabel(this, 1, 15, begy(), begx());
    ipconfigitemsipset.m_NcursesLabel->setFocusEnabled(false);
    ipconfigitemsipset.m_NCursesLineEdit = new NCursesLineEdit(this, 1, 3, begy(), begx());
    ipconfigitemsipset.m_NCursesLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
    ipconfigitemsipset.m_ErrorinfoLabel = new NcursesLabel(this, 1, 3, begy(), begx());
    ipconfigitemsipset.m_ErrorinfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    ipconfigitemsipset.m_ErrorinfoLabel->setFocusEnabled(false);
    NetwrokFrameItem ipconfigitemsmaskset;
    ipconfigitemsmaskset.m_NcursesLabel = new NcursesLabel(this, 1, 15, begy(), begx());
    ipconfigitemsmaskset.m_NcursesLabel->setFocusEnabled(false);
    ipconfigitemsmaskset.m_NCursesLineEdit = new NCursesLineEdit(this, 1, 3, begy(), begx());
    ipconfigitemsmaskset.m_NCursesLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
    ipconfigitemsmaskset.m_ErrorinfoLabel = new NcursesLabel(this, 1, 3, begy(), begx());
    ipconfigitemsmaskset.m_ErrorinfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    ipconfigitemsmaskset.m_ErrorinfoLabel->setFocusEnabled(false);
    NetwrokFrameItem ipconfigitemsgatewayset;
    ipconfigitemsgatewayset.m_NcursesLabel = new NcursesLabel(this, 1, 15, begy(), begx());
    ipconfigitemsgatewayset.m_NcursesLabel->setFocusEnabled(false);
    ipconfigitemsgatewayset.m_NCursesLineEdit = new NCursesLineEdit(this, 1, 3, begy(), begx());
    ipconfigitemsgatewayset.m_NCursesLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
    ipconfigitemsgatewayset.m_ErrorinfoLabel = new NcursesLabel(this, 1, 3, begy(), begx());
    ipconfigitemsgatewayset.m_ErrorinfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    ipconfigitemsgatewayset.m_ErrorinfoLabel->setFocusEnabled(false);
    NetwrokFrameItem ipconfigitemsprimarydnsset;
    ipconfigitemsprimarydnsset.m_NcursesLabel = new NcursesLabel(this, 1, 15, begy(), begx());
    ipconfigitemsprimarydnsset.m_NcursesLabel->setFocusEnabled(false);
    ipconfigitemsprimarydnsset.m_NCursesLineEdit = new NCursesLineEdit(this, 1, 3, begy(), begx());
    ipconfigitemsprimarydnsset.m_NCursesLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
    ipconfigitemsprimarydnsset.m_ErrorinfoLabel = new NcursesLabel(this, 1, 3, begy(), begx());
    ipconfigitemsprimarydnsset.m_ErrorinfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    ipconfigitemsprimarydnsset.m_ErrorinfoLabel->setFocusEnabled(false);

    m_ipconfigitems.push_back(ipconfigitemsipset);
    m_ipconfigitems.push_back(ipconfigitemsmaskset);
    m_ipconfigitems.push_back(ipconfigitemsgatewayset);
    m_ipconfigitems.push_back(ipconfigitemsprimarydnsset);


    QString strBack = ::QObject::tr("Back");
    QString strNext = ::QObject::tr("Next");

    m_pBackButton = new NcursesButton(this, strBack, 3, 14, begy() + height() - 5, begx() + 5);
    m_pBackButton->drawShadow(true);
    m_pBackButton->box();
    m_pBackButton->setObjectName(strBack);

    m_pNextButton = new NcursesButton(this, strNext, 3, 14, begy() + height() - 5, begx() + width() - 20);
    m_pNextButton->drawShadow(true);
    m_pNextButton->box();
    m_pNextButton->setObjectName(strNext);

    m_networkconnecterrorlabel = new NcursesLabel(this, m_networkconnecterrorstr,
                                                  1,
                                                  m_networkconnecterrorstr.length() + 2,
                                                  begy() + height() - 7,
                                                  begx() + (width() - m_networkconnecterrorstr.length()) / 2);
    m_networkconnecterrorlabel->setBackground(NcursesUtil::getInstance()->error_attr());
    m_networkconnecterrorlabel->setFocusEnabled(false);
}

void NetwrokFramePrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    printTitle(::QObject::tr("Configure Network"), width());

    m_titledes.clear();
//    m_titledes.append("  " + ::QObject::tr("Do network set by auto, use dhcp to set network"));
    //m_titledes.append("  " + ::QObject::tr("Do network set by manual, in this page you can set IP. Mask. Gateway. DNS"));
    //m_titledes.append("  " + ::QObject::tr("Do not set network now, if you not want set the network now, you can do it with the installation complete"));
    m_titledes.append("  " + ::QObject::tr("Configure Ethernet according to your needs, but you can skip it as well."));
    m_titledes.append("  " + ::QObject::tr("Set the IP address, gateway, netmask, DNS please."));
    if(installer::ReadLocale() == "zh_CN") {
        m_titledesbrower->setText(m_titledes[0], true);
    } else {
        m_titledesbrower->setText(m_titledes[0]);
    }

//    m_networkconfigtypestr = ::QObject::tr("Configuration type") + ":";
    m_networkconfigtypelabel->erase();
    m_networkconfigtypelabel->setText(m_networkconfigtypestr);
    m_networkconfigtypelabel->show();
    if(m_currentchoicetype == 0) {
        if(m_operationchoice.at(0).m_NcursesLabel->isOnFoucs()) {
            m_childpagecounttext->setText(" [1/1] ");
        } else {
            m_childpagecounttext->setText("");
        }
    } else if(m_currentchoicetype == 1) {
        m_childpagecounttext->setText(" [1/1] ");
    }

    for(int i = 0; i < m_operationchoice.size(); i++) {
        m_operationchoice.at(i).m_NcursesLabel->erase();
    }

    m_operationchoice.at(0).m_NcursesLabel->setText(::QObject::tr("Configure Now"));
    m_operationchoice.at(1).m_NcursesLabel->setText(::QObject::tr("Skip"));

    for(int i = 0; i < m_ipconfigitems.size(); i++) {
        m_ipconfigitems.at(i).m_NcursesLabel->erase();
    }

    m_ipconfigitems.at(0).m_NcursesLabel->setText(::QObject::tr("IP:"));
    m_ipconfigitems.at(1).m_NcursesLabel->setText(::QObject::tr("Netmask:"));
    m_ipconfigitems.at(2).m_NcursesLabel->setText(::QObject::tr("Gateway:"));
    m_ipconfigitems.at(3).m_NcursesLabel->setText(::QObject::tr("Primary DNS:"));

    m_networkconnecterrorstr = ::QObject::tr("Network connection error, check the configuration please");
    m_networkconnecterrorlabel->setText(m_networkconnecterrorstr);

    FrameInterfacePrivate::updateTs();

    layout();
}

void NetwrokFramePrivate::show()
{
    if(!m_isshow){
        NCursesWindowBase::show();
        m_isshow = true;
        m_pNextButton->setFocus(true);
        m_networkconnecterrorlabel->hide();
    }
    m_titledesbrower->show();
    updateChoiceType(m_currentchoicetype);
}

void NetwrokFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

bool NetwrokFramePrivate::validate()
{
    m_currentchoicetype = 0;
    m_currentlineeditindex = 0;
    return true;
}

void NetwrokFramePrivate::initConnection()
{
    for(int i = 0; i < m_ipconfigitems.size(); i++) {
        connect(m_ipconfigitems.at(i).m_NCursesLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slot_EidtTextChange(const QString&)));
    }
}

void NetwrokFramePrivate::initIPConfig()
{
    NetworkManager::Device::List list = NetworkManager::networkInterfaces();

    foreach(NetworkManager::Device::Ptr device, list) {
        if(device->type() == NetworkManager::Device::Type::Ethernet) {
            m_ipv4Device = device;
        }

        qDebug() << device->uni();
        qDebug() << "managed: " << device->managed();
        qDebug() << "type: " << device->type();
        qDebug() << "interface name: " << device->interfaceName();
    }

    qDebug() << "***********************************************************************";

    QList<QNetworkInterface> testlist = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface interface, testlist) {
        //接口名称
        qDebug() << "Name:" << interface.name();
        //硬件地址
        qDebug() << "hardwareAddress:" << interface.hardwareAddress();
        qDebug() << "Type:" << interface.type();
        qDebug() << "humanReadableName:" << interface.humanReadableName();
        //获取IP地址条目列表，每个条目包含一个IP地址，一个子网掩码和广播地址
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        //遍历每一个IP地址条目
        foreach (QNetworkAddressEntry entry, entryList) {
            //IP地址
            qDebug() << "---IP address:" << entry.ip().toString();
            qDebug() << "---netmask:" << entry.netmask().toString();
            qDebug() << "---Broadcast:" << entry.broadcast().toString();
        }
    }
}

bool NetwrokFramePrivate::writeInfoList()
{
    if (m_currentchoicetype == 0) {
        return true;
    } else if (m_currentchoicetype == 1) {
        bool isallinputok = true;
        for(int i = 0; i < m_ipconfigitems.size(); i++) {
            if(m_ipconfigitems.at(i).m_IsOK == false) {
                isallinputok = false;
                m_networkconnecterrorlabel->show();
                break;
            }
        }

        if(isallinputok) {
            NetworkSettingInfo networkSettingInfo;
            networkSettingInfo.ip         = m_ipconfigitems.at(0).m_NCursesLineEdit->text();
            networkSettingInfo.mask       = m_ipconfigitems.at(1).m_NCursesLineEdit->text();
            networkSettingInfo.gateway    = m_ipconfigitems.at(2).m_NCursesLineEdit->text();
            networkSettingInfo.primaryDNS = m_ipconfigitems.at(3).m_NCursesLineEdit->text();
            networkSettingInfo.setIpMode  = m_dhcpType;

            NetworkOperate testNetworkOperate(m_ipv4Device);
            testNetworkOperate.setIpV4(networkSettingInfo);


            const auto interfaces = QNetworkInterface::allInterfaces();
            QNetworkInterface interface;
            for (const QNetworkInterface i : interfaces) {
                // FIXME: name == lo
                if (i.name() != "lo") {
                    interface = i;
                    break;
                }
            }

            int setipv4result = QProcess::execute("nmcli", QStringList() << "con"
                                          << "add"
                                          << "type"
                                          << "ethernet"
                                          << "con-name"
                                          << QString("\"%1-lab\"").arg(interface.name())
                                          << "ifname"
                                          << interface.name()
                                          << "ip4"
                                          << QString("%1/%2").arg(networkSettingInfo.ip).arg(coverMask(networkSettingInfo.mask))
                                          << "gw4"
                                          << networkSettingInfo.gateway
                                          );

            int setipv4dnsresult = QProcess::execute("nmcli", QStringList() << "con"
                                          << "mod"
                                          << QString("\"%1-lab\"").arg(interface.name())
                                          << "ipv4.dns"
                                          << QString("%1 %2").arg(networkSettingInfo.primaryDNS, ""));

            int setifnameresult = QProcess::execute("nmcli", QStringList() << "con"
                                          << "up"
                                          << QString("\"%1-lab\"").arg(interface.name())
                                          << "ifname"
                                          << interface.name());

            if ((setipv4result == 0) && (setipv4dnsresult == 0) && (setifnameresult == 0)) {
                return true;
            } else {
                m_networkconnecterrorlabel->show();
                return false;
            }
        } else {
            return false;
        }

    }

    /*if(m_dhcpType == DHCPTYpe::Manual) {
    } else if(m_dhcpType == DHCPTYpe::Auto) {
    }*/
}

void NetwrokFramePrivate::updateChoiceType(int type)
{
    if(type == 0) {
        for(int i = 0; i < m_ipconfigitems.size(); i++) {
            m_ipconfigitems.at(i).m_NcursesLabel->hide();
            m_ipconfigitems.at(i).m_NCursesLineEdit->hide();
            m_ipconfigitems.at(i).m_ErrorinfoLabel->hide();
        }

        for(int i = 0; i < m_operationchoice.size(); i++) {
            m_operationchoice.at(i).m_NcursesLabel->mvwin(m_operationchoice.at(i).m_begy, m_operationchoice.at(i).m_begx);
            m_operationchoice.at(i).m_NcursesLabel->show();
            m_operationchoice.at(i).m_NcursesLabel->refresh();
        }

    } else if(type == 1){
        for(int i = 0; i < m_operationchoice.size(); i++) {
            m_operationchoice.at(i).m_NcursesLabel->hide();
        }

        int testcurrx = m_ipconfigitems.at(3).m_NcursesLabel->width();
        for(int i = 0; i < m_ipconfigitems.size(); i++) {
            m_ipconfigitems.at(i).m_NcursesLabel->mvwin(m_ipconfigitems.at(i).m_begy, m_ipconfigitems.at(i).m_begx);
            m_ipconfigitems.at(i).m_NcursesLabel->show();
            m_ipconfigitems.at(i).m_NcursesLabel->refresh();
            m_ipconfigitems.at(i).m_NCursesLineEdit->mvwin(m_ipconfigitems.at(i).m_begy, m_ipconfigitems.at(i).m_begx + testcurrx + 1);
            m_ipconfigitems.at(i).m_NCursesLineEdit->show();
            m_ipconfigitems.at(i).m_NCursesLineEdit->refresh();
            m_ipconfigitems.at(i).m_ErrorinfoLabel->mvwin(m_ipconfigitems.at(i).m_begy + 1, m_ipconfigitems.at(i).m_begx + testcurrx + 1);
            m_ipconfigitems.at(i).m_ErrorinfoLabel->show();
            m_ipconfigitems.at(i).m_ErrorinfoLabel->refresh();
        }
    }
}

void NetwrokFramePrivate::setFocusEnableType(int type)
{
    if(type == 0) {
        for(int i = 0; i < m_ipconfigitems.size(); i++) {
            m_ipconfigitems.at(i).m_NcursesLabel->setFocusEnabled(false);
            m_ipconfigitems.at(i).m_NCursesLineEdit->setFocusEnabled(false);
            m_ipconfigitems.at(i).m_ErrorinfoLabel->setFocusEnabled(false);
        }

        for(int i = 0; i < m_operationchoice.size(); i++) {
            m_operationchoice.at(i).m_NcursesLabel->setFocusEnabled(false);
        }

        bool ishasfocus = false;
        for(int i = 0; i < m_operationchoice.size(); i++) {
            if(m_operationchoice.at(i).m_NcursesLabel->isOnFoucs()) {
                ishasfocus = true;
                break;
            }
        }

        if(!ishasfocus) {
            m_operationchoice.at(0).m_NcursesLabel->setFocus(true);
        }

        m_pBackButton->setFocus(false);
        m_pNextButton->setFocus(true);

    } else if(type == 1){
        for(int i = 0; i < m_operationchoice.size(); i++) {
            m_operationchoice.at(i).m_NcursesLabel->setFocusEnabled(false);
        }

        for(int i = 0; i < m_ipconfigitems.size(); i++) {
            m_ipconfigitems.at(i).m_NcursesLabel->setFocusEnabled(false);
            m_ipconfigitems.at(i).m_NCursesLineEdit->setFocusEnabled(true);
            m_ipconfigitems.at(i).m_ErrorinfoLabel->setFocusEnabled(false);
        }

        bool ishasfocus = false;
        for(int i = 0; i < m_ipconfigitems.size(); i++) {
            if(m_ipconfigitems.at(i).m_NCursesLineEdit->isOnFoucs()) {
                ishasfocus = true;
                m_currentlineeditindex = i;
                break;
            }
        }

        if(!ishasfocus) {
            m_ipconfigitems.at(0).m_NCursesLineEdit->setFocus(true);
            m_currentlineeditindex = 0;
        }

        m_pBackButton->setFocus(false);
        m_pNextButton->setFocus(false);
    }

    m_currentchoicetype = type;
}

void NetwrokFramePrivate::onKeyPress(int keyCode)
{
    switch (keyCode) {
    case KEY_UP:
        if(m_currentchoicetype == 0) {
            for(int i = 0; i < m_operationchoice.size(); i++) {
                if(m_operationchoice.at(i).m_NcursesLabel->isOnFoucs()){
                    if(i != 0) {
                        m_operationchoice.at(i).m_NcursesLabel->setFocus(false);
                        m_operationchoice.at(i - 1).m_NcursesLabel->setFocus(true);
                        m_titledesbrower->setText(m_titledes[i - 1]);
                        m_titledesbrower->show();
                        m_titledesbrower->refresh();

                        if((i - 1) == 0) {
                            m_childpagecounttext->setText(" [1/1] ");
                            m_childpagecounttext->show();
                        } else {
                            m_childpagecounttext->setText("");
                            m_childpagecounttext->show();
                        }
                        break;
                    }
                }
            }

            m_pBackButton->setFocus(false);
            m_pNextButton->setFocus(true);
        }

        break;

    case KEY_DOWN:
        if(m_currentchoicetype == 0) {
            for(int i = 0; i < m_operationchoice.size(); i++) {
                if(m_operationchoice.at(i).m_NcursesLabel->isOnFoucs()){
                    if((i + 1) != m_operationchoice.size()) {
                        m_operationchoice.at(i).m_NcursesLabel->setFocus(false);
                        m_operationchoice.at(i + 1).m_NcursesLabel->setFocus(true);
                        m_titledesbrower->setText(m_titledes[i + 1]);
                        m_titledesbrower->show();
                        m_titledesbrower->refresh();

                        if((i + 1) == 0) {
                            m_childpagecounttext->setText(" [1/1] ");
                            m_childpagecounttext->show();
                        } else {
                            m_childpagecounttext->setText("");
                            m_childpagecounttext->show();
                        }
                        break;
                    }
                }
            }
            m_pBackButton->setFocus(false);
            m_pNextButton->setFocus(true);
        }

        break;

    case KEY_ENTER:
    case KEY_ENTER_OTHER:
        if(m_pBackButton->isOnFoucs()) {
            doBackBtnClicked();
        } else if (m_pNextButton->isOnFoucs()) {
            doNextBtnClicked();
        }
        break;
    }

    NCursesWindowBase::onKeyPress(keyCode);
}

void NetwrokFramePrivate::resetValue()
{
    m_currentchoicetype = 0;
}

void NetwrokFramePrivate::manualConfigure()
{

}

void NetwrokFramePrivate::AutoConfigure()
{

}

void NetwrokFramePrivate::doBackBtnClicked()
{
    if(m_currentchoicetype == 1) {
        m_childpagecounttext->setText(" [1/1] ");
        m_childpagecounttext->show();
        updateChoiceType(0);
        setFocusEnableType(0);
    } else {
      emit back();
    }
}

void NetwrokFramePrivate::doNextBtnClicked()
{
    if(m_currentchoicetype == 1) {
        if(writeInfoList()){
            emit next();
        }
    } else if(m_currentchoicetype == 0) {
/*        if(m_operationchoice.at(0).m_NcursesLabel->isOnFoucs()) {
            m_dhcpType = DHCPTYpe::Auto;
            if(writeInfoList()) {
                emit next();
            }
        } else */if(m_operationchoice.at(0).m_NcursesLabel->isOnFoucs()) {
            m_dhcpType = DHCPTYpe::Manual;
            m_childpagecounttext->setText(" [2/2] ");
            m_childpagecounttext->show();
            updateChoiceType(1);
            setFocusEnableType(1);
        } else if(m_operationchoice.at(1).m_NcursesLabel->isOnFoucs()) {
            emit next();
        }
    }
}

void NetwrokFramePrivate::editTextCheck(int index, const QString &text)
{
    QRegularExpression checktest = QRegularExpression("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)?\\.){0,3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)?");
    QRegularExpression testrx_ip(
                "^(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.("
                "\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])$");
    QRegularExpression testrx_gateway(
                "^(254|252|248|240|224|192|128|0)\\.0\\.0\\.0|255\\.(254|252|248|240|224|192|128|"
                "0)\\.0\\.0|255\\.255\\.(254|252|248|240|224|192|128|0)\\.0|255\\.255\\.255\\.("
                "254|252|248|240|224|192|128|0)$");

    if(!text.compare("")) {
        if(index == 1) {
            if(!m_ipconfigitems.at(index).m_ErrorinfoLabel->text().compare(::QObject::tr("e.g., 255.255.255.0"))) {
                m_ipconfigitems[index].m_IsErrorTextChange = false;
            } else {
                m_ipconfigitems[index].m_IsErrorTextChange = true;
            }
            m_ipconfigitems.at(index).m_ErrorinfoLabel->setText(::QObject::tr("e.g., 255.255.255.0"));
            m_ipconfigitems[index].m_IsOK = false;
        } else {
            if(!m_ipconfigitems.at(index).m_ErrorinfoLabel->text().compare(::QObject::tr("e.g., 192.168.30.30"))) {
                m_ipconfigitems[index].m_IsErrorTextChange = false;
            } else {
                m_ipconfigitems[index].m_IsErrorTextChange = true;
            }
            m_ipconfigitems.at(index).m_ErrorinfoLabel->setText(::QObject::tr("e.g., 192.168.30.30"));
            m_ipconfigitems[index].m_IsOK = false;
        }
    } else {
        if(checktest.match(text).hasMatch()) {
            if(index == 1) {
                if(testrx_gateway.match(text).hasMatch()) {
                    if(!m_ipconfigitems.at(index).m_ErrorinfoLabel->text().compare("")) {
                        m_ipconfigitems[index].m_IsErrorTextChange = false;
                    } else {
                        m_ipconfigitems[index].m_IsErrorTextChange = true;
                    }
                    m_ipconfigitems.at(index).m_ErrorinfoLabel->setText(::QObject::tr(""));
                    m_ipconfigitems[index].m_IsOK = true;
                } else {
                    if(!m_ipconfigitems.at(index).m_ErrorinfoLabel->text().compare(::QObject::tr("e.g., 255.255.255.0"))) {
                        m_ipconfigitems[index].m_IsErrorTextChange = false;
                    } else {
                        m_ipconfigitems[index].m_IsErrorTextChange = true;
                    }
                    m_ipconfigitems.at(index).m_ErrorinfoLabel->setText(::QObject::tr("e.g., 255.255.255.0"));
                    m_ipconfigitems[index].m_IsOK = false;
                }
            } else {
                if(testrx_ip.match(text).hasMatch()) {
                    if(!m_ipconfigitems.at(index).m_ErrorinfoLabel->text().compare("")) {
                        m_ipconfigitems[index].m_IsErrorTextChange = false;
                    } else {
                        m_ipconfigitems[index].m_IsErrorTextChange = true;
                    }
                    m_ipconfigitems.at(index).m_ErrorinfoLabel->setText("");
                    m_ipconfigitems[index].m_IsOK = true;
                } else {
                    if(!m_ipconfigitems.at(index).m_ErrorinfoLabel->text().compare(::QObject::tr("e.g., 192.168.30.30"))) {
                        m_ipconfigitems[index].m_IsErrorTextChange = false;
                    } else {
                        m_ipconfigitems[index].m_IsErrorTextChange = true;
                    }
                    m_ipconfigitems.at(index).m_ErrorinfoLabel->setText(::QObject::tr("e.g., 192.168.30.30"));
                    m_ipconfigitems[index].m_IsOK = false;
                }
            }
        } else {
            if(!m_ipconfigitems.at(index).m_ErrorinfoLabel->text().compare(::QObject::tr("e.g., 192.168.30.30"))) {
                m_ipconfigitems[index].m_IsErrorTextChange = false;
            } else {
                m_ipconfigitems[index].m_IsErrorTextChange = true;
            }
            m_ipconfigitems.at(index).m_ErrorinfoLabel->setText(::QObject::tr("e.g., 192.168.30.30"));
            m_ipconfigitems[index].m_IsOK = false;
        }
    }

    if(m_ipconfigitems.at(index).m_IsErrorTextChange){
        m_ipconfigitems.at(index).m_ErrorinfoLabel->show();
        m_ipconfigitems.at(index).m_ErrorinfoLabel->refresh();
    }
}

void NetwrokFramePrivate::slot_EidtTextChange(const QString &text)
{
    if(m_currentchoicetype == 1) {

        for(int i = 0; i < m_ipconfigitems.size(); i++) {
            if(m_ipconfigitems.at(i).m_NCursesLineEdit->isOnFoucs()) {
                if(m_currentlineeditindex != i) {
                    editTextCheck(m_currentlineeditindex, m_ipconfigitems.at(m_currentlineeditindex).m_NCursesLineEdit->text());
                    m_currentlineeditindex = i;
                    editTextCheck(m_currentlineeditindex, m_ipconfigitems.at(m_currentlineeditindex).m_NCursesLineEdit->text());
                } else {
                    editTextCheck(m_currentlineeditindex, m_ipconfigitems.at(m_currentlineeditindex).m_NCursesLineEdit->text());
                }
                break;
            }
        }
    }
}

/*void NetwrokFramePrivate::slot_onDeviceListChanged(const QList<dde::network::NetworkDevice *> devices)
{
    // add wireless device list
    bool testhasAp = false;
    bool testhasWired = false;
    bool testhasWireless = false;
    for (auto const dev : devices) {
        if (dev->type() == dde::network::NetworkDevice::Wired) {
            testhasWired = true;
        }
        if (dev->type() != dde::network::NetworkDevice::Wireless)
            continue;
        testhasWireless = true;
        if (qobject_cast<dde::network::WirelessDevice *>(dev)->supportHotspot()) {
            testhasAp = true;
        }
    }
    qDebug() << "[Network] device state : " << testhasWired << "," << testhasWireless << "," << testhasAp;
}*/


void NetwrokFramePrivate::layout()
{
    //updateTs();
    int testcurry = begy() + m_titledesbrower->height() + 6;
    int testcurrx = width() / 2 - m_operationchoice.at(0).m_NcursesLabel->text().length() / 2;

    for(int i = 0; i < m_operationchoice.size(); i++) {
        m_operationchoice[i].m_begy = testcurry + i;
        m_operationchoice[i].m_begx = begx() + testcurrx;
        m_operationchoice.at(i).m_NcursesLabel->mvwin(m_operationchoice.at(i).m_begy, m_operationchoice.at(i).m_begx);
    }

    testcurrx = m_ipconfigitems.at(3).m_NcursesLabel->width();
    int edittorisize = width() / 2 - testcurrx - 2;
    for(int i = 0; i < m_ipconfigitems.size(); i++) {
        m_ipconfigitems[i].m_begy = testcurry + i;
        m_ipconfigitems[i].m_begx = begx() + width() / 2 - (testcurrx + edittorisize) / 2;
        m_ipconfigitems.at(i).m_NcursesLabel->mvwin(m_ipconfigitems.at(i).m_begy, m_ipconfigitems.at(i).m_begx);
        m_ipconfigitems.at(i).m_NCursesLineEdit->wresize(1, edittorisize);
        m_ipconfigitems.at(i).m_NCursesLineEdit->mvwin(m_ipconfigitems.at(i).m_begy, m_ipconfigitems.at(i).m_begx + testcurrx + 1);
        m_ipconfigitems.at(i).m_ErrorinfoLabel->wresize(1, width() - testcurrx - 2 - width() / 2 + (testcurrx + edittorisize) / 2);
        m_ipconfigitems.at(i).m_ErrorinfoLabel->mvwin(m_ipconfigitems.at(i).m_begy + 1, m_ipconfigitems.at(i).m_begx + testcurrx + 1);
        testcurry++;
    }
}


NetwrokFrame::NetwrokFrame(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new NetwrokFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
}

NetwrokFrame::~NetwrokFrame()
{

}


bool NetwrokFrame::init()
{
    Q_D(NetwrokFrame);
    if (m_currState == FRAME_STATE_NOT_START) {
        m_private->layout();
        m_currState = FRAME_STATE_RUNNING;
    }
    d->resetValue();
    d->initIPConfig();
    d->updateChoiceType(0);
    d->setFocusEnableType(0);
    return true;
}

QString NetwrokFrame::getFrameName()
{
    return "NetwrokFrame";
}

bool NetwrokFrame::handle()
{
    return true;
}

}
