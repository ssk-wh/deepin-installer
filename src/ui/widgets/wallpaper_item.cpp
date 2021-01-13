/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#include "wallpaper_item.h"
#include "service/settings_manager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QResizeEvent>

namespace installer {

WallpaperItem::WallpaperItem(const QRect &rect, QWidget* parent) :
    QLabel(parent){

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    this->setObjectName("wallpaper_item");
    this->initUI();
}

void WallpaperItem::resizeEvent(QResizeEvent *event)
{
    update(event->size());
}

void WallpaperItem::initUI() {
    QRect rect = QApplication::desktop()->availableGeometry();
    move(rect.topLeft());
    update(rect.size());
}

void WallpaperItem::update(const QSize &size)
{
    const QString image_path = GetWindowBackground();
    const QPixmap orig_pixmap(image_path);
    const QPixmap pixmap = orig_pixmap.scaled(size,
                                              Qt::KeepAspectRatioByExpanding);
    this->setPixmap(pixmap);
}

}  // namespace installer
