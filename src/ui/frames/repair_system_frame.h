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

#ifndef REPAIR_SYSTEM_FRAME_H
#define REPAIR_SYSTEM_FRAME_H

#include <QFrame>
#include <DPushButton>

DWIDGET_USE_NAMESPACE

namespace installer {

class OperatorWidget;
class NavButton;

class RepairSystemFrame : public QFrame
{
    Q_OBJECT
public:
    explicit RepairSystemFrame(QWidget *parent);

    bool shouldDisplay() const;

signals:
 // This signal is emitted when continue-button is clicked.
    void finished() const;
    void repair() const;

public slots:
    void repairSystem() const;

private:
    void initUi();
    void initConnection() const;
    bool isRepair() const;

private:
    OperatorWidget* m_installerWidget = nullptr;
    OperatorWidget* m_repairWidget = nullptr;
    NavButton* m_nextButton;
};

}

#endif // REPAIR_SYSTEM_FRAME_H
