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

#ifndef INSTALLER_UI_FRAMES_INNER_PREPARE_INSTALL_FRAME_CLI_H
#define INSTALLER_UI_FRAMES_INNER_PREPARE_INSTALL_FRAME_CLI_H

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
class PartitionTypeModel;

class PrepareInstallFrame : public FrameInterfacePrivate {
  Q_OBJECT

 public:
  PrepareInstallFrame(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, AdvancedPartitionDelegate* delegate);

  // Reset partition information at |partition_path|.
  void show() override;
  void hide() override;
  void updateTs();
  void layout();
  void onKeyPress(int keycode) override;
 signals:
  void finished(bool isOK);
  void keyEventTrigerSignal(int keycode);
  void doBackBtnClickedSignal();
  void doNectBtnClickedSignal();
public:
    void keyPresseEvent(int keycode);
 private:
  void initConnections();
  void initUI();


  NcursesLabel* m_titleLabel_ = nullptr;
  NcursesLabel* m_commentLabel = nullptr;
  NcursesListView* operations_box_ = nullptr;

  NcursesButton* cancel_button_ = nullptr;
  NcursesButton* create_button_ = nullptr;

  AdvancedPartitionDelegate* delegate_ = nullptr;  

  PartitionTypeModel* type_model_ = nullptr;
  Partition::Ptr partition_;


 private slots:
  // Append operations to |delegate| when create_button_ is clicked.
  void onCreateButtonClicked();
  void onBackButtonClicked();

 protected: 
  bool m_isshow = false;
  int m_currentchoicetype = -1;
  QVector<NCursesWindowBase* > m_showChild;
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_INNER_NEW_PARTITION_FRAME_H
