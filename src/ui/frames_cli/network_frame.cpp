#include "network_frame.h"

#include "ui/ncurses_widgets/ncurses_text_brower.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_line_edit.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_button.h"
#include "base/utils.h"

#include <QProcess>
#include <QThread>
#include <QNetworkInterface>

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

    m_titledes.append("    " + ::QObject::tr("Configure Ethernet according to your needs, but you can skip it as well."));
    m_titledes.append("    " + ::QObject::tr("Set the IP address, gateway, netmask, DNS please."));

    m_networkconnecterrorstr = ::QObject::tr("Network connection error, check the configuration please");

    m_titledesbrower = new NcursesTextBrower(this, 3, width() - 2, begy() + 2, begx() + 1);
    m_titledesbrower->setFocusEnabled(false);
    m_titledesbrower->setText(m_titledes[m_currentchoicetype]);
    //m_titledesbrower->hide();

    m_childpagecounttext = new NcursesLabel(this, " [1/2] ", 1, 8, begy() + 6, begx() + 1);
    m_childpagecounttext->setFocusEnabled(false);
    //m_childpagecounttext->hide();

    m_operationchoice = new NcursesListView(this, 2, width() / 2, begy(), begx());
    ////m_operationchoice->setFocus(true);
    //m_operationchoice->hide();

    NetwrokFrameItem ipconfigitemsipset;
    ipconfigitemsipset.m_NcursesLabel = new NcursesLabel(this, 1, 15, begy(), begx());
    ipconfigitemsipset.m_NcursesLabel->setFocusEnabled(false);
    //ipconfigitemsipset.m_NcursesLabel->hide();
    ipconfigitemsipset.m_NCursesLineEdit = new NCursesLineEdit(this, 1, 3, begy(), begx());
    ipconfigitemsipset.m_NCursesLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
    ipconfigitemsipset.m_NCursesLineEdit->setModle(NCursesLineEdit::IPEDIT);
    //ipconfigitemsipset.m_NCursesLineEdit->hide();
    ipconfigitemsipset.m_ErrorinfoLabel = new NcursesLabel(this, 1, 3, begy(), begx());
    ipconfigitemsipset.m_ErrorinfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    ipconfigitemsipset.m_ErrorinfoLabel->setFocusEnabled(false);
    //ipconfigitemsipset.m_ErrorinfoLabel->hide();
    NetwrokFrameItem ipconfigitemsmaskset;
    ipconfigitemsmaskset.m_NcursesLabel = new NcursesLabel(this, 1, 15, begy(), begx());
    ipconfigitemsmaskset.m_NcursesLabel->setFocusEnabled(false);
    //ipconfigitemsmaskset.m_NcursesLabel->hide();
    ipconfigitemsmaskset.m_NCursesLineEdit = new NCursesLineEdit(this, 1, 3, begy(), begx());
    ipconfigitemsmaskset.m_NCursesLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
    ipconfigitemsmaskset.m_NCursesLineEdit->setModle(NCursesLineEdit::IPEDIT);
    //ipconfigitemsmaskset.m_NCursesLineEdit->hide();
    ipconfigitemsmaskset.m_ErrorinfoLabel = new NcursesLabel(this, 1, 3, begy(), begx());
    ipconfigitemsmaskset.m_ErrorinfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    ipconfigitemsmaskset.m_ErrorinfoLabel->setFocusEnabled(false);
    //ipconfigitemsmaskset.m_ErrorinfoLabel->hide();
    NetwrokFrameItem ipconfigitemsgatewayset;
    ipconfigitemsgatewayset.m_NcursesLabel = new NcursesLabel(this, 1, 15, begy(), begx());
    ipconfigitemsgatewayset.m_NcursesLabel->setFocusEnabled(false);
    //ipconfigitemsgatewayset.m_NcursesLabel->hide();
    ipconfigitemsgatewayset.m_NCursesLineEdit = new NCursesLineEdit(this, 1, 3, begy(), begx());
    ipconfigitemsgatewayset.m_NCursesLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
    ipconfigitemsgatewayset.m_NCursesLineEdit->setModle(NCursesLineEdit::IPEDIT);
    //ipconfigitemsgatewayset.m_NCursesLineEdit->hide();
    ipconfigitemsgatewayset.m_ErrorinfoLabel = new NcursesLabel(this, 1, 3, begy(), begx());
    ipconfigitemsgatewayset.m_ErrorinfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    ipconfigitemsgatewayset.m_ErrorinfoLabel->setFocusEnabled(false);
    //ipconfigitemsgatewayset.m_ErrorinfoLabel->hide();
    NetwrokFrameItem ipconfigitemsprimarydnsset;
    ipconfigitemsprimarydnsset.m_NcursesLabel = new NcursesLabel(this, 1, 15, begy(), begx());
    ipconfigitemsprimarydnsset.m_NcursesLabel->setFocusEnabled(false);
    //ipconfigitemsprimarydnsset.m_NcursesLabel->hide();
    ipconfigitemsprimarydnsset.m_NCursesLineEdit = new NCursesLineEdit(this, 1, 3, begy(), begx());
    ipconfigitemsprimarydnsset.m_NCursesLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
    ipconfigitemsprimarydnsset.m_NCursesLineEdit->setModle(NCursesLineEdit::IPEDIT);
    //ipconfigitemsprimarydnsset.m_NCursesLineEdit->hide();
    ipconfigitemsprimarydnsset.m_ErrorinfoLabel = new NcursesLabel(this, 1, 3, begy(), begx());
    ipconfigitemsprimarydnsset.m_ErrorinfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    ipconfigitemsprimarydnsset.m_ErrorinfoLabel->setFocusEnabled(false);
    //ipconfigitemsprimarydnsset.m_ErrorinfoLabel->hide();
    NetwrokFrameItem ipconfigitemssecondarydnsset;
    ipconfigitemssecondarydnsset.m_NcursesLabel = new NcursesLabel(this, 1, 15, begy(), begx());
    ipconfigitemssecondarydnsset.m_NcursesLabel->setFocusEnabled(false);
    //ipconfigitemssecondarydnsset.m_NcursesLabel->hide();
    ipconfigitemssecondarydnsset.m_NCursesLineEdit = new NCursesLineEdit(this, 1, 3, begy(), begx());
    ipconfigitemssecondarydnsset.m_NCursesLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
    ipconfigitemssecondarydnsset.m_NCursesLineEdit->setModle(NCursesLineEdit::IPEDIT);
    //ipconfigitemssecondarydnsset.m_NCursesLineEdit->hide();
    ipconfigitemssecondarydnsset.m_ErrorinfoLabel = new NcursesLabel(this, 1, 3, begy(), begx());
    ipconfigitemssecondarydnsset.m_ErrorinfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    ipconfigitemssecondarydnsset.m_ErrorinfoLabel->setFocusEnabled(false);
    //ipconfigitemssecondarydnsset.m_ErrorinfoLabel->hide();

    m_ipconfigitems.push_back(ipconfigitemsipset);
    m_ipconfigitems.push_back(ipconfigitemsmaskset);
    m_ipconfigitems.push_back(ipconfigitemsgatewayset);
    m_ipconfigitems.push_back(ipconfigitemsprimarydnsset);
    m_ipconfigitems.push_back(ipconfigitemssecondarydnsset);


    QString strBack = ::QObject::tr("Back");
    QString strNext = ::QObject::tr("Next");

    m_pNextButton = new NcursesButton(this, strNext, 3, 14, begy() + height() - 5, begx() + width() - 20);
    m_pNextButton->drawShadow(true);
    m_pNextButton->box();
    m_pNextButton->setObjectName(strNext);
    //m_pNextButton->hide();

    m_pBackButton = new NcursesButton(this, strBack, 3, 14, begy() + height() - 5, begx() + 5);
    m_pBackButton->drawShadow(true);
    m_pBackButton->box();
    m_pBackButton->setObjectName(strBack);
    //m_pBackButton->hide();

    m_networkconnecterrorlabel = new NcursesLabel(this, m_networkconnecterrorstr,
                                                  1,
                                                  m_networkconnecterrorstr.length() + 2,
                                                  begy() + height() - 7,
                                                  begx() + (width() - m_networkconnecterrorstr.length()) / 2);
    m_networkconnecterrorlabel->setBackground(NcursesUtil::getInstance()->error_attr());
    m_networkconnecterrorlabel->setFocusEnabled(false);
    //m_networkconnecterrorlabel->hide();
}

void NetwrokFramePrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    printTitle(::QObject::tr("Configure Network"), width());

    m_titledes.clear();
    m_titledes.append("  " + ::QObject::tr("Configure Ethernet according to your needs, but you can skip it as well."));
    m_titledes.append("  " + ::QObject::tr("Set the IP address, gateway, netmask, DNS please."));
    if(installer::ReadLocale() == "zh_CN") {
        m_titledesbrower->setText(m_titledes[m_currentchoicetype], true);
    } else {
        m_titledesbrower->setText(m_titledes[m_currentchoicetype]);
    }

    //m_networkconfigtypelabel->erase();
    //m_networkconfigtypelabel->setText(m_networkconfigtypestr);
    //m_networkconfigtypelabel->show();

    if(m_currentchoicetype == 0) {
        if(m_operationchoice->getCurrentIndex() == 0) {
            m_childpagecounttext->setText(" [1/2] ");
        } else {
            m_childpagecounttext->setText("");
        }
    } else if(m_currentchoicetype == 1) {
        m_childpagecounttext->setText(" [1/2] ");
    }

    QStringList strList;
    strList << ::QObject::tr("Configure Now");
    strList << ::QObject::tr("Skip");
    m_operationchoice->setList(strList);


    for(int i = 0; i < m_ipconfigitems.size(); i++) {
        m_ipconfigitems.at(i).m_NcursesLabel->erase();
    }

    m_ipconfigitems.at(0).m_NcursesLabel->setText(::QObject::tr("IP:"));
    m_ipconfigitems.at(1).m_NcursesLabel->setText(::QObject::tr("Netmask:"));
    m_ipconfigitems.at(2).m_NcursesLabel->setText(::QObject::tr("Gateway:"));
    m_ipconfigitems.at(3).m_NcursesLabel->setText(::QObject::tr("Primary DNS:"));
    m_ipconfigitems.at(4).m_NcursesLabel->setText(::QObject::tr("Secondary DNS:"));

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
        m_pBackButton->setFocus(false);
        m_pNextButton->setFocus(true);
        m_networkconnecterrorlabel->hide();
        updateChoiceType(m_currentchoicetype);
    }
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
        m_isallinputok = true;
        for(int i = 0; i < m_ipconfigitems.size() - 2; i++) {
            if(m_ipconfigitems.at(i).m_IsOK == false) {
                m_isallinputok = false;
                m_networkconnecterrorlabel->show();
                break;
            }
        }

        if(m_isallinputok) {
            for(int i = m_ipconfigitems.size() - 3; i < m_ipconfigitems.size(); i++) {
                if((m_ipconfigitems.at(i).m_IsOK == false) && (m_ipconfigitems.at(i).m_NCursesLineEdit->text().compare(""))) {
                    m_isallinputok = false;
                    m_networkconnecterrorlabel->show();
                    break;
                }
            }
        }

        if(m_isallinputok) {
            NetworkSettingInfo networkSettingInfo;
            networkSettingInfo.ip         = m_ipconfigitems.at(0).m_NCursesLineEdit->text();
            networkSettingInfo.mask       = m_ipconfigitems.at(1).m_NCursesLineEdit->text();
            networkSettingInfo.gateway    = m_ipconfigitems.at(2).m_NCursesLineEdit->text();
            networkSettingInfo.primaryDNS = m_ipconfigitems.at(3).m_NCursesLineEdit->text();
            networkSettingInfo.secondaryDNS = m_ipconfigitems.at(4).m_NCursesLineEdit->text();
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
                                          << QString("%1 %2").arg(networkSettingInfo.primaryDNS, networkSettingInfo.secondaryDNS));

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

    return false;
}

void NetwrokFramePrivate::updateChoiceType(int type)
{
    if(type == 0) {
        for(int i = 0; i < m_ipconfigitems.size(); i++) {
            m_ipconfigitems.at(i).m_NcursesLabel->hide();
            m_ipconfigitems.at(i).m_NCursesLineEdit->hide();
            m_ipconfigitems.at(i).m_ErrorinfoLabel->hide();
        }
        m_operationchoice->show();
        m_operationchoice->refresh();

    } else if(type == 1){
        m_operationchoice->hide();

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
        m_operationchoice->setFocusEnabled(true);

        m_pNextButton->setFocusEnabled(true);
        m_pBackButton->setFocusEnabled(true);

    } else if(type == 1){
        m_operationchoice->setFocusEnabled(false);

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

        m_pBackButton->setFocusEnabled(false);
        m_pNextButton->setFocusEnabled(m_isallinputok);
        m_pBackButton->setFocusEnabled(true);
    }

    m_currentchoicetype = type;
}

void NetwrokFramePrivate::onKeyPress(int keyCode)
{
    qDebug() << "keyCode = " << keyCode;
    switch (keyCode) {
    case KEY_TAB:
            switchChildWindowsFoucs();
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
        m_childpagecounttext->setText(" [1/2] ");
        m_childpagecounttext->show();
        m_networkconnecterrorlabel->hide();
        updateChoiceType(0);
        setFocusEnableType(0);
        if(installer::ReadLocale() == "zh_CN") {
            m_titledesbrower->setText(m_titledes[m_currentchoicetype], true);
        } else {
            m_titledesbrower->setText(m_titledes[m_currentchoicetype]);
        }
        m_titledesbrower->show();
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
        if(m_operationchoice->getCurrentIndex() == 0) {
            m_dhcpType = DHCPTYpe::Manual;
            m_childpagecounttext->setText(" [2/2] ");
            m_childpagecounttext->show();
            updateChoiceType(1);
            setFocusEnableType(1);
            if(installer::ReadLocale() == "zh_CN") {
                m_titledesbrower->setText(m_titledes[m_currentchoicetype], true);
            } else {
                m_titledesbrower->setText(m_titledes[m_currentchoicetype]);
            }
            m_titledesbrower->show();
            m_pNextButton->setFocus(false);
        } else if(m_operationchoice->getCurrentIndex() == 1) {
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
            if(m_ipconfigitems.at(i).m_NCursesLineEdit->isOnFoucs() && m_ipconfigitems.at(i).m_NCursesLineEdit->text().compare("")) {
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

        m_isallinputok = true;
        for (int i = 0; i < m_ipconfigitems.size() - 2; i++) {
            if (m_ipconfigitems.at(i).m_IsOK == false) {
                m_isallinputok = false;
                break;
            }
        }

        if(m_isallinputok) {
            for(int i = m_ipconfigitems.size() - 3; i < m_ipconfigitems.size(); i++) {
                if((m_ipconfigitems.at(i).m_IsOK == false) && (m_ipconfigitems.at(i).m_NCursesLineEdit->text().compare(""))) {
                    m_isallinputok = false;
                    break;
                }
            }

            setFocusEnableType(1);
        }
    }
}


void NetwrokFramePrivate::layout()
{
    m_operationchoice->adjustSizeByContext();
    int testcurry = begy() + m_titledesbrower->height() + 6;
    int testcurrx = begx() + width() / 2 - m_operationchoice->width() / 2;
    int curry = testcurry;

    m_operationchoice->mvwin(testcurry, testcurrx);

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
        curry = testcurry + i;
    }

    m_networkconnecterrorlabel->adjustSizeByContext();
    testcurrx = begx() + width() / 2 - (m_networkconnecterrorlabel->width()) / 2;
    m_networkconnecterrorlabel->mvwin(curry + 1, testcurrx);
}


NetwrokFrame::NetwrokFrame(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = MAINWINDOW_HEIGHT;//LINES / 2;
    int w = MAINWINDOW_WIDTH;//COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new NetwrokFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
    //m_private->hide();
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
