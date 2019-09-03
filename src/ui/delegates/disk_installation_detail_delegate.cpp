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
#include <QStyle>
#include <QApplication>
#include <QFileInfo>

Q_DECLARE_METATYPE(installer::Device::Ptr)

namespace installer {

namespace {

const int kOsIconLeftMargin = 25;
// Left margin of text content.
const int kTextLeftMargin = 80;
const int kDiskSizeLeftMarin = 300;
const int kDiskPercentLeftMarin = 500;
const int kDiskPercentHeight = 6;
const int kSelectedLeftMargin = 600;
// Right margin of selected item, used to locate background image.
const int kSelectedRightMargin = 20;
// Size of bottom border.
const int kBorderBottom = 1;
const int kItemRightMargin = 10;
const QString kDriverIcon = ":/images/driver_128.svg";
const QString kDriverInstallIcon = ":/images/driver_install_128.svg";

}  // namespace

DiskInstallationDetailDelegate::DiskInstallationDetailDelegate(QObject* parent)
 :QStyledItemDelegate(parent)
{

}

void DiskInstallationDetailDelegate::paint(QPainter* painter,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const {
    painter->save();

    const QRect& rect(option.rect);
    const QRect background_rect(rect.x(), rect.y(), rect.width(),
                                rect.height() - kBorderBottom);

    if (option.state & QStyle::State_Selected) {
      // Draw background image of selected item.
      const QPixmap pixmap = installer::renderPixmap(":/images/select.svg");
      const qreal ratio = qApp->devicePixelRatio();
      const int x = rect.width() - static_cast<int>(pixmap.width() / ratio) - kSelectedRightMargin;
      const int y = rect.y() + static_cast<int>((rect.height() - pixmap.height() / ratio) / 2);
      const QRect pixmap_rect(x, y, static_cast<int>(pixmap.width() / ratio), static_cast<int>(pixmap.height() / ratio));
      painter->drawPixmap(pixmap_rect, pixmap);

      // Draw background color of selected item, no matter it is active or not.
      const QColor selected_color(255, 255, 255, 51);
      const QBrush background_brush(selected_color);
      painter->fillRect(background_rect, background_brush);
    }  else if (option.state & QStyle::State_MouseOver) {
      // Draw background color when mouse is hover
      const QColor selected_color(255, 255, 255, 25);
      const QBrush background_brush(selected_color);
      painter->fillRect(background_rect, background_brush);
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
    const QColor text_color(255, 255, 255, 255);
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
    const QColor full_color(255,255,255, 25);
    const QRect full_rect(rect.x() + kDiskPercentLeftMarin,
          static_cast<int>(rect.y()+(rect.height() - kDiskPercentHeight)/2),
          kSelectedLeftMargin - kItemRightMargin - (rect.x() + kDiskPercentLeftMarin),
          kDiskPercentHeight);
    QPainterPath full_path;
    full_path.addRoundedRect(full_rect, full_rect.height()/2, full_rect.height()/2);
    painter->fillPath(full_path,full_color);

    qreal disk_percent = static_cast<qreal>(device_size.freespace)/static_cast<qreal>(device_size.length);
    const QColor percent_color(44, 167, 248, 196);
    const QRect percent_rect(rect.x() + kDiskPercentLeftMarin,
          static_cast<int>(rect.y()+(rect.height() - kDiskPercentHeight)/2),
          static_cast<int>((kSelectedLeftMargin - kItemRightMargin - (rect.x() + kDiskPercentLeftMarin))*disk_percent),
          kDiskPercentHeight);
    QPainterPath percent_path;
    percent_path.addRoundedRect(percent_rect, percent_rect.height()/2, percent_rect.height()/2);
    painter->fillPath(percent_path, percent_color);

    // Draw bottom border of item without last one.
    if (index.row() + 1 != index.model()->rowCount(index)) {
      const QColor border_color(0, 0, 0, 20);
      QPen border_pen(border_color);
      border_pen.setWidth(kBorderBottom);
      painter->setPen(border_pen);
      const QLine border_line(option.rect.bottomLeft(), option.rect.bottomRight());
      painter->drawLine(border_line);
    }

    painter->restore();
}
const QString DiskInstallationDetailDelegate::humanReadableDeviceName(const Device::Ptr & device)
{
    return QFileInfo(device->path).fileName();
}

const DiskInstallationDetailDelegate::DeviceSize DiskInstallationDetailDelegate::humanReadableDeviceSize(const Device::Ptr & device)
{
    qint64 length = device->getByteLength();
    qint64 free_length = 0;
    for ( Partition::Ptr partition : device->partitions ) {
        if (partition->length > 0 && partition->freespace > 0
                && (partition->freespace < partition->length)) {
            free_length += length;
        }
    }
    return DeviceSize { length, free_length};
}

const QString DiskInstallationDetailDelegate::humanReadableDeviceSizeString(const DiskInstallationDetailDelegate::DeviceSize  & size)
{
    if (size.length < kGibiByte) {
        return QString("%1/%2M")
              .arg(ToMebiByte(size.freespace))
              .arg(ToMebiByte(size.length));
    }
    else {
        return QString("%1/%2G")
              .arg(ToGigByte(size.freespace))
              .arg(ToGigByte(size.length));
    }
}

}
