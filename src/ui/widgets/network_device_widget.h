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

class NetworkDeviceWidget : public DButtonBoxButton
{
    Q_OBJECT

public:
    explicit NetworkDeviceWidget(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setDesc(const QString& desc);

    void updateCheckedAppearance();

    void setDeviceInfo(NetworkManager::Device::Ptr device);
    NetworkManager::Device::Ptr getDevice() const;
    NetworkOperate* networkOperate() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QHBoxLayout* m_hLayout;
    QVBoxLayout* m_vLayout;
    QLabel* m_deviceName;
    QLabel* m_descLabel;
    QLabel *m_checkedLabel;
    bool m_isflag;

    NetworkManager::Device::Ptr m_device = nullptr;
    NetworkOperate *m_networkOperate = nullptr;
};

}
