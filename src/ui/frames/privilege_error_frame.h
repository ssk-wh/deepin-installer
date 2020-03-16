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

#ifndef INSTALLER_UI_FRAMES_PRIVILEGE_ERROR_FRAME_H
#define INSTALLER_UI_FRAMES_PRIVILEGE_ERROR_FRAME_H

#include "ui/interfaces/frameinterface.h"

#include <QFrame>

namespace installer {

class NavButton;
class PrivilegeErrorFramePrivate;

// Display this page when root privilege is not guaranteed.
class PrivilegeErrorFrame : public FrameInterface {
  Q_OBJECT

  friend PrivilegeErrorFramePrivate;

protected:
  void paintEvent(QPaintEvent *event) override;

 public:
  explicit PrivilegeErrorFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
  ~PrivilegeErrorFrame() override;

  virtual void init() override;
  virtual void finished() override;
  virtual bool shouldDisplay() const override;

private:
    QScopedPointer<PrivilegeErrorFramePrivate> m_private;
};

}  // namespace installer

#endif  // INSTALLER_UI_FRAMES_PRIVILEGE_ERROR_FRAME_H
