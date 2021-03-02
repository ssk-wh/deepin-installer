
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

#ifndef INSTALLER_UI_FRAMES_INSTALL_FAILED_FRAME_H
#define INSTALLER_UI_FRAMES_INSTALL_FAILED_FRAME_H

#include <QFrame>
#include <QScopedPointer>

namespace installer {
class InstallFailedFramePrivate;
// Displays error message and a QR image
class InstallFailedFrame : public QFrame {
  Q_OBJECT

 public:
  explicit InstallFailedFrame(QWidget* parent = nullptr);
  ~InstallFailedFrame() override;

  void setButtonFocusPolicyUseTab(bool isuse);
  bool doSelect();

 signals:
  // Emitted when reboot button is clocked.
  void finished();

  void showSaveLogFrame();

 public slots:
  // Reload error message.
  void updateMessage();

 protected:
  void changeEvent(QEvent* event) override;

private:
    QScopedPointer<InstallFailedFramePrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, InstallFailedFrame)
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_INSTALL_FAILED_FRAME_H
