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

#ifndef INSTALLER_UI_WIDGETS_TOOLTIP_CONTAINER_H
#define INSTALLER_UI_WIDGETS_TOOLTIP_CONTAINER_H

#include <QFrame>
class QKeyEvent;
class QListView;
class QPaintEvent;
class QStringListModel;

namespace installer {

// Used to display popup menu with sharp corner at middle of bottom edge.
class PopupMenu : public QFrame {
  Q_OBJECT

 public:
  explicit PopupMenu(QWidget* parent = nullptr);

  // Returns the list used by menu_model to store menu items.
  QStringList stringList() const;

 signals:
  // Emitted when window is hidden.
  void onHide();

  // Emitted when a menu item at |index| is activated.
  void menuActivated(int index);

 public slots:
  // Show tooltip container at |pos| and grab keyboard focus.
  void popup(const QPoint& pos);

  // Set menu models's internal list to |strings|.
  void setStringList(const QStringList& strings);

 protected:
  // Release keyboard focus when window is hidden.
  void hideEvent(QHideEvent* event) override;

  // Hide window when Escape button is pressed.
  void keyPressEvent(QKeyEvent* event) override;

  void paintEvent(QPaintEvent* event) override;

  // Monitors global mouse event when menu is popup.
  void showEvent(QShowEvent* event) override;

 private:
  void initConnections();
  void initUI();

  QListView* menu_view_ = nullptr;
  QStringListModel* menu_model_ = nullptr;

 private slots:
  void onMenuViewActivated(const QModelIndex& index);
};

}  // namespace installer

#endif  // INSTALLER_UI_WIDGETS_TOOLTIP_CONTAINER_H
