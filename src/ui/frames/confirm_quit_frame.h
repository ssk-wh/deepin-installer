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

#include <DSuggestButton>
#include <DDialog>
#include <DImageButton>

DWIDGET_USE_NAMESPACE

namespace installer {

class CommentLabel;
class SelectButton;

// This frame is displayed when close-button is clicked.
class ConfirmQuitFrame : public DDialog {
  Q_OBJECT

 public:
  explicit ConfirmQuitFrame(QWidget* parent = nullptr);
  void display();
  void updateTsForSuccessPage();

 signals:
  // Emitted when cancel-button is clicked.
  void quitCancelled();

  // Emitted when users confirms to quit this program.
  void quitConfirmed();

 protected:
  void changeEvent(QEvent* event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  bool eventFilter(QObject *watched, QEvent *event) override;
  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;

 private:
  void initConnections();
  void initUI();
  void setupCloseButton();

  CommentLabel *comment_label_ = nullptr;
  SelectButton* continue_button_ = nullptr;
  DSuggestButton* abort_button_ = nullptr;

  DImageButton *m_close_button = nullptr;
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_CONFIRM_QUIT_FRAME_H
