/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/delegates/disk_installation_detail_delegate.h"
#include "ui/utils/widget_util.h"
#include "ui/delegates/partition_util.h"

#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QApplication>
#include <QFileInfo>

Q_DECLARE_METATYPE(installer::Device::Ptr)

namespace installer {

namespace {

const int kItemSpace = 10;

const int kOsIconLeftMargin = 10;
// Left margin of text content.
const int kTextLeftMargin = 40;
const int kDiskSizeLeftMarin = 173;
const int kDiskPercentLeftMarin = 250;
const int kDiskPercentHeight = 6;
const int kSelectedLeftMargin = 366;
// Right margin of selected item, used to locate background image.
const int kSelectedRightMargin = 2;
// Size of bottom border.
const int kBorderBottom = 10;
const int kItemRightMargin = 10;
const QString kDriverIcon = ":/images/driver_128.svg";
const QString kDriverInstallIcon = ":/images/driver_install_128.svg";

}  // namespace

DiskInstallationDetailDelegate::DiskInstallationDetailDelegate(QObject* parent)
 :QStyledItemDelegate(parent)
{

}

void DiskInstallationDetailDelegate::setItemSize(QSize itemSize)
{
    m_itemSize = itemSize;
}

void DiskInstallationDetailDelegate::paint(QPainter* painter,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const {
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    QRect rect(option.rect.x() + kItemSpace, option.rect.y() + kItemSpace
               , option.rect.width() - 2 * kItemSpace, option.rect.height() - kItemSpace);

    QPainterPath path;
    path.addRoundedRect(rect, 8, 8);
    painter->setClipPath(path);

    if (option.state & QStyle::State_Selected) {
      // Draw background color of selected item, no matter it is active or not.
      painter->fillRect(rect, QBrush(getSelectedColor()));

      // Draw background image of selected item.
      const QPixmap pixmap = installer::renderPixmap(":/images/select_blue.svg");
      const qreal ratio = qApp->devicePixelRatio();
      const int x = rect.width() - static_cast<int>(pixmap.width() / ratio) - kSelectedRightMargin;
      const int y = rect.y() + static_cast<int>((rect.height() - pixmap.height() / ratio) / 2);
      const QRect pixmap_rect(x, y, static_cast<int>(pixmap.width() / ratio), static_cast<int>(pixmap.height() / ratio));
      painter->drawPixmap(pixmap_rect, pixmap);
    }  else if (option.state & QStyle::State_MouseOver) {
      // Draw background color when mouse is hover
      painter->fillRect(rect, QBrush(getHoveredColor()));
    }
    else {
      painter->fillRect(rect, QBrush(getNormalColor()));
    }

    Device::Ptr device (index.model()->data(index).value<Device::Ptr>());

    // Draw os icon
    QString os_icon_path = option.state & QStyle::State_Selected ? kDriverInstallIcon:kDriverIcon;
    const QPixmap os_icon = installer::renderPixmap(os_icon_path);
    const qreal ratio = qApp->devicePixelRatio();
    const int os_height = static_cast<int>(os_icon.height()/ ratio) > rect.height() ?
                rect.height():static_cast<int>(os_icon.height()/ ratio);
    const int os_width = static_cast<int>(os_icon.width()*os_height/os_icon.height());
    const int x = rect.x() + kOsIconLeftMargin;
    const int y = rect.y() + static_cast<int>((rect.height() - os_height) / 2);
    const QRect os_rect(x, y, os_width, os_height);
    painter->drawPixmap(os_rect, os_icon);

    // Draw text
    QString text = humanReadableDeviceName(device);
    const QColor text_color(Qt::black);
    painter->setPen(QPen(text_color));
    int text_x = std::max(rect.x() + kTextLeftMargin, os_rect.right() + kItemRightMargin);
    QRect text_rect(text_x, rect.y(),
             kDiskSizeLeftMarin - text_x, rect.height());
    text = painter->fontMetrics().elidedText(text,Qt::TextElideMode::ElideRight, text_rect.width());
    painter->drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);

    // Draw disk size
    DeviceSize device_size = humanReadableDeviceSize(device);
    text = humanReadableDeviceSizeString(device_size);
    text_rect = QRect(rect.x() + kDiskSizeLeftMarin, rect.y(),
           kDiskPercentLeftMarin - kItemRightMargin - (rect.x() + kDiskSizeLeftMarin),
           rect.height());
    text = painter->fontMetrics().elidedText(text,Qt::TextElideMode::ElideRight, text_rect.width());
    painter->drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);

    //Draw disk percent
    const QColor full_color(Qt::gray);
    const QRect full_rect(rect.x() + kDiskPercentLeftMarin,
          static_cast<int>(rect.y()+(rect.height() - kDiskPercentHeight)/2),
          kSelectedLeftMargin - kItemRightMargin - (rect.x() + kDiskPercentLeftMarin) - 40,
          kDiskPercentHeight);
    QPainterPath full_path;
    full_path.addRoundedRect(full_rect, full_rect.height()/2, full_rect.height()/2);
    painter->fillPath(full_path,full_color);

    qreal disk_percent = humanReadableDeviceSizePercent(device_size);
    if (disk_percent < 0) {
        disk_percent = 0.0;
    }
    const QColor percent_color(Qt::blue);
    const QRect percent_rect(rect.x() + kDiskPercentLeftMarin,
          static_cast<int>(rect.y()+(rect.height() - kDiskPercentHeight)/2),
          static_cast<int>((kSelectedLeftMargin - kItemRightMargin - (rect.x() + kDiskPercentLeftMarin))*disk_percent),
          kDiskPercentHeight);
    QPainterPath percent_path;
    percent_path.addRoundedRect(percent_rect, percent_rect.height()/2, percent_rect.height()/2);
    painter->fillPath(percent_path, percent_color);

    painter->restore();
}

QSize DiskInstallationDetailDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (m_itemSize.isValid()) {
        return m_itemSize;
    }

    return QStyledItemDelegate::sizeHint(option, index);
}

const QString DiskInstallationDetailDelegate::humanReadableDeviceName(const Device::Ptr & device)
{
    return QFileInfo(device->path).fileName();
}

const DiskInstallationDetailDelegate::DeviceSize DiskInstallationDetailDelegate::humanReadableDeviceSize(const Device::Ptr & device)
{
    DeviceSize size { 0, 0 };
    size.length = device->getByteLength();
    if (size.length < 0) {
        size.length = 0;
    }
    for ( Partition::Ptr partition : device->partitions ) {
        if (partition->type == PartitionType::Extended
                || partition->type == PartitionType::Unallocated) {
            continue;
        }
        if (partition->length > 0 && partition->freespace > 0
                && (partition->freespace < partition->length)) {
            size.used += partition->length - partition->freespace;
        }
    }
    return size;
}

const QString DiskInstallationDetailDelegate::humanReadableDeviceSizeString(const DiskInstallationDetailDelegate::DeviceSize  & size)
{
    if (size.length < kGibiByte) {
        return QString("%1/%2M")
              .arg(ToMebiByte(size.used))
              .arg(ToMebiByte(size.length));
    }
    else {
        return QString("%1/%2G")
              .arg(ToGigByte(size.used))
              .arg(ToGigByte(size.length));
    }
}

qreal DiskInstallationDetailDelegate::humanReadableDeviceSizePercent(const DiskInstallationDetailDelegate::DeviceSize& size)
{
    if (size.length == 0) {
        return 0.0;
    }
    return static_cast<qreal>(size.used)/static_cast<qreal>(size.length);
}

QColor DiskInstallationDetailDelegate::getNormalColor() const
{
    // TODO(chenxiong): use dtk normal color
    return QColor(245,245,245);
}

QColor DiskInstallationDetailDelegate::getHoveredColor() const
{
    // TODO(chenxiong): use dtk hovered color
    return QColor(206,206,206);
}

QColor DiskInstallationDetailDelegate::getSelectedColor() const
{
    // TODO(chenxiong): use dtk selected color
    return QColor(245,245,245);
}

}
