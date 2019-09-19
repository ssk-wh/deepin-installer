#pragma once

#include "partman/device.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QMap>

namespace installer {
class FullDiskPartitionColorBar: public QWidget {
    Q_OBJECT

public:
    using QWidget::QWidget;
    void setDevice(const Device::Ptr device);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    Device::Ptr m_device;
};

class FullDiskPartitionWidget: public QWidget{
    Q_OBJECT

public:
    FullDiskPartitionWidget(QWidget* parent=nullptr);
    void setDevice(const Device::Ptr device);
    void setDevices(const DeviceList& devices);

private:
    FullDiskPartitionColorBar *m_fullDiskPartitionColorBar;

    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_labelLayout;
    QMap<QHBoxLayout*, QList<QWidget*>> m_labels;
};
}
