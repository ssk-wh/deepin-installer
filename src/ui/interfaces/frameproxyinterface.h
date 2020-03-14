/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     justforlxz <justforlxz@outlook.com>
 *
 * Maintainer: justforlxz <justforlxz@outlook.com>
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

#ifndef FRAMEPROXYINTERFACE_H
#define FRAMEPROXYINTERFACE_H

#include <QWidget>

namespace installer {
class FrameInterface;
class ChildFrameInterface;
class FrameProxyInterface {
public:
    // 显示下一个Frame
    virtual void nextFrame() = 0;

    // 显示子页面
    virtual void showChildFrame(FrameInterface *frame) = 0;

    // 退出安装
    virtual void exitInstall(bool reboot = false) = 0;

    virtual void showChindFrame(ChildFrameInterface* childFrameInterface) {}

    virtual void hideChildFrame() const = 0;
};
}  // namespace installer

#endif  // FRAMEPROXYINTERFACE_H
