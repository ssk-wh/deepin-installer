#include "full_disk_partition_colorbar.h"
#include "base/file_util.h"
#include "ui/delegates/partition_util.h"

#include <QPainter>
#include <QPainterPath>
#include <QLabel>

namespace installer {

static const QMap<QString, QString> PART_NAME_COLOR_NAME_MAP{
    { QString("/boot"), QString("#C100AB") },
    { QString("/boot/efi"), QString("#F6C000") },
    { QString("swap"), QString("#0587F0") },
    { QString("/"), QString("#7F23FF") },
    { QString("/home"), QString("#00A951") },
    { QString(""), QString("#FB7A1F") },
};

void FullDiskPartitionColorBar::setDevice(const Device::Ptr device)
{
    m_device = device;
    update();
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

    for (Partition::Ptr partition : partitions) {
        const float ratio = static_cast<float>(partition->length) / static_cast<float>(m_device->length);
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

FullDiskPartitionWidget::FullDiskPartitionWidget(QWidget* parent)
    :QWidget(parent)
{
    m_fullDiskPartitionColorBar = new FullDiskPartitionColorBar;

    m_labelLayout = new QHBoxLayout;
    m_labelLayout->setSpacing(50);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->addWidget(m_fullDiskPartitionColorBar);

    QHBoxLayout* bottomLayout = new QHBoxLayout;
    bottomLayout->setMargin(0);
    bottomLayout->setSpacing(0);

    bottomLayout->addStretch();
    bottomLayout->addLayout(m_labelLayout);
    bottomLayout->addStretch();

    m_mainLayout->addLayout(bottomLayout);

    setLayout(m_mainLayout);
    setStyleSheet(ReadFile(":/styles/full_disk_partition_colorbar.css"));
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
        layout->setSpacing(5);

        QLabel *colorLable = new QLabel;
        colorLable->setFixedSize(QSize(10, 10));
        QPalette palette = colorLable->palette();
        palette.setColor(QPalette::Background, QColor(PART_NAME_COLOR_NAME_MAP[partition->mount_point]));
        colorLable->setAutoFillBackground(true);
        colorLable->setPalette(palette);
        layout->addWidget(colorLable);

        QLabel *partNameLable = new QLabel();
        partNameLable->setText(partition->mount_point);
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
}

}
