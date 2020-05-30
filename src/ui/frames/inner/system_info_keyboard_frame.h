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
class QLineEdit;
class QLabel;

namespace installer {

class KeyboardLayoutModel;
class KeyboardLayoutVariantModel;
class NavButton;
class TitleLabel;

class SystemInfoKeyboardFramePrivate;
// Keyboard layout setup page.
class SystemInfoKeyboardFrame : public QFrame {
    Q_OBJECT

public:
    explicit SystemInfoKeyboardFrame(QWidget* parent = nullptr);
    ~SystemInfoKeyboardFrame() override;

    // Set keyboard layout to default value.
    void readConf();

signals:
    // Emitted when back_button_ is clicked.
    void finished();

    // Emitted when new keyboard layout is selected.
    void layoutUpdated(const QString& layout);

public slots:
    // Save current keyboard layout to settings file.
    void writeConf();

protected:
    void changeEvent(QEvent* event) override;

private:
    QScopedPointer<SystemInfoKeyboardFramePrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, SystemInfoKeyboardFrame)
};

}

#endif  // INSTALLER_UI_FRAMES_INNER_SYSTEM_INFO_KEYBOARD_FRAME_H
