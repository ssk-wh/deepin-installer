#include "network_device_widget.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/auto_wrap_label.h"
#include "ui/widgets/ticker_label.h"
#include "base/command.h"

#include <QStyleOption>
#include <QPainter>
#include <QEvent>
#include <QPainterPath>

namespace installer {

namespace {
    const int kNetworkDeviceWidgetWidth = 227;
    const int kNetworkDeviceWidgetHeight = 64;
    const int KQLabelWidth = 155;
    const int kTitleFont = 12; // 12pt
    const int kDescFont = 10; // 10pt

    const int kBusInfoPos = 4; // ex. /sys/devices/pci0000:00/0000:00:19.0/net/eno1
}

QString GetVendorInfo(const QString& udi)
{
    QStringList list = udi.split("/");
    qDebug() << "device UDI:" << list;

    if (list.count() > kBusInfoPos) {
        // delete empty string ""
        list.removeFirst();
        QString busInfo = list.at(kBusInfoPos);

        int index;
        if ((index = busInfo.indexOf(":")) >= 0) {
            QString pciId = busInfo.mid(index + 1);
            QString output, err;

            SpawnCmd("lspci", {"-s", pciId}, output, err);
            if (output.isEmpty()) {
                qCritical() << QString("lspci -s %1 command failed. err: %2. output: %3")
                               .arg(pciId).arg(err).arg(output);
                return "";
            }

            qInfo() << QString("lspci -s %1 output: %2").arg(pciId).arg(output);
            if ((index = output.indexOf(": ")) >= 0) {
                output = output.mid(index + 1);
                if ((index = output.indexOf("(rev")) > 0) {
                    output = output.left(index).trimmed();
                }

                qInfo() << "device vendor: " << output;
                return output;
            }
        }
        else {
            qWarning() << "Invalid bus info" << busInfo;
        }
    }
    else {
        qWarning() << "Invalid device UDI:" << udi;
    }

    return "";
}

NetworkDeviceWidget::NetworkDeviceWidget(QWidget *parent)
    : DButtonBoxButton("", parent)
    , m_isflag(false)
    , m_deviceEnable(true)
    , m_dhcpType(DHCPTYpe::Auto)
{
    QFont titleFont;
    titleFont.setPointSize(kTitleFont);

    m_iconLabel = new QLabel;
    QPixmap pixmap = installer::renderPixmap(":/images/ethernet.svg");
    Q_ASSERT(!pixmap.isNull());
    m_iconLabel->setPixmap(pixmap);
    m_iconLabel->setFixedSize(pixmap.size() / devicePixelRatioF());

    m_deviceName = new TickerLabel;
    m_deviceName->setObjectName("titleLabel");
    m_deviceName->setFixedWidth(KQLabelWidth);
    m_deviceName->setFont(titleFont);

    m_descLabel = new QLabel;
    m_descLabel->setObjectName("descLabel");
    m_descLabel->setFixedWidth(KQLabelWidth);
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
    pixmap = installer::renderPixmap(":/images/select_blue.svg");
    Q_ASSERT(!pixmap.isNull());
    m_checkedLabel->setPixmap(pixmap);
    m_checkedLabel->setFixedSize(pixmap.size() / devicePixelRatioF());
    m_checkedLabel->setVisible(isChecked());

    m_hLayout = new QHBoxLayout;
    m_hLayout->setContentsMargins(7, 7, 1, 7);
    m_hLayout->setSpacing(0);

    m_hLayout->addWidget(m_iconLabel, 0, Qt::AlignLeft);
    m_hLayout->addSpacing(2);
    m_hLayout->addLayout(m_vLayout);
    m_hLayout->addWidget(m_checkedLabel, 0, Qt::AlignRight);

    setFixedWidth(kNetworkDeviceWidgetWidth);
    setFixedHeight(kNetworkDeviceWidgetHeight);
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

    m_deviceName->start();

    QWidget::leaveEvent(event);
}

void NetworkDeviceWidget::leaveEvent(QEvent* event)
{
    m_isflag = false;

    update();

    m_deviceName->stop();

    QWidget::leaveEvent(event);
}

void NetworkDeviceWidget::setTitle(const QString &title)
{
    m_deviceName->setText(title);
}

void NetworkDeviceWidget::setDesc(const QString &desc)
{
    m_descLabel->setText(desc);
    m_descLabel->setToolTip(desc);
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
        setTitle(::QObject::tr("Ethernet") + QString(" (%1)").arg(device->interfaceName()));
    }
    else if (device->type() == NetworkManager::Device::Type::Wifi) {
        setTitle(::QObject::tr("WLAN") + QString(" (%1)").arg(device->interfaceName()));
    }
    else {
        setTitle(::QObject::tr("Unknown device") + QString(" (%1)").arg(device->interfaceName()));
    }

    setDesc(GetVendorInfo(device->udi()));

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
        deviceTypeName = ::QObject::tr("Ethernet");
    }
    else if (m_device->type() == NetworkManager::Device::Type::Wifi) {
        deviceTypeName = ::QObject::tr("WLAN");
    }
    else {
        deviceTypeName = ::QObject::tr("Unknown device");
    }

    return deviceTypeName;
}

}
