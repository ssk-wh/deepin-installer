#include "full_disk_partition_colorbar.h"

#include <QPainter>
#include <QPainterPath>

namespace installer {

static const QMap<QString, QString> PART_NAME_COLOR_NAME_MAP{
    { QString("/boot"), QString("#C100AB") },
    { QString("/boot/efi"), QString("#F6C000") },
    { QString("swap"), QString("#0587F0") },
    { QString("/"), QString("#7F23FF") },
    { QString("/home"), QString("#00A951") },
    { QString(""), QString("#F69315") },
};

void FullDiskPartitionColorBar::setDevice(const Device::Ptr device)
{
    this->device = device;
    update();
}

void FullDiskPartitionColorBar::paintEvent(QPaintEvent *)
{
    if(device.isNull())
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::HighQualityAntialiasing);
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    path.addRoundedRect(rect(), 5, 5);
    painter.setClipPath(path);

    const PartitionList& partitions = device->partitions;
    int shift = 0;

    painter.fillRect(rect(), Qt::white);

    for (Partition::Ptr partition : partitions) {
        const float ratio = static_cast<float>(partition->length) / static_cast<float>(device->length);
        const int w = qRound(ratio * width());
        painter.fillRect(QRect(shift, 0, w, this->height())
            , QColor(PART_NAME_COLOR_NAME_MAP[partition->mount_point]));
        shift += w;
    }
}

QSize FullDiskPartitionColorBar::sizeHint() const
{
    return QSize(800, 20);
}
}
