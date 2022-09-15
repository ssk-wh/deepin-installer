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

#include "service/settings_manager.h"
#include "service/screen_adaptation_manager.h"

#include <QGuiApplication>
#include <QDesktopWidget>

using namespace installer;

int main(int argc, char* argv[]) {
    /* 属性的设置一定要在app初始化之前，否则是无效的 */
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    app.setApplicationName("deepin-installer-init");

    // Delete old settings file and generate a new one.
    if (isLiveSystem()) {
        installer::AddConfigFile();
    }

    ScreenAdaptationManager::initDpiScale();

    return 0;
}
