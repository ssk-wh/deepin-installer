#include "network_operate.h"

#include <QtDBus/QtDBus>
#include <QtDBus/QDBusAbstractInterface>

using namespace NetworkManager;

namespace installer {

NetworkOperate::NetworkOperate(NetworkManager::Device::Ptr device, QObject* parent)
    : QObject(parent)
    , m_device(device)
    , m_connection(nullptr)
    , m_configMethod(DHCPTYpe::Auto)
{
    Q_ASSERT(m_device != nullptr);
    m_interfaceName = m_device->interfaceName();

    initNetworkConnection();
}

NetworkOperate::~NetworkOperate()
{}

void NetworkOperate::initNetworkConnection()
{
    qDebug() << "The device " << m_device->interfaceName() << " availableConnections:";
    foreach (Connection::Ptr conn , m_device->availableConnections()) {
        qDebug() << conn->path();
    }

    ActiveConnection::Ptr activeConnection = m_device->activeConnection();
    if (!activeConnection.isNull()) {
        m_connection = findConnectionByUuid(activeConnection->uuid());

        m_connectionSettings = m_connection->settings();

        QString connName = QString("%1-lap").arg(m_device->interfaceName());
        QString tmp = m_connectionSettings->id();
        m_connectionSettings->setId(connName);
        qInfo() << QString("Update network connection id %1 to %2").arg(tmp).arg(connName);

        NetworkManager::Ipv4Setting::Ptr ipv4Setting
                = m_connectionSettings->setting(Setting::Ipv4).dynamicCast<Ipv4Setting>();
        m_configMethod = ipv4Setting->method() == NetworkManager::Ipv4Setting::ConfigMethod::Manual ?
                    DHCPTYpe::Manual : DHCPTYpe::Auto;
    }
    else {
        qInfo() << "This device has no active connection";

        Connection::List connections = m_device->availableConnections();

        if (connections.isEmpty()) {
            if (!createNetworkConnection()) {
                qInfo() << "initNetworkConnection() create network connection failed";
            }
        }
        else {
            m_connection = connections.first();

            if (!activateConn()) {
                qCritical() << "initNetworkConnection() active connection failed";
            }
        }
    }

    m_connectionSettings = m_connection->settings();
}

Connection::Ptr NetworkOperate::getConnection() const
{
    return m_connection;
}

bool NetworkOperate::createNetworkConnection()
{
    NetworkManager::ConnectionSettings::Ptr connectionSettings =
            QSharedPointer<NetworkManager::ConnectionSettings>(
                new NetworkManager::ConnectionSettings(
                    NetworkManager::ConnectionSettings::ConnectionType::Wired));
    connectionSettings->setInterfaceName(m_device->interfaceName());
    QString connName = QString("%1-lap").arg(m_device->interfaceName());
    connectionSettings->setId(connName);
    connectionSettings->setUuid(connectionSettings->createNewUuid());

    QDBusPendingReply<QDBusObjectPath> reply = addConnection(connectionSettings->toMap());
    reply.waitForFinished();
    const QString &connPath = reply.value().path();
    m_connection = findConnection(connPath);
    if (!m_connection) {
        qCritical() << "create connection failed..." << reply.error();
        return false;
    }

    // update function saves the settings on the hard disk
    QDBusPendingReply<> reply2 = m_connection->update(connectionSettings->toMap());
    reply2.waitForFinished();
    if (reply2.isError()) {
        qCritical() << "error occurred while updating the connection" << reply2.error();
        return false;
    }

    if (!activateConn()) {
        qCritical() << "createNetworkConnection() active connection failed";
        return false;
    }

    return true;
}

bool NetworkOperate::setIpV4(NetworkSettingInfo info)
{
    Q_ASSERT(m_connection);

    qDebug() << "The device " << m_device->interfaceName() << " availableConnections:";
    foreach (Connection::Ptr conn , m_device->availableConnections()) {
        qDebug() << conn->path();
    }

    qDebug() << "Current setting connection uuid: " << m_connection->uuid();

    NetworkManager::ConnectionSettings::Ptr settings = m_connection->settings();
    NetworkManager::Ipv4Setting::Ptr ipv4Setting
            = settings->setting(Setting::Ipv4).dynamicCast<Ipv4Setting>();

    NetworkManager::IpAddress ipAddress;
    ipAddress.setIp(QHostAddress(info.ip));
    ipAddress.setNetmask(QHostAddress(info.mask));
    ipAddress.setGateway(QHostAddress(info.gateway));
    if (info.setIpMode == DHCPTYpe::Manual){
        ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Manual);
        ipv4Setting->setAddresses(QList<NetworkManager::IpAddress>() << ipAddress);
        ipv4Setting->setDns(QList<QHostAddress>() << QHostAddress(info.primaryDNS)
                                                  << QHostAddress(info.secondaryDNS));
    }
    else {
        ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Automatic);

        QList<NetworkManager::IpAddress>().clear();
        NetworkManager::IpAddress ipAddressAuto;
        ipAddressAuto.setIp(QHostAddress(""));
        ipAddressAuto.setNetmask(QHostAddress(""));
        ipAddressAuto.setGateway(QHostAddress(""));
        ipv4Setting->setAddresses(QList<NetworkManager::IpAddress>() << ipAddressAuto);

        ipv4Setting->setDns(QList<QHostAddress>());
    }

    QDBusPendingReply<> reply;
    ActiveConnection::Ptr activeConnection = m_device->activeConnection();

    if (info.setIpMode == DHCPTYpe::Auto) {
        if (activeConnection.isNull()) {
            qCritical() << "The connection has not corresponding active connection";
        }
        else {
            reply = deactivateConnection(activeConnection->path());
            reply.waitForFinished();
            if (reply.isError()) {
                qDebug() << "error occurred while deactivate connection" << reply.error();
                // Go on, don't return here.
            }
        }
    }

    // update function saves the settings on the hard disk
    reply = m_connection->update(settings->toMap());
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "error occurred while updating the connection" << reply.error();
        return false;
    }

    if (info.setIpMode == DHCPTYpe::Auto) {
        if (!activateConn()) {
            qCritical() << "setIpV4() active connection failed";
            return false;
        }
    }

    return true;
}

void NetworkOperate::setDeviceEnable(const QString &devPath, const bool enable)
{
    QDBusInterface ddeNetworkMnager(
              "com.deepin.daemon.Network",
              "/com/deepin/daemon/Network",
              "com.deepin.daemon.Network",
              QDBusConnection::sessionBus());

    QFile ddeSessionDaemon("/usr/lib/deepin-daemon/dde-session-daemon");
    if (ddeSessionDaemon.exists()) {
        setDeviceEnableByDdeBus(ddeNetworkMnager, devPath, enable);
    }
    else {
        qCritical() << "com.deepin.daemon.Network daemon is invalid";

        QDBusInterface freedesktopNetworkMnager("org.freedesktop.NetworkManager",
                                           devPath,
                                           "org.freedesktop.NetworkManager.Device",
                                           QDBusConnection::systemBus());
        if (!freedesktopNetworkMnager.isValid()) {
            qCritical() << "org.freedesktop.NetworkManager daemon is invalid";
            return;
        }

        setDeviceEnableByNetworkBus(freedesktopNetworkMnager, devPath, enable);
    }

    if (enable) {
        if (m_connection) {
            if (!activateConn()) {
                qCritical() << "setDeviceEnable() active connection failed";
            }
        }
    }
}

void NetworkOperate::setDeviceEnableByNetworkBus(QDBusInterface &deviceManager, const QString &devPath, const bool enable)
{
    QList<QVariant> arg;

    if (!enable) {
        QDBusPendingReply<> reply = deviceManager.asyncCallWithArgumentList(QStringLiteral("Disconnect"), arg);
        reply.waitForFinished();
    }
}

void NetworkOperate::setDeviceEnableByDdeBus(QDBusInterface &deviceManager, const QString &devPath, const bool enable)
{
    QList<QVariant> arg;
    arg << QVariant::fromValue(QDBusObjectPath(devPath)) << QVariant::fromValue(enable);

    deviceManager.callWithArgumentList(QDBus::Block, "EnableDevice", arg);
}

bool NetworkOperate::getDeviceEnable(const QString &devPath)
{
    QDBusInterface deviceManager("com.deepin.daemon.Network",
              "/com/deepin/daemon/Network",
              "com.deepin.daemon.Network",
              QDBusConnection::sessionBus());

    QList<QVariant> arg;
    arg << QVariant::fromValue(QDBusObjectPath(devPath));
    QDBusPendingReply<bool> reply = deviceManager.callWithArgumentList(QDBus::Block, "IsDeviceEnabled", arg);

    bool enable = reply.value();
    return enable;
}

DHCPTYpe NetworkOperate::getDhcp() const
{
    return m_configMethod;
}

bool NetworkOperate::activateConn()
{
    QDBusPendingReply<QDBusObjectPath> reply = activateConnection(m_connection->path(), m_device->uni(), "");
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "error occurred while activate connection" << reply.error();
        return false;
    }

    return true;
}

bool NetworkOperate::isIpv4Address(const QString &ip)
{
    QHostAddress ipAddr(ip);
    if (ipAddr == QHostAddress(QHostAddress::Null) || ipAddr == QHostAddress(QHostAddress::AnyIPv4)
            || ipAddr.protocol() != QAbstractSocket::NetworkLayerProtocol::IPv4Protocol) {
        return false;
    }
    return true;
}

// TODO: only working for manual config, will failed if auto.
void NetworkOperate::readIpInfo(NetworkSettingInfo& networkSettingInfo)
{
    if (!m_connectionSettings) {
        qCritical() << "Connection settings is nullptr";
        return;
    }

    NetworkManager::Ipv4Setting::Ptr ipv4Setting
            = m_connectionSettings->setting(Setting::Ipv4).staticCast<NetworkManager::Ipv4Setting>();

    QList<NetworkManager::IpAddress> ipAddressList = ipv4Setting->addresses();
    if (!ipAddressList.isEmpty()) {
        // Use the first ipaddress of list
        NetworkManager::IpAddress ipAddress = ipAddressList.first();
        networkSettingInfo.ip = ipAddress.ip().toString();
        networkSettingInfo.mask = ipAddress.netmask().toString();
        const QString &gateStr = ipAddress.gateway().toString();
        networkSettingInfo.gateway = isIpv4Address(gateStr) ? gateStr : "";
    } else {
        networkSettingInfo.ip = "0.0.0.0";
        networkSettingInfo.mask = "255.255.255.0";
    }

    const QList<QHostAddress> &dns = ipv4Setting->dns();
    for (int i = 0; i < dns.size(); ++i) {
        if (i == 0) {
            networkSettingInfo.primaryDNS = dns.at(i).toString();
        }
        if (i == 1) {
            networkSettingInfo.secondaryDNS = dns.at(i).toString();
        }
    }
}

}
