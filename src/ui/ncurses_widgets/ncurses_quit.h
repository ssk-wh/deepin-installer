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

#ifndef NCURSES_QUIT_H
#define NCURSES_QUIT_H

#include "ncurses_windows_base.h"

namespace installer {

class NcursesButton;
class NcursesLabel;

class NcursesQuit : public NCursesWindowBase
{
    Q_OBJECT
public:
    NcursesQuit(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, bool shadow = false, bool box = false);

    void show() override;
    void hide() override;
    void keyHandle(int key);

signals:
    void quit();
    void cancel();

private:
    void keyHandle();

private:
    NcursesLabel *m_title_lab = nullptr;
    NcursesLabel *m_desc_lab = nullptr;
    NcursesButton *m_quit_bnt = nullptr;
    NcursesButton *m_cancel_bnt = nullptr;
};

}

#endif // NCURSES_QUIT_H
