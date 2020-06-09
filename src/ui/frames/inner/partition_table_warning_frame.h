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

#ifndef INSTALLER_UI_FRAMES_INNER_PARTITION_TABLE_WARNING_FRAME_H
#define INSTALLER_UI_FRAMES_INNER_PARTITION_TABLE_WARNING_FRAME_H

#include <QFrame>
#include <DButtonBox>

DWIDGET_USE_NAMESPACE

class QLabel;
class QPushButton;
class QButtonGroup;
class QAbstractButton;

namespace installer {

class CommentLabel;
class NavButton;
class TitleLabel;
class OperatorWidget;

// Displayed when partition table type and system boot method does not match.
class PartitionTableWarningFrame : public QFrame {
  Q_OBJECT

 public:
  explicit PartitionTableWarningFrame(QWidget* parent = nullptr);

  // Get/set device_path_.
  QString devicePath() const;
  void setDevicePath(const QString& device_path);

 signals:
  // Emitted when cancel button is clicked.
  void canceled();

  // Emitted when continue button is clicked.
  // |device_path| is the device disk path to be formatted.
  void confirmed(const QString& device_path);

  // Emitted when reboot button is clicked.
  void reboot();

 protected:
  void changeEvent(QEvent* event) override;

 private:
  void initConnections();
  void initUI();

  TitleLabel* title_label_ = nullptr;
  CommentLabel* comment_label_ = nullptr;

  DButtonBox* m_buttonBox = nullptr;
  QAbstractButton* m_currentButton = nullptr;
  OperatorWidget *m_warningWidget1 = nullptr;
  OperatorWidget *m_warningWidget2 = nullptr;
  OperatorWidget *m_warningWidget3 = nullptr;

  QPushButton* next_button_ = nullptr;

  QString device_path_;

 private slots:
  void onConfirmButtonClicked();
  void onButtonGroupToggled(QAbstractButton *button);
  void onNextButtonClicked();
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_INNER_PARTITION_TABLE_WARNING_FRAME_H
