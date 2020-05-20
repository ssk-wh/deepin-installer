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

#ifndef OPERATOR_WIDGET_H
#define OPERATOR_WIDGET_H

#include "ui/ncurses_widgets/ncurses_windows_base.h"

namespace installer {

class NcursesLabel;

class OperatorWidget : public NCursesWindowBase
{
public:
    OperatorWidget(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, bool shadow = false, bool box = false);

    void setTitle(const QString &text);
    void setDesc(const QString &text);
    void move(int y, int x);

protected:
    void drawFoucs() override;

private:
    NcursesLabel* m_titleLab = nullptr;
    NcursesLabel* m_descLab = nullptr;
};

}

#endif // OPERATOR_WIDGET_H
