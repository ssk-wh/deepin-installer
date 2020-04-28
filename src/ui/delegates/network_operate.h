#pragma once

#include <arpa/inet.h>

#include <QObject>
#include <QString>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/IpAddress>
#include <NetworkManagerQt/IpConfig>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Settings>

using namespace NetworkManager;

namespace installer {
enum class DHCPTYpe {
    Auto = 0,
    Manual,
};

typedef struct tagNetworkSettingInfo {
    QString ip;
    QString mask;
    QString gateway;
    QString primaryDNS;
    DHCPTYpe setIpMode = DHCPTYpe::Auto;
} NetworkSettingInfo;

class NetworkOperate : public QObject {
    Q_OBJECT
public:
    explicit NetworkOperate(NetworkManager::Device::Ptr device, QObject* parent = nullptr);
    ~NetworkOperate();

    bool createNetworkConnection();
    void setNetworkConnection();
    bool setIpV4(NetworkSettingInfo info);
    void setDeviceEnable(const QString &devPath, const bool enable);
    bool getDeviceEnable(const QString &devPath);

    NetworkManager::Connection::Ptr getConnection() const;

private:
    QString m_interfaceName;
    NetworkManager::Device::Ptr m_device = nullptr;
    NetworkManager::Connection::Ptr m_connection = nullptr;
    ActiveConnection::Ptr m_activeConnection = nullptr;
};
}  // namespace installer
