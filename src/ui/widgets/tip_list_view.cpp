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

#include "tip_list_view.h"

#include <QMouseEvent>
#include <QToolTip>
#include <QModelIndex>

installer::TipListView::TipListView(QWidget *parent):
    DListView(parent)
{
    setMouseTracking(true);         // QWidget类型的控件需要在构造函数调用setMouseTracking，才会响应mouseMoveEvent事件
    setItemSize(QSize(47, 47));     // 默认的item的大小，如果需要tip显示的位置正确，则需要设置item的width
}

void installer::TipListView::mouseMoveEvent(QMouseEvent *event)
{  
    QModelIndex index = this->indexAt(event->pos());
    // 计算tip显示的位置，统一转换成全局位置

    int posX = mapToGlobal(this->pos()).x() + (this->width() / 2);
    int posY = mapToGlobal(this->pos()).y() + (this->itemSize().width() * index.row()) + 6;
    QToolTip::showText(QPoint(posX, posY), index.data().toString()); // 显示tip

    return DListView::mouseMoveEvent(event);
}
