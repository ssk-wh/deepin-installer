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

#ifndef TIP_LIST_VIEW_H
#define TIP_LIST_VIEW_H

#include <DListView>

DWIDGET_USE_NAMESPACE

namespace installer {

// 处理需要在QListView的item中显示tooltip的场景
class TipListView : public DListView {
    Q_OBJECT
public:
    TipListView(QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
};

}

#endif // TIP_LIST_VIEW_H
