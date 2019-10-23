#include "full_disk_partition_colorbar.h"
#include "base/file_util.h"
#include "ui/delegates/partition_util.h"

#include <QPainter>
#include <QPainterPath>
#include <QLabel>

namespace installer {

const int kWindowWidth = 960;
const int kPartitionLabelSpace = 15;

static const QMap<QString, QString> PART_NAME_COLOR_NAME_MAP{
    { QString("/boot"), QString("#C100AB") },
    { QString("/boot/efi"), QString("#F6C000") },
    { QString("/"), QString("#FB7A1F") },
    { QString(":rootb"), QString("#0587F0") },
    { QString("/data"), QString("#7F23FF") },
    { QString("swap"), QString("#2330C1") },
    { QString(":backup"), QString("#00A951") },

    { QString("/home"), QString("#00A951") },
    { QString(""), QString("#00FF00") },
    { QString(":"), QString("#00FF00") },
};

void FullDiskPartitionColorBar::setDevice(const Device::Ptr device)
{
    m_device = device;
    update();
}

static const QString GetPartitionDisplayText(const Partition::Ptr& partition) {
    return partition->mount_point.length() > 0
              ? partition->mount_point
              : partition->label.toLower();
}

static QColor GetPartitionColor(const Partition::Ptr& partition) {
    QColor color;
    if (partition->mount_point.length() > 0 &&
        PART_NAME_COLOR_NAME_MAP.find(partition->mount_point) != PART_NAME_COLOR_NAME_MAP.end()) {
        color = PART_NAME_COLOR_NAME_MAP[partition->mount_point];
    }
    else {
        color = PART_NAME_COLOR_NAME_MAP[QString(":%1").arg(partition->label.toLower())];
    }
    return color;
}

void FullDiskPartitionColorBar::paintEvent(QPaintEvent *)
{
    if(m_device.isNull())
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::HighQualityAntialiasing);
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    path.addRoundedRect(rect(), 5, 5);
    painter.setClipPath(path);

    const PartitionList& partitions = m_device->partitions;
    int shift = 0;

    painter.fillRect(rect(), Qt::white);

    int i = 0;
    int full_width = rect().width();
    for (Partition::Ptr partition : partitions) {
        const float ratio = static_cast<float>(partition->length) / static_cast<float>(m_device->length);
        int w = qRound(ratio * width());
        if (i == partitions.length() - 1 && shift + w < full_width) {
            w = full_width - shift;
        }
        painter.fillRect(QRect(shift, 0, w, this->height()), GetPartitionColor(partition));
        shift += w;
        i++;
    }
}

QSize FullDiskPartitionColorBar::sizeHint() const
{
    return QSize(kWindowWidth, 20);
}

FullDiskPartitionWidget::FullDiskPartitionWidget(QWidget* parent)
    :QWidget(parent)
{
    m_fullDiskPartitionColorBar = new FullDiskPartitionColorBar;

    m_labelLayout = new QHBoxLayout;
    m_labelLayout->setSpacing(kPartitionLabelSpace);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->addWidget(m_fullDiskPartitionColorBar, 0, Qt::AlignHCenter);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* bottomLayout = new QHBoxLayout;
    bottomLayout->setMargin(0);
    bottomLayout->setSpacing(0);

    bottomLayout->addStretch();
    bottomLayout->addLayout(m_labelLayout);
    bottomLayout->addStretch();

    m_mainLayout->addLayout(bottomLayout);

    setFixedWidth(kWindowWidth);
    setLayout(m_mainLayout);
    setStyleSheet(ReadFile(":/styles/full_disk_partition_colorbar.css"));
}

void FullDiskPartitionWidget::setDevices(const DeviceList& devices)
{
    const qint64 kFakeSectorSize = 512;
    Device::Ptr new_device(new Device());
    new_device->partitions.clear();

    for (const Device::Ptr& device : devices) {
        for (const Partition::Ptr& partition : device->partitions) {
                if (partition->end_sector < 0
                 || partition->start_sector < 0
                 || partition->type == PartitionType::Extended) {
                    continue;
                }

                Partition::Ptr new_partition(new Partition(*partition));
                new_partition->type = PartitionType::Unallocated;
                new_partition->start_sector *= partition->sector_size / kFakeSectorSize;
                new_partition->end_sector *= partition->sector_size / kFakeSectorSize;
                new_partition->sector_size = kFakeSectorSize;
                new_partition->length = (new_partition->end_sector - new_partition->start_sector + 1) * new_partition->sector_size;
                new_device->partitions << new_partition;
                new_device->length += new_partition->length;
        }
    }
    new_device->sector_size = kFakeSectorSize;
    new_device->sectors = new_device->getByteLength() / kFakeSectorSize;
    setDevice(new_device);
}

void FullDiskPartitionWidget::setDevice(const Device::Ptr device)
{
    m_fullDiskPartitionColorBar->setDevice(device);

    const PartitionList& partitions = device->partitions;

    for (auto it = m_labels.begin(); it != m_labels.end(); ++it) {
        for (QWidget* w : it.value()) {
            it.key()->removeWidget(w);
            w->deleteLater();
        }
        it.key()->deleteLater();
    }

    m_labels.clear();

    for (Partition::Ptr partition : partitions) {
        QHBoxLayout *layout = new QHBoxLayout;
        layout->setMargin(0);
        layout->setSpacing(3);

        QLabel *colorLable = new QLabel;
        colorLable->setFixedSize(QSize(10, 10));
        QPalette palette = colorLable->palette();
        palette.setColor(QPalette::Background, GetPartitionColor(partition));
        colorLable->setAutoFillBackground(true);
        colorLable->setPalette(palette);
        layout->addWidget(colorLable);

        QLabel *partNameLable = new QLabel();
        partNameLable->setText(GetPartitionDisplayText(partition));
        layout->addWidget(partNameLable);

        QLabel *partSize = new QLabel();
        QString tmp = GetPartitionUsage(partition);
        int index = tmp.lastIndexOf('/');
        partSize->setText(index < 0 ? tmp : tmp.mid(index + 1));
        layout->addWidget(partSize);

        QLabel *fileSysType = new QLabel();
        fileSysType->setText(GetFsTypeName(partition->fs));
        layout->addWidget(fileSysType);

        m_labels[layout] << colorLable << partNameLable << partSize << fileSysType;

        m_labelLayout->addLayout(layout);
    }

    setVisible(!device->partitions.isEmpty());
}

}
