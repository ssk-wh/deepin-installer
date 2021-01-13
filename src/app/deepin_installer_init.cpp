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

// Main program of installer.

#include "ui/widgets/wallpaper_item.h"

#include <QApplication>
#include <QDesktopWidget>

using namespace installer;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationDisplayName("Deepin Installer init");
    app.setApplicationName("deepin-installer");

    WallpaperItem *background = new WallpaperItem;
    background->setFixedSize(QApplication::desktop()->availableGeometry().size());
    background->show();
    background->lower();

    return app.exec();
}
