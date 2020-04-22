#include "network_operate.h"

#include <QtDBus/QtDBus>
#include <QtDBus/QDBusAbstractInterface>

using namespace NetworkManager;

namespace installer {
// TODO: delete interfaceName.
NetworkOperate::NetworkOperate(NetworkManager::Device::Ptr device, QObject* parent)
    : QObject(parent)
    , m_device(device)
    , m_connection(nullptr)
    , m_activeConnection(nullptr)
{
    Q_ASSERT(m_device != nullptr);
    m_interfaceName = m_device->interfaceName();

    setNetworkConnection();
}

NetworkOperate::~NetworkOperate()
{}

void NetworkOperate::setNetworkConnection()
{
    m_activeConnection = m_device->activeConnection();
    if (!m_activeConnection.isNull()) {
        m_connection = findConnectionByUuid(m_activeConnection->uuid());
    }
    else {
        qDebug() << "This device has no active connection";
    }
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
        qDebug() << "create connection failed..." << reply.error();
        return false;
    }

    // update function saves the settings on the hard disk
    QDBusPendingReply<> reply2 = m_connection->update(connectionSettings->toMap());
    reply2.waitForFinished();
    if (reply2.isError()) {
        qDebug() << "error occurred while updating the connection" << reply2.error();
        return false;
    }

    reply = activateConnection(m_connection->path(), m_device->uni(), "");
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "error occurred while activate connection" << reply.error();
        return false;
    }

    return true;
}

bool NetworkOperate::setIpV4(NetworkSettingInfo info)
{
    if (!m_connection) {
        qDebug() << "setIpV4() connection is null";
        return false;
    }

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
    }
    else {
        ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Automatic);
    }

    QDBusPendingReply<> reply;
    if (!m_activeConnection.isNull()) {
        reply = deactivateConnection(m_activeConnection->path());
        reply.waitForFinished();
        if (reply.isError()) {
            qDebug() << "error occurred while deactivate connection" << reply.error();
            // Go on, don't return here.
        }
    }

    // update function saves the settings on the hard disk
    reply = m_connection->update(settings->toMap());
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "error occurred while updating the connection" << reply.error();
        return false;
    }

    reply = activateConnection(m_connection->path(), m_device->uni(), "");
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "error occurred while activate connection" << reply.error();
        return false;
    }

    return true;
}

void NetworkOperate::setDeviceEnable(const QString &devPath, const bool enable)
{
    QDBusInterface deviceManager("com.deepin.daemon.Network",
              "/com/deepin/daemon/Network",
              "com.deepin.daemon.Network",
              QDBusConnection::sessionBus());

    QList<QVariant> arg;
    arg << QVariant::fromValue(QDBusObjectPath(devPath)) << QVariant::fromValue(enable);

    deviceManager.callWithArgumentList(QDBus::Block, "EnableDevice", arg);
}

}
