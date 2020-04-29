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

#ifndef INSTALLER_UI_FRAMES_INSTALL_PROGRESS_FRAME_H
#define INSTALLER_UI_FRAMES_INSTALL_PROGRESS_FRAME_H

#include "ui/interfaces/frameinterface.h"

#include <QFrame>
#include <QScopedPointer>

namespace installer {

class InstallProgressFramePrivate;

// Displays when system is being installed to disk.
// A progress bar is shown at bottom of page.
class InstallProgressFrame : public FrameInterface {
    Q_OBJECT
    Q_PROPERTY(int progress READ progress WRITE setProgress)

public:
    explicit InstallProgressFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
    ~InstallProgressFrame() override;

    int progress() const { return progress_; }
    void setProgress(int progress);

    // Returns true is installation process failed.
    bool failed() const;

    // Show slide now.
    void startSlide();

    void init() override;
    void finished() override;
    bool shouldDisplay() const override;
    QString returnFrameName() const override;
    // 控制是否允许上一步
    bool allowPrevious() const override;

public slots:
    // Run hooks when partition job is done
    void runHooks(bool ok);

    // Update progress value with a QTimer object.
    void simulate();

protected:
    void changeEvent(QEvent* event) override;

private:
    // Progress value.
    int progress_;

    QScopedPointer<InstallProgressFramePrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, InstallProgressFrame)
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_INSTALL_PROGRESS_FRAME_H
