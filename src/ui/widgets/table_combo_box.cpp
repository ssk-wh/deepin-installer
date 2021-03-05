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

#include "ui/widgets/table_combo_box.h"
#include "base/file_util.h"
#include "ui/utils/widget_util.h"

#include <QAbstractItemView>

namespace installer {

TableComboBox::TableComboBox(QWidget* parent) : QComboBox(parent) {
    // Same as PartitionSizeSlider.
    setFixedSize(310, 36);
    view()->parentWidget()->setFixedWidth(310);
}

void TableComboBox::setHeight(int height)
{
    view()->parentWidget()->setFixedHeight(height);
}

void TableComboBox::mousePressEvent(QMouseEvent *e)
{
    emit signalMousePress();
    QComboBox::mousePressEvent(e);
}

}  // namespace installer
