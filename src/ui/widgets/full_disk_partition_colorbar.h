#pragma once

#include "partman/device.h"
#include <QWidget>

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
    Device::Ptr device;
};
}

