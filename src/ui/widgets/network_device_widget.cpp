#include "network_device_widget.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/auto_wrap_label.h"

#include <QStyleOption>
#include <QPainter>
#include <QEvent>
#include <QPainterPath>

namespace installer {

namespace {
    const int kNetworkDeviceWidgetWidth = 240;
    const int kNetworkDeviceWidgetHeight = 90;
    const int KQLabelWidth = 200;
    const int kTitleFont = 14; // 14pt
    const int kDescFont = 12; // 12pt
}

NetworkDeviceWidget::NetworkDeviceWidget(QWidget *parent)
    : DButtonBoxButton("", parent)
    , m_isflag(false)
    , m_deviceEnable(true)
    , m_dhcpType(DHCPTYpe::Auto)
{
    m_deviceName = new AutoWrapLabel;
    m_deviceName->setObjectName("titleLabel");
    m_deviceName->setFixedWidth(KQLabelWidth);
    m_deviceName->setWordWrap(true);
    m_deviceName->adjustSize();

    QFont titleFont;
    titleFont.setPointSize(kTitleFont);
    m_deviceName->setFont(titleFont);

    m_descLabel = new QLabel;
    m_descLabel->setObjectName("descLabel");
    m_descLabel->setFixedWidth(KQLabelWidth);
    m_descLabel->setWordWrap(true);
    m_descLabel->adjustSize();

    QFont descFont;
    descFont.setPointSize(kDescFont);
    m_descLabel->setFont(descFont);

    m_vLayout = new QVBoxLayout;
    m_vLayout->setContentsMargins(0, 0, 0, 0);
    m_vLayout->setSpacing(0);
    m_vLayout->addStretch();
    m_vLayout->addWidget(m_deviceName, 0, Qt::AlignLeft);
    m_vLayout->addWidget(m_descLabel, 0, Qt::AlignLeft);
    m_vLayout->addStretch();

    m_checkedLabel = new QLabel;
    const QPixmap pixmap = installer::renderPixmap(":/images/select.svg");
    Q_ASSERT(!pixmap.isNull());
    m_checkedLabel->setPixmap(pixmap);
    m_checkedLabel->setFixedSize(pixmap.size() / devicePixelRatioF());
    m_checkedLabel->setVisible(isChecked());

    m_hLayout = new QHBoxLayout;
    m_hLayout->setContentsMargins(10, 10, 10, 10);
    m_hLayout->setSpacing(0);

    m_hLayout->addLayout(m_vLayout);
    m_hLayout->addWidget(m_checkedLabel, 0, Qt::AlignRight);

    setFixedWidth(kNetworkDeviceWidgetWidth);
    setMaximumHeight(kNetworkDeviceWidgetHeight);
    setObjectName("PartitionTableWarningWidget");
    setContentsMargins(0, 0, 0, 0);
    setLayout(m_hLayout);
}

void NetworkDeviceWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath PaintPath;
    QColor color(0, 0, 0, 15);

    if (m_isflag) {
        color = QColor(0, 0, 0, 55);
    }

    PaintPath.addRoundedRect(rect(), 10, 10);
    painter.fillPath(PaintPath, color);

    QWidget::paintEvent(event);
}

void NetworkDeviceWidget::enterEvent(QEvent* event)
{
    m_isflag = true;

    update();

    QWidget::leaveEvent(event);
}

void NetworkDeviceWidget::leaveEvent(QEvent* event)
{
    m_isflag = false;

    update();

    QWidget::leaveEvent(event);
}

void NetworkDeviceWidget::setTitle(const QString &title)
{
    m_deviceName->setText(title);
}

void NetworkDeviceWidget::setDesc(const QString &desc)
{
    m_descLabel->setText(desc);
}

void NetworkDeviceWidget::setNetworkSettingInfo(const QMap<DHCPTYpe, NetworkSettingInfo> &info)
{
    m_networkSettingInfo = info;
}

QMap<DHCPTYpe, NetworkSettingInfo> NetworkDeviceWidget::getNetworkSettingInfo() const
{
    return m_networkSettingInfo;
}

void NetworkDeviceWidget::readNetworkSettingInfo()
{
    if (m_device.isNull()) {
        return;
    }

    NetworkManager::IpConfig ipConfig = m_device->ipV4Config();
    if (!ipConfig.isValid()) {
        return;
    }

    if (ipConfig.addresses().isEmpty()) {
        return;
    }

    NetworkManager::IpAddress address = ipConfig.addresses().at(0);

    NetworkSettingInfo networkSettingInfo;
    networkSettingInfo.setIpMode = m_networkOperate->getDhcp();
    setDhcp(m_networkOperate->getDhcp());

    networkSettingInfo.ip = address.ip().toString();
    networkSettingInfo.mask = address.netmask().toString();
    networkSettingInfo.gateway = ipConfig.gateway();
    if (!ipConfig.nameservers().isEmpty()) {
        networkSettingInfo.primaryDNS = ipConfig.nameservers().at(0).toString();
        if (ipConfig.nameservers().count() > 1) {
            networkSettingInfo.secondaryDNS = ipConfig.nameservers().at(1).toString();
        }
    }

    m_networkSettingInfo[networkSettingInfo.setIpMode] = networkSettingInfo;

    // If set ip mode is auto, then the manual configuration is the same as the automatic configuration.
    // If set ip mode is manual, then the auto configuration ip info is empty.
    if (networkSettingInfo.setIpMode == DHCPTYpe::Auto) {
        networkSettingInfo.setIpMode = DHCPTYpe::Manual;
        m_networkSettingInfo[networkSettingInfo.setIpMode] = networkSettingInfo;
    }
    else {
        NetworkSettingInfo info;
        info.setIpMode = DHCPTYpe::Auto;
        m_networkSettingInfo[info.setIpMode] = info;
    }
}

bool NetworkDeviceWidget::deviceEnable() const
{
    return m_deviceEnable;
}

void NetworkDeviceWidget::setDeviceEnable(const bool enable)
{
    m_deviceEnable = enable;
}

DHCPTYpe NetworkDeviceWidget::getDhcp() const
{
    return m_dhcpType;
}

void NetworkDeviceWidget::setDhcp(const DHCPTYpe dhcp)
{
    m_dhcpType = dhcp;
}

void NetworkDeviceWidget::updateCheckedAppearance()
{
    m_checkedLabel->setVisible(isChecked());
}

void NetworkDeviceWidget::setDeviceInfo(NetworkManager::Device::Ptr device) {
    qDebug() << "Device type: " << device->type();

    if (device->type() == NetworkManager::Device::Type::Ethernet) {
        setTitle(tr("Ethernet (%1)").arg(device->interfaceName()));
    }
    else if (device->type() == NetworkManager::Device::Type::Wifi) {
        setTitle(tr("Wifi (%1)").arg(device->interfaceName()));
    }
    else {
        setTitle(tr("UnknownDevice (%1)").arg(device->interfaceName()));
    }

    m_device = device;
    m_networkOperate = new NetworkOperate(device);

    readNetworkSettingInfo();
}

NetworkManager::Device::Ptr NetworkDeviceWidget::getDevice() const {
    return m_device;
}

NetworkOperate *NetworkDeviceWidget::networkOperate() const
{
    return m_networkOperate;
}

QString NetworkDeviceWidget::getDeviceType() const
{
    if (m_device.isNull()) {
        qWarning() << "getDeviceType() member device ptr is null";
        return "";
    }

    QString deviceTypeName;

    qDebug() << "Device type: " << m_device->type();

    if (m_device->type() == NetworkManager::Device::Type::Ethernet) {
        deviceTypeName = "Ethernet";
    }
    else if (m_device->type() == NetworkManager::Device::Type::Wifi) {
        deviceTypeName = "Wifi";
    }
    else {
        deviceTypeName = "UnknownType";
    }

    return deviceTypeName;
}

}
