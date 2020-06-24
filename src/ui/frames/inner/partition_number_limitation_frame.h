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

#ifndef INSTALLER_UI_FRAMES_INNER_PARTITION_NUMBER_LIMITATION_FRAME_H
#define INSTALLER_UI_FRAMES_INNER_PARTITION_NUMBER_LIMITATION_FRAME_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>

namespace installer {

class CommentLabel;
class TitleLabel;

// Display this info page when reaching max number of primary partitions
// and no more partitions can be created anymore.
class PartitionNumberLimitationFrame : public QFrame {
  Q_OBJECT

 public:
  explicit PartitionNumberLimitationFrame(QWidget* parent = nullptr);

 signals:
  // Emitted when back button is clicked.
  void finished();

 protected:
  void showEvent(QShowEvent* event) override;

 private:
  void initConnections();
  void initUI();

  QLabel* title_label_ = nullptr;
  CommentLabel* comment1_label_ = nullptr;
  CommentLabel* comment2_label_ = nullptr;
  QPushButton* back_button_ = nullptr;
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_INNER_PARTITION_NUMBER_LIMITATION_FRAME_H
