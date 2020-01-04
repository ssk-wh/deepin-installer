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

#include "ui/widgets/pointer_button.h"
#include "ui/utils/widget_util.h"

#include <QMouseEvent>
#include <QDebug>

namespace installer {

PointerButton::PointerButton(QWidget* parent)
    : QPushButton(parent)
    , m_state(ButtonStatus::Normal)
{
}

PointerButton::PointerButton(const QString& text, QWidget* parent)
    : PointerButton(parent)
{
    setText(text);
}

void PointerButton::setNormalPic(const QString& normalPic)
{
    m_normalPic = normalPic;
    updatePic();
}

void PointerButton::setHoverPic(const QString& hoverPic)
{
    m_hoverPic = hoverPic;
    updatePic();
}

void PointerButton::setPressPic(const QString& pressPic)
{
    m_pressPic = pressPic;
    updatePic();
}

void PointerButton::enterEvent(QEvent* event)
{
    setCursor(Qt::PointingHandCursor);

    m_state = ButtonStatus::Hover;
    updatePic();

    return QPushButton::enterEvent(event);
}

void PointerButton::leaveEvent(QEvent* event)
{
    unsetCursor();

    m_state = ButtonStatus::Normal;
    updatePic();

    return QPushButton::leaveEvent(event);
}

void PointerButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_state = ButtonStatus::Press;
        updatePic();
    }

    return QPushButton::mousePressEvent(event);
}

void PointerButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_state == ButtonStatus::Press && rect().contains(event->pos())) {
        m_state = ButtonStatus::Hover;
        updatePic();
    }

    return QPushButton::mouseReleaseEvent(event);
}

void PointerButton::updatePic()
{
    switch(m_state){
    case ButtonStatus::Normal:
        if (!m_normalPic.isEmpty()) {
            setIcon(QIcon(installer::renderPixmap(m_normalPic)));
        }
        break;
    case ButtonStatus::Hover:
        if (!m_hoverPic.isEmpty()) {
            setIcon(QIcon(installer::renderPixmap(m_hoverPic)));
        }
        break;
    case ButtonStatus::Press:
        if (!m_pressPic.isEmpty()) {
            setIcon(QIcon(installer::renderPixmap(m_pressPic)));
        }
        break;
    default:
        qCritical() << "invalid PointerButton state";
        break;
    }
}

}  // namespace installer
