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

#ifndef PARTITION_NUMBER_LIMITATION_FRAME_CLI_H
#define PARTITION_NUMBER_LIMITATION_FRAME_CLI_H

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

class PartitionNumberLimitationFrame : public FrameInterfacePrivate {
  Q_OBJECT

 public:
  PartitionNumberLimitationFrame(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, PartitionModel* model);
  void inits();

  void show() override;
  void hide() override;
  void updateTs() override;
  void layout() override;
  void onKeyPress(int keycode) override;

 signals:
  void finished();
  void keyEventTrigerSignal(int keycode);
  void doBackBtnClickedSignal();
  void doNectBtnClickedSignal();
public:
    void keyPresseEvent(int keycode);    

protected:
    void backHandle() override;

 private:
  void initConnections();
  void initUI() override;

  //NcursesLabel* title_label_ = nullptr;
  NcursesLabel* m_commentLab = nullptr;
  NcursesLabel* m_commentLab1 = nullptr;
  NcursesButton* cancel_button_ = nullptr;

 protected:
  bool m_isshow = false;
  int m_currentchoicetype = -1;
  PartitionModel* m_partitionModel;
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_INNER_NEW_PARTITION_FRAME_H
