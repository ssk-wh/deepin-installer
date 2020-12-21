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

#ifndef INSTALLER_UI_FRAMES_INNER_SYSTEM_INFO_KEYBOARD_FRAME_H
#define INSTALLER_UI_FRAMES_INNER_SYSTEM_INFO_KEYBOARD_FRAME_H

#include <QFrame>
#include <QScopedPointer>

#include "ui/interfaces/frameinterface.h"
#include "ui/interfaces/frameproxyinterface.h"

class QLineEdit;
class QLabel;

namespace installer {

class KeyboardLayoutModel;
class KeyboardLayoutVariantModel;
class NavButton;
class TitleLabel;

class SystemInfoKeyboardFramePrivate;
// Keyboard layout setup page.
class SystemInfoKeyboardFrame : public FrameInterface {
    Q_OBJECT

public:
    explicit SystemInfoKeyboardFrame(FrameProxyInterface* parent = nullptr);
    ~SystemInfoKeyboardFrame() override;

    void init() override;

    // 写入配置文件
    void finished() override;

    bool shouldDisplay() const override;

    QString returnFrameName() const override{
        return ::QObject::tr("Keyboard Layout");
    }

private:
    void layoutViewScrolle(int testindex);
    void variantViewScrolle(int testindex);

protected:
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;

    bool focusSwitch() override;
    bool doSpace() override;
    bool doSelect() override;
    bool directionKey(int keyvalue) override;

private:
    QScopedPointer<SystemInfoKeyboardFramePrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, SystemInfoKeyboardFrame)
};

}

#endif  // INSTALLER_UI_FRAMES_INNER_SYSTEM_INFO_KEYBOARD_FRAME_H
