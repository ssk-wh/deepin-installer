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

#ifndef INSTALLER_POINTER_BUTTON_H
#define INSTALLER_POINTER_BUTTON_H

#include <QPushButton>

namespace installer {

// Change cursor shape when mouse moves over button
class PointerButton : public QPushButton {
  Q_OBJECT

public:
    explicit PointerButton(QWidget* parent = nullptr);
    PointerButton(const QString& text, QWidget* parent = nullptr);

    void setNormalPic(const QString& normalPic);
    void setHoverPic(const QString& hoverPic);
    void setPressPic(const QString& pressPic);

protected:
    // Override these two event handlers to implements hover effect.
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void updatePic();

private:
    enum class ButtonStatus {
        Normal,
        Hover,
        Press
    };

    ButtonStatus m_state;
    QString m_normalPic;
    QString m_hoverPic;
    QString m_pressPic;
};

}  // namespace installer

#endif  // INSTALLER_POINTER_BUTTON_H
