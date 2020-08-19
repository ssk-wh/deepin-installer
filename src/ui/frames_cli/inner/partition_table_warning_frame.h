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

#ifndef PARTITION_TABLE_WARNING_FRAME_CLI_H
#define PARTITION_TABLE_WARNING_FRAME_CLI_H

#include <QFrame>
class QLabel;
class QPushButton;

#include "partman/partition.h"
#include "ui/interfaces_cli/frameinterface.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_line_edit.h"

namespace installer {

class CommentLabel;
class MountPointModel;
class AdvancedPartitionDelegate;
class PartitionModel;

class PartitionTableWarningFrame : public FrameInterfacePrivate {
  Q_OBJECT

 public:
  PartitionTableWarningFrame(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, PartitionModel* model);
  void setDevicePath(const QString& device_path);

  void show() override;
  void hide() override;
  void updateTs();
  void layout();
  void onKeyPress(int keycode) override;

  void rebootSystem();

 signals:
  void finished();
  void keyEventTrigerSignal(int keycode);
  void doBackBtnClickedSignal();
  void doNectBtnClickedSignal();
public:
    void keyPresseEvent(int keycode);
    NcursesListView* m_warningBox = nullptr;

protected:
    void backHandle() override;

 private:
  void initConnections();
  void initUI();


  //NcursesLabel* title_label_ = nullptr;
  NcursesLabel* m_commentLab = nullptr;


  NcursesButton* cancel_button_ = nullptr;
  NcursesButton* create_button_ = nullptr;


 private slots:
  // Append operations to |delegate| when create_button_ is clicked.
  void onCreateButtonClicked();

 protected:
  QString m_devicePath;   
  bool m_isshow = false;
  int m_currentchoicetype = -1;
  PartitionModel* m_partitionModel;
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_INNER_NEW_PARTITION_FRAME_H
