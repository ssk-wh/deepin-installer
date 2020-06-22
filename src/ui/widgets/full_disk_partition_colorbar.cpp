#include "full_disk_partition_colorbar.h"
#include "base/file_util.h"
#include "ui/delegates/partition_util.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/partition_color_label.h"

#include <QPainter>
#include <QPainterPath>
#include <QLabel>

namespace installer {

const int kColorBarWidth = 543;
const int kPartitionLabelSpace = 10;

const int kPartitionNameFont = 14;
const int kPartitionMounLabFont = 12;

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
    return QSize(kColorBarWidth, 20);
}

FullDiskPartitionWidget::FullDiskPartitionWidget(QWidget* parent)
    :QWidget(parent)
{
    m_fullDiskPartitionColorBar = new FullDiskPartitionColorBar(this);
    m_fullDiskPartitionColorBar->setFixedHeight(20);

    m_labelLayout = new DFlowLayout(this);
    m_labelLayout->setFlow(DFlowLayout::Flow::LeftToRight);
    m_labelLayout->setContentsMargins(0, 0, 0, 0);
    //m_labelLayout->setHorizontalSpacing(kPartitionLabelSpace);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->addWidget(m_fullDiskPartitionColorBar, 0, Qt::AlignHCenter);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    QWidget *widget = new QWidget;
    widget->setFixedWidth(kColorBarWidth);
    widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    widget->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(m_labelLayout);

    m_mainLayout->addSpacing(2);
    m_mainLayout->addWidget(widget, 0, Qt::AlignHCenter);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setLayout(m_mainLayout);
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

    for (QWidget *w : m_labelLayoutWidgets) {
        ClearLayout(w->layout());
        w->layout()->deleteLater();
        m_labelLayout->removeWidget(w);
        w->deleteLater();
    }

    m_labelLayoutWidgets.clear();
    int widgetIndex = 0;

    QFont font;
    font.setPixelSize(kPartitionNameFont);

    QFont mountfont;
    mountfont.setPixelSize(kPartitionMounLabFont);

    int indexNum(0);

    for (Partition::Ptr partition : partitions) {
        QHBoxLayout *layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        PartitionColorLabel *colorLable = new PartitionColorLabel(GetPartitionColor(partition));
        layout->addWidget(colorLable);

        QString tooltipString;

        QLabel *partNameLable = new QLabel();
        partNameLable->setFont(font);
        partNameLable->setFixedWidth(75);
        partNameLable->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignBottom);
        partNameLable->setText(GetPartitionDisplayText(partition));
        partNameLable->setStyleSheet("QLabel{color:#363636} ");
        tooltipString.append(partNameLable->text());
        layout->addSpacing(2);
        layout->addWidget(partNameLable);

        QLabel *partSize = new QLabel();
        partSize->setFont(mountfont);
        partSize->setFixedWidth(42);
        partSize->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignBottom);
        QString tmp = GetPartitionUsage(partition);
        int index = tmp.lastIndexOf('/');
        partSize->setText(index < 0 ? tmp : tmp.mid(index + 1));
        tooltipString.append(" " + partSize->text());
        partSize->setStyleSheet("QLabel{color:#526A7F} ");
        layout->addWidget(partSize);

        QLabel *fileSysType = new QLabel();
        fileSysType->setFont(mountfont);
        fileSysType->setFixedWidth(42);
        fileSysType->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignBottom);

        fileSysType->setText(GetFsTypeName(partition->fs));
        tooltipString.append(" " + fileSysType->text());
        fileSysType->setStyleSheet("QLabel{color:#526A7F} ");        
        layout->addWidget(fileSysType);

        partNameLable->setToolTip(tooltipString);
        partSize->setToolTip(tooltipString);
        fileSysType->setToolTip(tooltipString);

        int dis = fileSysType->width() - fileSysType->text().size() * 7;
        if (dis< 0) {
            int num = fileSysType->width() / 7 - 3;
            if (num > 0) {
                fileSysType->setText(fileSysType->text().left(num+1) + "...");
            }
        }

        if (indexNum % 3 != 2 ) {
            layout->addSpacing(25);
        }

        indexNum++;

        QWidget *labelsWrapWidget = new QWidget;
        labelsWrapWidget->setFixedSize(180, 22);
        labelsWrapWidget->setLayout(layout);
        m_labelLayoutWidgets << labelsWrapWidget;
        m_labelLayout->insertWidget(widgetIndex, labelsWrapWidget);
        ++widgetIndex;
    }

    setVisible(!device->partitions.isEmpty());
}

}
