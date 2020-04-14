#include "network_operate.h"

using namespace NetworkManager;

namespace installer {
NetworkOperate::NetworkOperate(const QString &interfaceName, QObject* parent)
    : QObject(parent)
    , m_interfaceName(interfaceName)
    , m_device(nullptr)
    , m_connection(nullptr)
    , m_activeConnection(nullptr)
{
    NetworkManager::Device::List list = NetworkManager::networkInterfaces();

    for (NetworkManager::Device::Ptr dev : list) {
        if (dev->interfaceName() == m_interfaceName) {
            m_device = dev;
        }
    }

    setConnection();
}

NetworkOperate::~NetworkOperate()
{}

void NetworkOperate::setConnection()
{
    if (!m_device) {
        return;
    }

    for (ActiveConnection::Ptr aConn : activeConnections()) {
        for (QString devPath : aConn->devices()) {
            qDebug() << "devPath:" << devPath;

            if (devPath == m_device->uni()) {
                m_connection = findConnectionByUuid(aConn->uuid());
                if (!m_connection) {
                    qDebug() << "can't find connection by uuid:" << aConn->uuid();
                    return;
                }

                m_activeConnection = aConn;
                return;
            }
        }
    }
}

bool NetworkOperate::setIpV4(NetworkSettingInfo info)
{
    if (!m_connection) {
        return false;
    }

    NetworkManager::ConnectionSettings::Ptr settings = m_connection->settings();
    NetworkManager::Ipv4Setting::Ptr ipv4Setting
            = settings->setting(Setting::Ipv4).dynamicCast<Ipv4Setting>();

    NetworkManager::IpAddress ipAddress;
    ipAddress.setIp(QHostAddress(info.ip));
    ipAddress.setNetmask(QHostAddress(info.mask));
    ipAddress.setGateway(QHostAddress(info.gateway));
    ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Manual);
    ipv4Setting->setAddresses(QList<NetworkManager::IpAddress>() << ipAddress);

    QDBusPendingReply<> reply = deactivateConnection(m_activeConnection->path());
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "error occurred while deactivate connection" << reply.error();
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
}
