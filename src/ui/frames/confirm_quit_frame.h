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

#ifndef INSTALLER_UI_FRAMES_CONFIRM_QUIT_FRAME_H
#define INSTALLER_UI_FRAMES_CONFIRM_QUIT_FRAME_H

#include <QPushButton>
#include <DDialog>

DWIDGET_USE_NAMESPACE

namespace installer {

class CommentLabel;

// This frame is displayed when close-button is clicked.
class ConfirmQuitFrame : public DDialog {
  Q_OBJECT

 public:
  explicit ConfirmQuitFrame(QWidget* parent = nullptr);
  void display();

 signals:
  // Emitted when cancel-button is clicked.
  void quitCancelled();

  // Emitted when users confirms to quit this program.
  void quitConfirmed();

 protected:
  void changeEvent(QEvent* event) override;

 private:
  void initConnections();
  void initUI();

  CommentLabel *comment_label_ = nullptr;
  QPushButton* continue_button_ = nullptr;
  QPushButton* abort_button_ = nullptr;
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_CONFIRM_QUIT_FRAME_H
