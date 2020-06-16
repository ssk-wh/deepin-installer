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

#ifndef INSTALLER_UI_FRAMES_INNER_SYSTEM_INFO_FORM_FRAME_H
#define INSTALLER_UI_FRAMES_INNER_SYSTEM_INFO_FORM_FRAME_H

#include <QFrame>
#include <QScopedPointer>

namespace installer {

class AvatarButton;
class SystemInfoTip;
class TitleLabel;
class SystemInfoFormFramePrivate;
class SystemInfoFormFrame : public QFrame {
    Q_OBJECT

public:
    explicit SystemInfoFormFrame(QWidget* parent = nullptr);
    ~SystemInfoFormFrame() override;

    // Validate form content.
    bool validateUserInfo();

    void checkNextButtonEnable();

signals:
    // Emitted when the avatar button is clicked.
    void avatarClicked();

    // Emitted when user input finished.
    void systemInfoFormDone();

    void requestNextButtonEnable(bool enable);

public slots:
    // Update user avatar image.
    void updateAvatar(const QString& avatar);

    void readConf();

    // Write form content to conf file.
    void writeConf();

protected:
    void changeEvent(QEvent* event) override;

    // Set username_edit_ as the default focused widget.
    void showEvent(QShowEvent* event) override;

private:
    QScopedPointer<SystemInfoFormFramePrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, SystemInfoFormFrame)

};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_INNER_SYSTEM_INFO_FORM_FRAME_H
