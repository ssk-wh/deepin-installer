#include "networkframe.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/system_info_tip.h"
#include "ui/widgets/network_device_widget.h"
#include "ui/delegates/network_operate.h"

#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSettings>
#include <QUuid>
#include <QVBoxLayout>
#include <list>
#include <utility>
#include <QEvent>
#include <QNetworkInterface>
#include <QProcess>
#include <DBackgroundGroup>
#include <QComboBox>
#include <QStringListModel>
#include <memory>
#include <QTimer>
#include <QPainter>
#include <QPushButton>
#include <QFont>
#include <DFrame>
#include <DVerticalLine>
#include <DLineEdit>
#include <DSwitchButton>

DWIDGET_USE_NAMESPACE

using namespace installer;

namespace {
    const int kViewWidth = 260;
    const int kViewHeight = 370;

    const int kNextButtonWidth = 340;
    const int kNextButtonHeight = 36;

    const int kEditSaveButtonWidth = 140;
    const int kEditSaveButtonHeight = 36;

    const int kLineEditWidth = 140;
    const int kLineEditHeight = 36;

    const int kFontSize = 10;
}

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

static bool checkip(QString ip)
{
    QRegularExpression rx(
        "^(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.("
        "\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])$");
    return rx.match(ip).hasMatch();
}

static bool checkMask(QString mask)
{
    QRegularExpression rx(
        "^(254|252|248|240|224|192|128|0)\\.0\\.0\\.0|255\\.(254|252|248|240|224|192|128|"
        "0)\\.0\\.0|255\\.255\\.(254|252|248|240|224|192|128|0)\\.0|255\\.255\\.255\\.("
        "254|252|248|240|224|192|128|0)$");

    return rx.match(mask).hasMatch();
}

namespace installer {

class NetworkEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NetworkEditWidget (QWidget* parent = nullptr) : QWidget(parent) {
        m_switch = new QWidget;
        m_switchButton =  new DSwitchButton;
        m_switchButton->setEnabled(false);
        m_deviceEnable = false;
        m_connectTypeWidget = new QWidget;
        m_dhcpTypeWidget = new QComboBox;
        m_dhcpTypeWidget->setEnabled(false);
        m_ipWidget = new QWidget;
        m_maskWidget = new QWidget;
        m_gatewayWidget = new QWidget;
        m_primaryDNSWidget = new QWidget;

        m_device = nullptr;
        m_networkOperate = nullptr;

        m_editBtn = new QPushButton(tr("Edit"));
        m_editBtn->setEnabled(m_deviceEnable);
        m_editBtn->setFixedSize(kEditSaveButtonWidth, kEditSaveButtonHeight);
        m_acceptBtn = new QPushButton(tr("Accept"));
        m_acceptBtn->setFixedSize(kEditSaveButtonWidth, kEditSaveButtonHeight);
        m_dhcpType = DHCPTYpe::Auto;

        QFont font;
        font.setPixelSize(kFontSize);
        m_ipv4Edit = new DLineEdit;
        m_ipv4Edit->setFixedSize(kLineEditWidth, kLineEditHeight);
        m_ipv4Edit->lineEdit()->setFont(font);
        m_maskEdit = new DLineEdit;
        m_maskEdit->setFixedSize(kLineEditWidth, kLineEditHeight);
        m_maskEdit->lineEdit()->setFont(font);
        m_gatewayEdit = new DLineEdit;
        m_gatewayEdit->setFixedSize(kLineEditWidth, kLineEditHeight);
        m_gatewayEdit->lineEdit()->setFont(font);
        m_primaryDNSEdit = new DLineEdit;
        m_primaryDNSEdit->setFixedSize(kLineEditWidth, kLineEditHeight);
        m_primaryDNSEdit->lineEdit()->setFont(font);

        m_ipv4Edit->lineEdit()->setPlaceholderText(tr("IP Address"));
        m_maskEdit->lineEdit()->setPlaceholderText(tr("Netmask"));
        m_gatewayEdit->lineEdit()->setPlaceholderText(tr("Gateway"));
        m_primaryDNSEdit->lineEdit()->setPlaceholderText(tr("Primary DNS"));

        m_errorTip = new SystemInfoTip(this);
        m_errorTip->hide();

        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->setMargin(0);
        mainLayout->setSpacing(20);

        QMap<QWidget*, QString> tmpM {
            {m_ipWidget, tr("Ip:")},
            {m_maskWidget, tr("Mask:")},
            {m_gatewayWidget, tr("Gateway:")},
            {m_primaryDNSWidget, tr("Primary DNS:")},
        };

        m_widgetList = {
            {m_ipWidget, m_ipv4Edit},
            {m_maskWidget, m_maskEdit},
            {m_gatewayWidget, m_gatewayEdit},
            {m_primaryDNSWidget, m_primaryDNSEdit},
        };

        m_editList = {
            m_ipv4Edit,
            m_maskEdit,
            m_gatewayEdit,
            m_primaryDNSEdit,
        };

        for (auto it = m_editList.begin(); it != m_editList.end(); ++it) {
            connect(*it, &DLineEdit::textEdited, this, &NetworkEditWidget::onEditingLineEdit);
            (*it)->lineEdit()->setValidator(m_validityCheck.get());
            (*it)->lineEdit()->setAlignment(Qt::AlignCenter);
        }

        int i = 0;
        for (auto it = tmpM.begin(); it != tmpM.end(); ++it) {
            QHBoxLayout* layout = new QHBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);

            QLabel* name = new QLabel(it.value());
            name->setFixedSize(100, 20);
            QLabel* valueName = new QLabel;

            layout->addWidget(name, 0, Qt::AlignLeft | Qt::AlignHCenter);
            layout->addWidget(valueName, 0, Qt::AlignRight | Qt::AlignHCenter);
            Q_ASSERT(i < m_editList.size());
            layout->addWidget(m_editList[i], 0, Qt::AlignRight | Qt::AlignHCenter);
            m_editList[i]->hide();
            ++i;

            labelHandleMap[it.key()] = [=](const QString& text) -> void {
                valueName->setText(text);
            };

            labelShowMap[it.key()] = [=](const bool show) -> void {
                if (show) {
                    valueName->show();
                }
                else {
                    valueName->hide();
                }
            };

            labelTextMap[it.key()] = [=]() -> QString {
                return valueName->text();
            };

            it.key()->setLayout(layout);
        }

        QHBoxLayout* dhcpLayout = new QHBoxLayout;
        dhcpLayout->setMargin(0);
        dhcpLayout->setSpacing(0);

        QLabel* dhcpName = new QLabel(tr("DHCP"));
        dhcpName->setFixedSize(53, 20);
        dhcpLayout->addWidget(dhcpName, 0, Qt::AlignLeft | Qt::AlignHCenter);
        dhcpLayout->addWidget(m_dhcpTypeWidget, 0, Qt::AlignRight | Qt::AlignHCenter);

        QStringListModel* dhcpTypeModel = new QStringListModel(m_dhcpTypeWidget);
        dhcpTypeModel->setStringList({tr("Auto"), tr("Manual")});
        m_dhcpTypeWidget->setModel(dhcpTypeModel);
        m_dhcpTypeWidget->setFixedSize(kLineEditWidth, kLineEditHeight);

        m_connectTypeWidget->setLayout(dhcpLayout);

        QLabel* switchName = new QLabel(tr("Network Switch"));
        switchName->setFixedSize(130, 20);
        connect(m_switchButton, &DSwitchButton::checkedChanged, this, &NetworkEditWidget::setDeviceEnable);

        QHBoxLayout* switchLayout = new QHBoxLayout;
        switchLayout->setMargin(0);
        switchLayout->setSpacing(0);
        switchLayout->addWidget(switchName, 0, Qt::AlignLeft | Qt::AlignHCenter);
        switchLayout->addWidget(m_switchButton, 0, Qt::AlignRight | Qt::AlignHCenter);
        m_switch->setLayout(switchLayout);

        mainLayout->addWidget(m_switch);
        mainLayout->addWidget(m_connectTypeWidget);
        mainLayout->addWidget(m_ipWidget);
        mainLayout->addWidget(m_maskWidget);
        mainLayout->addWidget(m_gatewayWidget);
        mainLayout->addWidget(m_primaryDNSWidget);

        m_validityCheck = std::unique_ptr<
            QRegularExpressionValidator>(new QRegularExpressionValidator(QRegularExpression(
            "((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)?\\.){0,3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)?")));

        mainLayout->addStretch();
        mainLayout->addWidget(m_editBtn, 0, Qt::AlignHCenter);
        mainLayout->addWidget(m_acceptBtn, 0, Qt::AlignHCenter);

        setLayout(mainLayout);

        m_acceptBtn->hide();

        connect(m_ipv4Edit, &DLineEdit::editingFinished, this, &NetworkEditWidget::checkIPValidity);
        connect(m_gatewayEdit, &DLineEdit::editingFinished, this,
                &NetworkEditWidget::checkIPValidity);
        connect(m_primaryDNSEdit, &DLineEdit::editingFinished, this,
                &NetworkEditWidget::checkIPValidity);
        connect(m_maskEdit, &DLineEdit::editingFinished, this,
                &NetworkEditWidget::checkMaskValidity);

        connect(m_editBtn, &QPushButton::clicked, this, &NetworkEditWidget::onEdit);
        connect(m_acceptBtn, &QPushButton::clicked, this, &NetworkEditWidget::onEditFinished);
        connect(m_dhcpTypeWidget, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
                , this, &NetworkEditWidget::onDHCPChanged);
    }

    void setIpConfig(NetworkManager::Device::Ptr dev) {
        if (dev.isNull()) {
            clearIpInfo();
            return;
        }

        NetworkManager::IpConfig ipConfig = dev->ipV4Config();
        if (!ipConfig.isValid()) {
            clearIpInfo();
            return;
        }

        if (ipConfig.addresses().isEmpty()) {
            clearIpInfo();
            return;
        }

        NetworkManager::IpAddress address = ipConfig.addresses().at(0);
        labelHandleMap[m_ipWidget](address.ip().toString());
        labelHandleMap[m_maskWidget](address.netmask().toString());
        labelHandleMap[m_gatewayWidget](address.gateway().toString());
        if (!ipConfig.nameservers().isEmpty()) {
            labelHandleMap[m_primaryDNSWidget](ipConfig.nameservers().at(0).toString());
        }
    }

    void clearIpInfo()
    {
        labelHandleMap[m_ipWidget]("");
        labelHandleMap[m_maskWidget]("");
        labelHandleMap[m_gatewayWidget]("");
        labelHandleMap[m_primaryDNSWidget]("");
    }

    void onEdit() {
        for (auto it = m_widgetList.begin(); it != m_widgetList.end(); ++it) {
            labelShowMap[it->first](false);
            it->second->show();
        }
        m_editBtn->hide();
        m_acceptBtn->show();
        m_dhcpTypeWidget->setCurrentIndex(1);

        m_ipv4Edit->setText(labelTextMap[m_ipWidget]());
        m_maskEdit->setText(labelTextMap[m_maskWidget]());
        m_gatewayEdit->setText(labelTextMap[m_gatewayWidget]());
        m_primaryDNSEdit->setText(labelTextMap[m_primaryDNSWidget]());
        m_dhcpTypeWidget->setEnabled(true);
    }

    void onEditFinished() {
        for (auto it = m_widgetList.begin(); it != m_widgetList.end(); ++it) {
            labelShowMap[it->first](true);
            it->second->hide();
            labelHandleMap[it->first](it->second->text());
        }
        m_editBtn->show();
        m_acceptBtn->hide();
        m_dhcpTypeWidget->setEnabled(false);
        m_errorTip->hide();
    }

    void checkIPValidity()
    {
        DLineEdit *edit = qobject_cast<DLineEdit *>(sender());

        checkEditIPValidity(edit);
    }

    bool checkEditIPValidity(DLineEdit *edit)
    {
        if (!checkip(edit->text())) {
            m_errorTip->setText(tr("IP address error: illegal IP address, please have a check."));
            m_errorTip->showBottom(edit);
            return false;
        }
        else {
            m_errorTip->hide();
            return true;
        }
    }

    bool checkMaskValidity()
    {
        if (!checkMask(m_maskEdit->text())) {
            m_errorTip->setText(tr("Netmask error: illegal netmask, please have a check."));
            m_errorTip->showBottom(m_maskEdit);
            return false;
        }
        else {
            m_errorTip->hide();
            return true;
        }
    }

    bool validate()
    {
        for (auto it = m_editList.begin(); it != m_editList.end(); ++it) {
            if (*it != m_maskEdit) {
                if (!checkEditIPValidity(*it)) {
                    return false;
                }
            }
            else {
                if (!checkMaskValidity()) {
                    return false;
                }
            }
        }

        return true;
    }

    // Hide error tip frame when line-edit is being edited.
    void onEditingLineEdit()
    {
        if (m_errorTip->isVisible()) {
            m_errorTip->hide();
        }
    }

    void onDHCPChanged(int index) {
        m_dhcpType = index == 0 ? DHCPTYpe::Auto : DHCPTYpe::Manual;

        if (m_dhcpType == DHCPTYpe::Manual) {
            onEdit();
        }
        else {
            onEditFinished();
        }
    }

    bool event(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange) {
            m_ipv4Edit->lineEdit()->setPlaceholderText(tr("IP Address"));
            m_maskEdit->lineEdit()->setPlaceholderText(tr("Netmask"));
            m_gatewayEdit->lineEdit()->setPlaceholderText(tr("Gateway"));
            m_primaryDNSEdit->lineEdit()->setPlaceholderText(tr("Primary DNS"));
            m_errorTip->hide();
        }

        return QWidget::event(event);
    }

    void setDevice(NetworkManager::Device::Ptr device) {
        m_device = device;
        m_switchButton->setEnabled(true);
        m_switchButton->setChecked(true);
    }

    NetworkManager::Device::Ptr getDevice() const {
        return m_device;
    }

    void setNetworkOperate(NetworkOperate *networkOperate)
    {
        m_networkOperate = networkOperate;
    }

    NetworkOperate* getNetworkOperate()
    {
        return m_networkOperate;
    }

    QString ip() const {
        return labelTextMap[m_ipWidget]();
    }

    QString mask() const {
        return labelTextMap[m_maskWidget]();
    }

    QString gateway() const {
        return labelTextMap[m_gatewayWidget]();
    }

    QString primaryDNS() const {
        return labelTextMap[m_primaryDNSWidget]();
    }

    DHCPTYpe connectType() const {
        return m_dhcpType;
    }

    void setDeviceEnable(bool enable)
    {
        m_deviceEnable = enable;
        m_editBtn->setEnabled(enable);
    }

    bool getDeviceEnable() const
    {
        return m_deviceEnable;
    }

private:
    QWidget* m_switch;
    DSwitchButton* m_switchButton;
    bool m_deviceEnable = false;
    QWidget* m_connectTypeWidget;
    QWidget* m_ipWidget;
    QWidget* m_maskWidget;
    QWidget* m_gatewayWidget;
    QWidget* m_primaryDNSWidget;
    QList<QPair<QWidget*, DLineEdit*>> m_widgetList;
    QList<DLineEdit *> m_editList;
    QPushButton* m_editBtn;
    QPushButton* m_acceptBtn;
    DLineEdit* m_ipv4Edit;
    DLineEdit* m_maskEdit;
    DLineEdit* m_gatewayEdit;
    DLineEdit* m_primaryDNSEdit;
    DHCPTYpe m_dhcpType;
    std::unique_ptr<QRegularExpressionValidator> m_validityCheck;
    SystemInfoTip*                               m_errorTip;
    QMap<QWidget*, std::function<void (const QString& text)>> labelHandleMap;
    QMap<QWidget*, std::function<void (const bool show)>> labelShowMap;
    QMap<QWidget*, std::function<QString ()>> labelTextMap;
    QComboBox *m_dhcpTypeWidget;
    NetworkManager::Device::Ptr m_device = nullptr;
    NetworkOperate *m_networkOperate = nullptr;
};
}

NetworkFrame::NetworkFrame(FrameProxyInterface *frameProxyInterface, QWidget *parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_nextButton(new QPushButton(tr("Next")))
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(10);

    QLabel *logo_label = new QLabel;
    logo_label->setPixmap(installer::renderPixmap(GetVendorLogo()));

    layout->addWidget(logo_label, 0, Qt::AlignHCenter);

    m_subTitle = new QLabel(tr("Configure Network"));
    layout->addWidget(m_subTitle, 0, Qt::AlignHCenter);

    // 左侧布局
    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(10, 10, 10, 10);
    leftLayout->setSpacing(10);
    QFrame *leftWidget = new QFrame;
    leftWidget->setContentsMargins(0, 0, 0, 0);
    leftWidget->setFixedSize(kViewWidth, kViewHeight);
    leftWidget->setLayout(leftLayout);

    // 右侧布局
    QVBoxLayout* rightLayout = new QVBoxLayout;
    rightLayout->setContentsMargins(10, 10, 10, 10);
    rightLayout->setSpacing(0);

    m_currentNetworkEditWidget = new NetworkEditWidget;
    rightLayout->addWidget(m_currentNetworkEditWidget);
    rightLayout->addStretch();

    QFrame *rightWidget = new QFrame;
    rightWidget->setContentsMargins(0, 0, 0, 0);
    rightWidget->setFixedSize(kViewWidth, kViewHeight);
    rightWidget->setLayout(rightLayout);

    DVerticalLine* dVerticalLine = new DVerticalLine;

    QHBoxLayout* centerLayout = new QHBoxLayout;
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);

    centerLayout->addStretch();
    centerLayout->addWidget(leftWidget, 0, Qt::AlignRight);
    centerLayout->addWidget(dVerticalLine);
    centerLayout->addWidget(rightWidget, 0, Qt::AlignLeft);
    centerLayout->addStretch();

    DFrame *frame = new DFrame;
    frame->setFrameRounded(true);
    frame->setContentsMargins(1, 1, 1, 1);
    frame->setLayout(centerLayout);

    layout->addStretch();
    layout->addWidget(frame, 0, Qt::AlignHCenter);
    layout->addStretch();

    m_nextButton->setFixedSize(kNextButtonWidth, kNextButtonHeight);
    connect(m_nextButton, &QPushButton::clicked, this, &NetworkFrame::saveConf);

    layout->addWidget(m_nextButton, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    setLayout(layout);

    NetworkManager::Device::List list = NetworkManager::networkInterfaces();
    bool hasSet = false;

    foreach (NetworkManager::Device::Ptr dev, list) {
        qDebug() << dev->uni();
        qDebug() << "managed: " << dev->managed();
        qDebug() << "interface name: " << dev->interfaceName();

        if (dev->interfaceName() == "lo" || !dev->managed()) {
            continue;
        }

        NetworkDeviceWidget* deviceWidget = new NetworkDeviceWidget(this);
        deviceWidget->setCheckable(true);
        deviceWidget->setDeviceInfo(dev);
        m_buttonList << deviceWidget;

        if (!hasSet) {
            deviceWidget->setChecked(true);
            deviceWidget->updateCheckedAppearance();
            m_currentNetworkEditWidget->setDevice(dev);
            m_currentNetworkEditWidget->setNetworkOperate(deviceWidget->networkOperate());
            m_currentNetworkEditWidget->setIpConfig(dev);

            hasSet = true;
        }
    }

    m_buttonBox = new DButtonBox;
    m_buttonBox->setButtonList(m_buttonList, true);
    connect(m_buttonBox, &DButtonBox::buttonClicked, this
            , &NetworkFrame::onButtonGroupToggled);

    for (DButtonBoxButton* button : m_buttonList) {
        NetworkDeviceWidget* widget = qobject_cast<NetworkDeviceWidget*>(button);
        leftLayout->addWidget(widget);
    }

    leftLayout->addStretch();
}

QString NetworkFrame::returnFrameName() const
{
    return "Network Config";
}

bool NetworkFrame::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        m_subTitle->setText(tr("Configure Network"));
        m_nextButton->setText(tr("Next"));
    }

    return QWidget::event(event);
}

void NetworkFrame::init()
{

}

void NetworkFrame::finished()
{

}

bool NetworkFrame::shouldDisplay() const
{
    return !(GetSettingsBool(kSystemInfoSetupAfterReboot) || GetSettingsBool(kSkipNetworkPage));
}

void NetworkFrame::saveConf()
{
    // TODO:
    //    if (!m_currentNetworkEditWidget->validate()) {
    //        return;
    //    }

    NetworkManager::Device::Ptr device = m_currentNetworkEditWidget->getDevice();
    if (device.isNull()) {
        m_proxy->nextFrame();
        return;
    }

    NetworkOperate* operate = m_currentNetworkEditWidget->getNetworkOperate();

    if (!m_currentNetworkEditWidget->getDeviceEnable()) {
        operate->setDeviceEnable(device->uni(), false);
    }
    else {
        // If the user has previously disabled the network card
        // , then leave the page. The network card must be enabled
        // until the user come back to this page again.
        operate->setDeviceEnable(device->uni(), true);

        NetworkSettingInfo networkSettingInfo;
        networkSettingInfo.setIpMode = m_currentNetworkEditWidget->connectType();
        if (m_currentNetworkEditWidget->connectType() == DHCPTYpe::Manual) {
            networkSettingInfo.ip = m_currentNetworkEditWidget->ip();
            networkSettingInfo.mask = m_currentNetworkEditWidget->mask();
            networkSettingInfo.gateway = m_currentNetworkEditWidget->gateway();
            networkSettingInfo.primaryDNS = m_currentNetworkEditWidget->primaryDNS();
        }

        // If at least one network connection is found, then config it.
        // Else create one network connection with the configuration.
        if (!operate->getConnection().isNull()) {
            if (!operate->setIpV4(networkSettingInfo)) {
                qDebug() << "saveConf() set ipV4 failed";
            }
        }
        else {
            if (operate->createNetworkConnection()) {
                if (!operate->setIpV4(networkSettingInfo)) {
                    qDebug() << "saveConf() set ipV4 failed";
                }
            }
            else {
                qDebug() << "saveConf() create network connection failed";
            }
        }

        operate->setDeviceEnable(device->uni(), true);
    }

    m_proxy->nextFrame();
}

void NetworkFrame::onButtonGroupToggled(QAbstractButton *button)
{
    for (DButtonBoxButton* button : m_buttonList) {
        qobject_cast<NetworkDeviceWidget *>(button)->updateCheckedAppearance();
    }

    NetworkDeviceWidget* device = qobject_cast<NetworkDeviceWidget*>(button);

    // TODO: delete two.
    m_currentNetworkEditWidget->setIpConfig(device->getDevice());
    m_currentNetworkEditWidget->setDevice(device->getDevice());
    m_currentNetworkEditWidget->setNetworkOperate(device->networkOperate());
}

#include "networkframe.moc"
