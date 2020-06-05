#include "networkframe.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/system_info_tip.h"
#include "ui/widgets/network_device_widget.h"
#include "ui/delegates/network_operate.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"

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
        m_secondaryDNSWidget = new QWidget;

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
        m_secondaryDNSEdit = new DLineEdit;
        m_secondaryDNSEdit->setFixedSize(kLineEditWidth, kLineEditHeight);
        m_secondaryDNSEdit->lineEdit()->setFont(font);

        m_ipv4Edit->lineEdit()->setPlaceholderText(tr("IP Address"));
        m_maskEdit->lineEdit()->setPlaceholderText(tr("Netmask"));
        m_gatewayEdit->lineEdit()->setPlaceholderText(tr("Gateway"));
        m_primaryDNSEdit->lineEdit()->setPlaceholderText(tr("Primary DNS"));
        m_secondaryDNSEdit->lineEdit()->setPlaceholderText(tr("Secondary DNS"));

        m_errorTip = new SystemInfoTip(this);
        m_errorTip->hide();

        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->setMargin(0);
        mainLayout->setSpacing(10);

        QMap<QWidget*, QString> tmpM {
            {m_ipWidget, tr("Ip:")},
            {m_maskWidget, tr("Mask:")},
            {m_gatewayWidget, tr("Gateway:")},
            {m_primaryDNSWidget, tr("Primary DNS:")},
            {m_secondaryDNSWidget, tr("Secondary DNS:")},
        };

        m_widgetList = {
            {m_ipWidget, m_ipv4Edit},
            {m_maskWidget, m_maskEdit},
            {m_gatewayWidget, m_gatewayEdit},
            {m_primaryDNSWidget, m_primaryDNSEdit},
            {m_secondaryDNSWidget, m_secondaryDNSEdit},
        };

        m_editList = {
            m_ipv4Edit,
            m_maskEdit,
            m_gatewayEdit,
            m_primaryDNSEdit,
            m_secondaryDNSEdit,
        };

        for (auto it = m_editList.begin(); it != m_editList.end(); ++it) {
            connect(*it, &DLineEdit::textEdited, this, &NetworkEditWidget::onEditingLineEdit);
            (*it)->lineEdit()->setValidator(m_validityCheck.get());
            (*it)->lineEdit()->setAlignment(Qt::AlignCenter);
        }

        {
            QHBoxLayout* ipLayout = new QHBoxLayout;
            ipLayout->setMargin(0);
            ipLayout->setSpacing(0);
            QLabel* ipName = new QLabel(tr("IP Address:"));
            ipName->setFixedSize(100, 20);

            ipLayout->addWidget(ipName, 0, Qt::AlignLeft | Qt::AlignHCenter);
            ipLayout->addWidget(m_ipv4Edit, 0, Qt::AlignRight | Qt::AlignHCenter);
            m_ipWidget->setLayout(ipLayout);
            m_ipWidget->setFixedHeight(kLineEditHeight);

            QHBoxLayout* maskLayout = new QHBoxLayout;
            maskLayout->setMargin(0);
            maskLayout->setSpacing(0);
            QLabel* maskName = new QLabel(tr("Netmask:"));
            maskName->setFixedSize(100, 20);

            maskLayout->addWidget(maskName, 0, Qt::AlignLeft | Qt::AlignHCenter);
            maskLayout->addWidget(m_maskEdit, 0, Qt::AlignRight | Qt::AlignHCenter);
            m_maskWidget->setLayout(maskLayout);
            m_maskWidget->setFixedHeight(kLineEditHeight);

            QHBoxLayout* gatewayLayout = new QHBoxLayout;
            gatewayLayout->setMargin(0);
            gatewayLayout->setSpacing(0);
            QLabel* gatewayName = new QLabel(tr("Gateway:"));
            gatewayName->setFixedSize(100, 20);

            gatewayLayout->addWidget(gatewayName, 0, Qt::AlignLeft | Qt::AlignHCenter);
            gatewayLayout->addWidget(m_gatewayEdit, 0, Qt::AlignRight | Qt::AlignHCenter);
            m_gatewayWidget->setLayout(gatewayLayout);
            m_gatewayWidget->setFixedHeight(kLineEditHeight);

            QHBoxLayout* dnsLayout = new QHBoxLayout;
            dnsLayout->setMargin(0);
            dnsLayout->setSpacing(0);
            QLabel* dnsName = new QLabel(tr("Primary DNS:"));
            dnsName->setFixedSize(100, 20);

            dnsLayout->addWidget(dnsName, 0, Qt::AlignLeft | Qt::AlignHCenter);
            dnsLayout->addWidget(m_primaryDNSEdit, 0, Qt::AlignRight | Qt::AlignHCenter);
            m_primaryDNSWidget->setLayout(dnsLayout);
            m_primaryDNSWidget->setFixedHeight(kLineEditHeight);

            QHBoxLayout* secondaryDnsLayout = new QHBoxLayout;
            secondaryDnsLayout->setMargin(0);
            secondaryDnsLayout->setSpacing(0);
            QLabel* secondaryDnsName = new QLabel(tr("Secondary DNS:"));
            secondaryDnsName->setFixedSize(100, 20);

            secondaryDnsLayout->addWidget(secondaryDnsName, 0, Qt::AlignLeft | Qt::AlignHCenter);
            secondaryDnsLayout->addWidget(m_secondaryDNSEdit, 0, Qt::AlignRight | Qt::AlignHCenter);
            m_secondaryDNSWidget->setLayout(secondaryDnsLayout);
            m_secondaryDNSWidget->setFixedHeight(kLineEditHeight);
        }

        QHBoxLayout* dhcpLayout = new QHBoxLayout;
        dhcpLayout->setMargin(0);
        dhcpLayout->setSpacing(0);

        QLabel* dhcpName = new QLabel(tr("DHCP:"));
        dhcpName->setFixedSize(53, 20);
        dhcpLayout->addWidget(dhcpName, 0, Qt::AlignLeft | Qt::AlignHCenter);
        dhcpLayout->addWidget(m_dhcpTypeWidget, 0, Qt::AlignRight | Qt::AlignHCenter);

        QStringListModel* dhcpTypeModel = new QStringListModel(m_dhcpTypeWidget);
        dhcpTypeModel->setStringList({tr("Auto"), tr("Manual")});
        m_dhcpTypeWidget->setModel(dhcpTypeModel);
        m_dhcpTypeWidget->setFixedSize(kLineEditWidth, kLineEditHeight);

        m_connectTypeWidget->setLayout(dhcpLayout);
        // If has not any devices, label must have default name.
        m_switchName = new QLabel(tr("Network Switch"));
        m_switchName->setFixedSize(130, 20);
        connect(m_switchButton, &DSwitchButton::checkedChanged, this, &NetworkEditWidget::onSwitchStateChanged);

        QHBoxLayout* switchLayout = new QHBoxLayout;
        switchLayout->setMargin(0);
        switchLayout->setSpacing(0);
        switchLayout->addWidget(m_switchName, 0, Qt::AlignLeft | Qt::AlignHCenter);
        switchLayout->addWidget(m_switchButton, 0, Qt::AlignRight | Qt::AlignHCenter);
        m_switch->setLayout(switchLayout);

        mainLayout->addWidget(m_switch);
        mainLayout->addWidget(m_connectTypeWidget);
        mainLayout->addWidget(m_ipWidget);
        mainLayout->addWidget(m_maskWidget);
        mainLayout->addWidget(m_gatewayWidget);
        mainLayout->addWidget(m_primaryDNSWidget);
        mainLayout->addWidget(m_secondaryDNSWidget);

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
        connect(m_secondaryDNSEdit, &DLineEdit::editingFinished, this,
                &NetworkEditWidget::checkIPValidity);
        connect(m_maskEdit, &DLineEdit::editingFinished, this,
                &NetworkEditWidget::checkIPValidity);

        connect(m_editBtn, &QPushButton::clicked, this, &NetworkEditWidget::onEdit);
        connect(m_acceptBtn, &QPushButton::clicked, this, &NetworkEditWidget::onEditFinished);
        connect(m_dhcpTypeWidget, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
                , this, &NetworkEditWidget::onDHCPChanged);

        updateTs();
    }

    void updateTs()
    {
        m_ipv4Edit->lineEdit()->setPlaceholderText(tr("IP Address:"));
        m_maskEdit->lineEdit()->setPlaceholderText(tr("Netmask:"));
        m_gatewayEdit->lineEdit()->setPlaceholderText(tr("Gateway:"));
        m_primaryDNSEdit->lineEdit()->setPlaceholderText(tr("Primary DNS:"));
    }

    void initWidgetState()
    {
        clearWidgetIpInfo();

        m_dhcpTypeWidget->blockSignals(true);
        m_dhcpTypeWidget->setCurrentIndex(0);
        m_dhcpTypeWidget->blockSignals(false);

        m_switchButton->blockSignals(true);
        m_switchButton->setEnabled(false);
        m_switchButton->setChecked(false);
        m_switchButton->blockSignals(false);
    }

    void clearWidgetIpInfo()
    {
        m_ipv4Edit->setText("");
        m_maskEdit->setText("");
        m_gatewayEdit->setText("");
        m_primaryDNSEdit->setText("");
        m_secondaryDNSEdit->setText("");
    }

    void setIpLineEditConfig(const DHCPTYpe dhcp)
    {
        QMap<DHCPTYpe, NetworkSettingInfo> info = getNetworkDeviceWidget()->getNetworkSettingInfo();
        if (info.count() > 0) {
            m_ipv4Edit->setText(info[dhcp].ip);
            m_maskEdit->setText(info[dhcp].mask);
            m_gatewayEdit->setText(info[dhcp].gateway);
            m_primaryDNSEdit->setText(info[dhcp].primaryDNS);
            m_secondaryDNSEdit->setText(info[dhcp].secondaryDNS);
        }
        else {
            clearWidgetIpInfo();
        }
    }

    void readIpConfig() {
        int index = getNetworkDeviceWidget()->getDhcp() == DHCPTYpe::Auto ? 0 : 1;

        m_dhcpTypeWidget->blockSignals(true);
        m_dhcpTypeWidget->setCurrentIndex(index);
        m_dhcpType = index == 0 ? DHCPTYpe::Auto : DHCPTYpe::Manual;
        m_dhcpTypeWidget->blockSignals(false);

        QMap<DHCPTYpe, NetworkSettingInfo> info = getNetworkDeviceWidget()->getNetworkSettingInfo();
        // Info may be empty, for example, able didn't plug.
        if (info.count() > 0) {
            m_ipv4Edit->setText(info[m_dhcpType].ip);
            m_maskEdit->setText(info[m_dhcpType].mask);
            m_gatewayEdit->setText(info[m_dhcpType].gateway);
            m_primaryDNSEdit->setText(info[m_dhcpType].primaryDNS);
            m_secondaryDNSEdit->setText(info[m_dhcpType].secondaryDNS);
        }
        else {
            clearWidgetIpInfo();
        }
    }

    void setLineEditEnable(const bool enable)
    {
        for (auto it = m_widgetList.begin(); it != m_widgetList.end(); ++it) {
            it->second->setEnabled(enable);
        }
    }

    void onEdit() {
        m_dhcpTypeWidget->setEnabled(true);

        m_dhcpTypeWidget->blockSignals(true);
        m_dhcpTypeWidget->setCurrentIndex(1);
        m_dhcpType = DHCPTYpe::Manual;
        setIpLineEditConfig(m_dhcpType);
        m_dhcpTypeWidget->blockSignals(false);

        m_editBtn->hide();
        m_acceptBtn->setEnabled(true);
        m_acceptBtn->show();

        setLineEditEnable(true);
        m_errorTip->hide();
    }

    void saveConfigInfo()
    {
        QMap<DHCPTYpe, NetworkSettingInfo> info
                = getNetworkDeviceWidget()->getNetworkSettingInfo();

        NetworkSettingInfo networkSettingInfo;
        networkSettingInfo.setIpMode = connectType();

        // Save manual config info, auto config does not save.
        if (connectType() == DHCPTYpe::Manual) {
            networkSettingInfo.ip = ip();
            networkSettingInfo.mask = mask();
            networkSettingInfo.gateway = gateway();
            networkSettingInfo.primaryDNS = primaryDNS();
            networkSettingInfo.secondaryDNS = secondaryDNS();

            info[networkSettingInfo.setIpMode] = networkSettingInfo;

            getNetworkDeviceWidget()->setNetworkSettingInfo(info);
        }
    }

    void onEditFinished() {
        if (!validate()) {
            return;
        }

        getNetworkDeviceWidget()->setDhcp(m_dhcpType);

        m_dhcpTypeWidget->setEnabled(false);

        m_editBtn->show();
        m_editBtn->setEnabled(true);
        m_acceptBtn->hide();

        setLineEditEnable(false);
        m_errorTip->hide();

        saveConfigInfo();
    }

    void updateEditStateByDeviceToggle()
    {
        // Will toggle other widget state change.
        m_switchButton->blockSignals(true);
        m_switchButton->setEnabled(true);
        m_switchButton->setChecked(m_deviceWidget->deviceEnable());
        m_switchButton->blockSignals(false);

        m_switchName->setText(m_deviceWidget->getDeviceType());

        onSwitchStateChanged(m_deviceWidget->deviceEnable());
    }

    void checkIPValidity()
    {
        DLineEdit *edit = qobject_cast<DLineEdit *>(sender());

        checkValidity(edit);
    }

    bool checkValidity(DLineEdit *edit)
    {
        if (!checkip(edit->text())) {
            QWidget *parent = qobject_cast<QWidget *>(edit->parent());
            m_errorTip->setText(tr("Illegal %1, please have a check.")
                                .arg(edit->lineEdit()->placeholderText()));
            m_errorTip->setLabelSize(QSize(kLineEditWidth, 60));
            m_errorTip->setRelativePosition(parent->pos());
            m_errorTip->showBottom(edit);
            return false;
        }
        else {
            m_errorTip->hide();
            return true;
        }
    }

    bool checkEditIPValidity(DLineEdit *edit)
    {
        if (!checkip(edit->text())) {
            QWidget *parent = qobject_cast<QWidget *>(edit->parent());
            m_errorTip->setText(tr("IP address error: illegal IP address, please have a check."));
            m_errorTip->setRelativePosition(QPoint(0, parent->pos().y()));
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
            m_errorTip->setRelativePosition(m_maskWidget->pos());
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
        if (!checkValidity(m_ipv4Edit)) {
            return false;
        }

        if (!checkValidity(m_maskEdit)) {
            return false;
        }

        if (!m_gatewayEdit->text().isEmpty()) {
            if (!checkValidity(m_gatewayEdit)) {
                return false;
            }
        }

        if (!m_primaryDNSEdit->text().isEmpty()) {
            if (!checkValidity(m_primaryDNSEdit)) {
                return false;
            }
        }

        // If the first DNS is valid, then check the second DNS.
        if (!m_secondaryDNSEdit->text().isEmpty()) {
            if (!checkValidity(m_secondaryDNSEdit)) {
                return false;
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

        setIpLineEditConfig(m_dhcpType);

        getNetworkDeviceWidget()->setDhcp(m_dhcpType);

        if (m_dhcpType == DHCPTYpe::Manual) {
            m_editBtn->hide();
            m_acceptBtn->setEnabled(true);
            m_acceptBtn->show();

            setLineEditEnable(true);
            m_errorTip->hide();
        }
        else {
            m_dhcpTypeWidget->setEnabled(false);

            m_editBtn->show();
            m_editBtn->setEnabled(true);
            m_acceptBtn->hide();

            setLineEditEnable(false);
            m_errorTip->hide();
        }
    }

    bool event(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange) {
            m_ipv4Edit->lineEdit()->setPlaceholderText(tr("IP Address"));
            m_maskEdit->lineEdit()->setPlaceholderText(tr("Netmask"));
            m_gatewayEdit->lineEdit()->setPlaceholderText(tr("Gateway"));
            m_primaryDNSEdit->lineEdit()->setPlaceholderText(tr("Primary DNS"));
            m_secondaryDNSEdit->lineEdit()->setPlaceholderText(tr("Secondary DNS"));
            m_errorTip->hide();
        }

        return QWidget::event(event);
    }

    void setNetworkDeviceWidget(NetworkDeviceWidget* deviceWidget)
    {
        m_deviceWidget = deviceWidget;
        setDevice(m_deviceWidget->getDevice());
    }

    NetworkDeviceWidget* getNetworkDeviceWidget() const
    {
        return m_deviceWidget;
    }

    void setDevice(NetworkManager::Device::Ptr device) {
        m_device = device;

        setNetworkOperate(m_deviceWidget->networkOperate());
        readIpConfig();
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
        return m_ipv4Edit->text();
    }

    QString mask() const {
        return m_maskEdit->text();
    }

    QString gateway() const {
        return m_gatewayEdit->text();
    }

    QString primaryDNS() const {
        return m_primaryDNSEdit->text();
    }

    QString secondaryDNS() const {
        return m_secondaryDNSEdit->text();
    }

    DHCPTYpe connectType() const {
        return m_dhcpType;
    }

    void onSwitchStateChanged(bool enable)
    {
        m_deviceEnable = enable;

        if (m_deviceEnable) {
            m_dhcpTypeWidget->setEnabled(false);
            setLineEditEnable(false);

            m_editBtn->show();
            m_editBtn->setEnabled(true);
            m_acceptBtn->hide();

            m_errorTip->hide();
        }
        else {
            m_dhcpTypeWidget->setEnabled(false);
            setLineEditEnable(false);

            m_editBtn->show();
            m_editBtn->setEnabled(false);
            m_acceptBtn->hide();

            m_errorTip->hide();
        }

        getNetworkDeviceWidget()->setDeviceEnable(enable);
    }

    bool getDeviceEnable() const
    {
        return m_deviceEnable;
    }

private:
    QWidget* m_switch;
    QLabel* m_switchName;
    DSwitchButton* m_switchButton;
    bool m_deviceEnable = false;
    QWidget* m_connectTypeWidget;
    QWidget* m_ipWidget;
    QWidget* m_maskWidget;
    QWidget* m_gatewayWidget;
    QWidget* m_primaryDNSWidget;
    QWidget* m_secondaryDNSWidget;
    QList<QPair<QWidget*, DLineEdit*>> m_widgetList;
    QList<DLineEdit *> m_editList;
    QPushButton* m_editBtn;
    QPushButton* m_acceptBtn;
    DLineEdit* m_ipv4Edit;
    DLineEdit* m_maskEdit;
    DLineEdit* m_gatewayEdit;
    DLineEdit* m_primaryDNSEdit;
    DLineEdit* m_secondaryDNSEdit;
    DHCPTYpe m_dhcpType;
    std::unique_ptr<QRegularExpressionValidator> m_validityCheck;
    SystemInfoTip*                               m_errorTip;
    QComboBox *m_dhcpTypeWidget;
    NetworkManager::Device::Ptr m_device = nullptr;
    NetworkOperate *m_networkOperate = nullptr;
    NetworkDeviceWidget* m_deviceWidget = nullptr;
};
}

NetworkFrame::NetworkFrame(FrameProxyInterface *frameProxyInterface, QWidget *parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_nextButton(new QPushButton(tr("Next")))
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(10);

    title_label_ = new TitleLabel(tr("Configure Network"));
    comment_label_ = new CommentLabel(tr("IP address has been auto-configured, but you can configure the network as well"));

    layout->addWidget(title_label_, 0, Qt::AlignHCenter);
    layout->addWidget(comment_label_, 0, Qt::AlignHCenter);

    // 左侧布局
    m_leftLayout = new QVBoxLayout;
    m_leftLayout->setContentsMargins(10, 10, 10, 10);
    m_leftLayout->setSpacing(10);
    QFrame *leftWidget = new QFrame;
    leftWidget->setContentsMargins(0, 0, 0, 0);
    leftWidget->setFixedSize(kViewWidth, kViewHeight);
    leftWidget->setLayout(m_leftLayout);

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

    m_buttonBox = new DButtonBox;
    initDeviceWidgetList();

    updateTs();
}

void NetworkFrame::initDeviceWidgetList()
{
    ClearLayout(m_leftLayout);
    m_buttonList.clear();

    NetworkManager::Device::List list = NetworkManager::networkInterfaces();
    bool hasSet = false;

    foreach (NetworkManager::Device::Ptr dev, list) {
        qDebug() << dev->uni();
        qDebug() << "managed: " << dev->managed();
        qDebug() << "type: " << dev->type();
        qDebug() << "interface name: " << dev->interfaceName();

        // FIXME: what about !dev->managed()
        if (dev->interfaceName() == "lo"
            || dev->interfaceName().contains("vmnet")
            || dev->type() == NetworkManager::Device::Type::Wifi) {
            continue;
        }

        NetworkDeviceWidget* deviceWidget = new NetworkDeviceWidget(this);
        deviceWidget->setCheckable(true);
        deviceWidget->setDeviceInfo(dev);
        bool enable = deviceWidget->networkOperate()->getDeviceEnable(dev->uni());
        deviceWidget->setDeviceEnable(enable);
        m_buttonList << deviceWidget;

        if (!hasSet) {
            deviceWidget->setChecked(true);
            deviceWidget->updateCheckedAppearance();
            m_currentNetworkEditWidget->setNetworkDeviceWidget(deviceWidget);
            m_currentNetworkEditWidget->updateEditStateByDeviceToggle();

            hasSet = true;
        }
    }

    m_buttonBox->setButtonList(m_buttonList, true);
    connect(m_buttonBox, &DButtonBox::buttonClicked, this
            , &NetworkFrame::onButtonGroupToggled);

    for (DButtonBoxButton* button : m_buttonList) {
        NetworkDeviceWidget* widget = qobject_cast<NetworkDeviceWidget*>(button);
        m_leftLayout->addWidget(widget);
    }

    m_leftLayout->addStretch();
}

QString NetworkFrame::returnFrameName() const
{
    return "Network Config";
}

bool NetworkFrame::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        updateTs();
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

void NetworkFrame::showEvent(QShowEvent *event)
{
    m_currentNetworkEditWidget->initWidgetState();
    initDeviceWidgetList();
}

void NetworkFrame::saveConf()
{
    for (DButtonBoxButton* button : m_buttonList) {
        qobject_cast<NetworkDeviceWidget *>(button)->updateCheckedAppearance();
        NetworkDeviceWidget* deviceWidget = qobject_cast<NetworkDeviceWidget*>(button);

        NetworkManager::Device::Ptr device = deviceWidget->getDevice();
        Q_ASSERT(!device.isNull());

        NetworkOperate* operate = deviceWidget->networkOperate();

        if (!deviceWidget->deviceEnable()) {
            operate->setDeviceEnable(device->uni(), false);
        }
        else {
            // If the user has previously disabled the network card
            // , then leave the page. The network card must be enabled
            // until the user come back to this page again.
            operate->setDeviceEnable(device->uni(), true);

            QMap<DHCPTYpe, NetworkSettingInfo> inMap = deviceWidget->getNetworkSettingInfo();
            if (inMap.count() > 0) {
                NetworkSettingInfo networkSettingInfo = inMap[deviceWidget->getDhcp()];

                // If at least one network connection is found, then config it.
                // Else create one network connection with the configuration.
                if (!operate->setIpV4(networkSettingInfo)) {
                    qDebug() << "saveConf() set ipV4 failed";
                }

                operate->setDeviceEnable(device->uni(), true);
            }
        }
    }

//#ifndef QT_DEBUG
    m_proxy->nextFrame();
//#endif
}

void NetworkFrame::updateTs()
{
    m_currentNetworkEditWidget->updateTs();
    title_label_->setText(tr("Configure Network"));
    comment_label_->setText(tr("IP address has been auto-configured, but you can configure the network as well"));
    m_nextButton->setText(tr("Next"));
}

void NetworkFrame::onButtonGroupToggled(QAbstractButton *button)
{
    for (DButtonBoxButton* button : m_buttonList) {
        qobject_cast<NetworkDeviceWidget *>(button)->updateCheckedAppearance();
    }

    NetworkDeviceWidget* deviceWidget = qobject_cast<NetworkDeviceWidget*>(button);

    // TODO: delete two.
    m_currentNetworkEditWidget->initWidgetState();
    m_currentNetworkEditWidget->setNetworkDeviceWidget(deviceWidget);
    m_currentNetworkEditWidget->updateEditStateByDeviceToggle();
}

#include "networkframe.moc"
