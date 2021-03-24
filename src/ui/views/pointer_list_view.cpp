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

#include "ui/views/pointer_list_view.h"

#include <QMouseEvent>

#include "ui/utils/xutil.h"

namespace installer {

PointerListView::PointerListView(QWidget* parent)
    : QListView(parent)
    , m_isMousePressed(false)
{
  this->setObjectName("pointer_list_view");
  this->setMouseTracking(true);
}

void PointerListView::mouseMoveEvent(QMouseEvent* event) {
  QAbstractItemModel* model = this->model();
  if (model) {
    const QModelIndex index = this->indexAt(event->pos());
    // Change cursor shape to "Hand" when mouse enters list view item.
    if (index.isValid()) {
      setCursor(QCursor(Qt::PointingHandCursor));

    } else {
      setCursor(QCursor(Qt::ArrowCursor));
    }
  }

  if (!m_isMousePressed) {
      QListView::mouseMoveEvent(event);
  }
}

void PointerListView::mousePressEvent(QMouseEvent *event)
{
    m_isMousePressed = true;

    QListView::mousePressEvent(event);
}

void PointerListView::mouseReleaseEvent(QMouseEvent *event)
{
    m_isMousePressed = false;

    QListView::mouseReleaseEvent(event);
}

}  // namespace installer
