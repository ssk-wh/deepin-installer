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

#ifndef INSTALLER_UI_WIDGETS_TIMEZONE_MAP_H
#define INSTALLER_UI_WIDGETS_TIMEZONE_MAP_H

#include <QFrame>
class QLabel;
class QListView;
class QResizeEvent;
class QStringListModel;

#include "sysinfo/timezone.h"

namespace installer {

class PopupMenu;
class TooltipPin;

// Draw timezone map and bubble.
class TimezoneMap : public QFrame {
  Q_OBJECT

 public:
  explicit TimezoneMap(QWidget* parent = nullptr);
  ~TimezoneMap();

  // Get current selected timezone, might be empty.
  const QString getTimezone() const;

 signals:
  void timezoneUpdated(const QString& timezone);

 public slots:
  // Remark |timezone| on map.
  void setTimezone(const QString& timezone);
  void setTimezoneData(const QString& timezone);

  void hideMark();
  void showMark();

 protected:
  // Hide tooltips when window is resized.
//  void resizeEvent(QResizeEvent* event) override;

  bool eventFilter(QObject* watched, QEvent* event) override;

 private:
  void initConnections();
  void initUI();

  // Popup zone window at |pos|.
  void popupZoneWindow(const QPoint& pos);

  // Mark current zone on the map.
  void remark();

  void updateMap();

  // Currently selected/marked timezone.
  ZoneInfo current_zone_;

  // A list of zone info found in system.
  const ZoneInfoList total_zones_;

  // A list of zone info which are near enough to current cursor position.
  ZoneInfoList nearest_zones_;

  // A round dot to indicate position on the map.
  QLabel* dot_ = nullptr;

  // To mark a zone on map.
  TooltipPin* zone_pin_ = nullptr;

  // To display a list of zones on map.
  PopupMenu* popup_window_ = nullptr;

  QLabel* map_label_;

  QSize m_mapLabelSize;
  QPoint m_popupPoint;

 private slots:
  void onPopupWindowActivated(int index);
};

}  // namespace installer

#endif  // INSTALLER_UI_WIDGETS_TIMEZONE_MAP_H
