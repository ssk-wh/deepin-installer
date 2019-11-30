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

#ifndef INSTALLER_UI_FRAMES_SYSTEM_INFO_FRAME_H
#define INSTALLER_UI_FRAMES_SYSTEM_INFO_FRAME_H
#include "ui/interfaces/frameinterface.h"
#include <QFrame>
class QHBoxLayout;
class QPushButton;
class QStackedLayout;

namespace installer {

class SystemInfoAvatarFrame;
class SystemInfoFormFrame;
class SystemInfoKeyboardFrame;

// Provides a form to let use input username, password and hostname,
// select timezone and avatar.
class SystemInfoFrame : public FrameInterface {
  Q_OBJECT

 public:
  explicit SystemInfoFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);

  // read the configuration file
  void init() override;
  // writes to the configuration file
  void finished() override;
  // read the configuration file to verify that the current page is available
  bool shouldDisplay() const override;

 private:
  void initConnections();
  void initUI();

  // Update visibility of buttons in header bar based on current page.
  void updateHeadBar();

  QPushButton* keyboard_button_ = nullptr;
  QHBoxLayout* bottom_layout_ = nullptr;
  QStackedLayout* stacked_layout_ = nullptr;
  SystemInfoAvatarFrame* avatar_frame_ = nullptr;
  SystemInfoFormFrame* form_frame_ = nullptr;
  SystemInfoKeyboardFrame* keyboard_frame_ = nullptr;

  // To mark current page before switching to timezone page.
  int last_page_;

  // Do not show keyboard frame if this flag is true.
  bool disable_keyboard_;

 private slots:
  // Restore last page when timezone page is finished.
  void restoreLastPage();

  void showAvatarPage();
  void showFormPage();
  void showKeyboardPage();

  // Update text in keyboard button.
  void updateLayout(const QString& layout);
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_SYSTEM_INFO_FRAME_H
