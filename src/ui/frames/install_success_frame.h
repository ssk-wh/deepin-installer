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

#ifndef INSTALLER_UI_FRAMES_INSTALL_SUCCESS_FRAME_H
#define INSTALLER_UI_FRAMES_INSTALL_SUCCESS_FRAME_H

#include "ui/interfaces/frameinterface.h"

#include <QFrame>
#include <QScopedPointer>

namespace installer {

class CommentLabel;
class NavButton;
class TitleLabel;

class InstallSuccessFramePrivate;
class InstallSuccessFrame : public FrameInterface {
    Q_OBJECT

public:
    explicit InstallSuccessFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
    ~InstallSuccessFrame() override;

    // Read configuration file
    void init() override;

    // Write to the configuration file
    void finished() override;

    // Read the configuration file to confirm that the current page is available
    bool shouldDisplay() const override;

public slots:
    void setEjectLabelVisible(bool visible);

protected:
    void changeEvent(QEvent* event) override;

private:
    QScopedPointer<InstallSuccessFramePrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, InstallSuccessFrame)
};

}

#endif  // INSTALLER_UI_FRAMES_INSTALL_SUCCESS_FRAME_H
