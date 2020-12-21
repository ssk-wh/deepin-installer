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

#ifndef INSTALLER_UI_FRAMES_TIMEZONE_FRAME_H
#define INSTALLER_UI_FRAMES_TIMEZONE_FRAME_H

#include "sysinfo/timezone.h"

#include "ui/interfaces/frameinterface.h"

#include <QScopedPointer>

class QStackedLayout;
class QHBoxLayout;
class QPushButton;
class QVBoxLayout;
class QButtonGroup;
class QAbstractButton;

namespace installer {

class CommentLabel;
class TimezoneManager;
class TimezoneMap;
class TitleLabel;
class SystemDateFrame;
class SelectTimeZoneFrame;
class PointerButton;
class TimezoneFramePrivate;

// Displays a world map to let user select timezone.
class TimezoneFrame : public FrameInterface {
  Q_OBJECT

  friend TimezoneFramePrivate;
 public:
  explicit TimezoneFrame(FrameProxyInterface* frameProxyInterface,QWidget* parent = nullptr);
  ~TimezoneFrame() override;

 signals:
  // Emit this signal to request to hide timezone page and timezone button.
  void hideTimezone();

  // Emitted when a new timezone is chosen.
  void timezoneUpdated(const QString& timezone);

  // Emitted when select timezone in list.
  void timezoneSet(const QString& timezone);

 public:
  void init() override;
  void finished() override;
  bool shouldDisplay() const override;
  QString returnFrameName() const override;

  // Update timezone when new language is selected.
  void updateTimezoneBasedOnLanguage(const QString& timezone);

 protected:
  void changeEvent(QEvent* event) override;

  // Remark current timezone when current frame is raised.
  void showEvent(QShowEvent* event) override;

  void hideEvent(QHideEvent* event) override;

  bool eventFilter(QObject *watched, QEvent *event) override;

  bool focusSwitch() override;
  bool doSpace() override;
  bool doSelect() override;
  bool directionKey(int keyvalue) override;

private:
  QScopedPointer<TimezoneFramePrivate> m_private;
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_TIMEZONE_FRAME_H
