#pragma once

#include "ui/delegates/network_operate.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QNetworkInterface>

namespace installer {

class NetworkDeviceWidget : public QPushButton
{
    Q_OBJECT

public:
    explicit NetworkDeviceWidget(QWidget* parent = nullptr);

    bool isSelected() const;

    void setTitle(const QString& title);
    void setDesc(const QString& desc);
    void setSelected(bool selected);

    void setDeviceInfo(NetworkManager::Device::Ptr device);
    NetworkManager::Device::Ptr getDevice() const;
    NetworkOperate* networkOperate() const;

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
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
