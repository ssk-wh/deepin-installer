#pragma once

#include "ui/delegates/network_operate.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QNetworkInterface>
#include <DButtonBox>

DWIDGET_USE_NAMESPACE

namespace installer {
class AutoWrapLabel;
class TickerLabel;

class NetworkDeviceWidget : public DButtonBoxButton
{
    Q_OBJECT

public:
    explicit NetworkDeviceWidget(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setDesc(const QString& desc);

    void readNetworkSettingInfo();
    void setNetworkSettingInfo(const QMap<DHCPTYpe, NetworkSettingInfo>& info);
    QMap<DHCPTYpe, NetworkSettingInfo> getNetworkSettingInfo() const;

    bool deviceEnable() const;
    void setDeviceEnable(const bool enable);

    DHCPTYpe getDhcp() const;
    void setDhcp(const DHCPTYpe dhcp);

    void updateCheckedAppearance();

    void setDeviceInfo(NetworkManager::Device::Ptr device);
    NetworkManager::Device::Ptr getDevice() const;
    NetworkOperate* networkOperate() const;

    QString getDeviceType() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QHBoxLayout* m_hLayout;
    QVBoxLayout* m_vLayout;
    TickerLabel* m_deviceName;
    QLabel* m_descLabel;
    QLabel *m_checkedLabel;
    bool m_isflag;

    NetworkManager::Device::Ptr m_device = nullptr;
    NetworkOperate *m_networkOperate = nullptr;
    QMap<DHCPTYpe, NetworkSettingInfo> m_networkSettingInfo;
    bool m_deviceEnable = true;
    DHCPTYpe m_dhcpType = DHCPTYpe::Auto;
};

}
