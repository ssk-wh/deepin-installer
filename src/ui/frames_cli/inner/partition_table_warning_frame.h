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

#ifndef PARTITION_TABLE_WARNING_FRAME_CLI_H
#define PARTITION_TABLE_WARNING_FRAME_CLI_H

#include <QScopedPointer>

#include "ui/interfaces_cli/frameinterfaceprivate.h"
#include "ui/interfaces_cli/frameinterface.h"

namespace installer {

class NcursesLabel;
class OperatorWidget;
class PartitionTableWarningFrame;

class PartitionTableWarningFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    PartitionTableWarningFramePrivate(FrameInterface* parent, int lines, int cols, int beginY, int beginX);

protected:
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void keyEventTriger(int key) override;

private:
    NcursesLabel* m_titleLab;
    NcursesLabel* m_commentLab;
    OperatorWidget* m_warning1;
    OperatorWidget* m_warning2;
    OperatorWidget* m_warning3;

    PartitionTableWarningFrame *q_ptr;
    Q_DECLARE_PUBLIC(PartitionTableWarningFrame)
};

class PartitionTableWarningFrame  : public FrameInterface
{
    Q_OBJECT
public:
    PartitionTableWarningFrame(FrameInterface* parent);

signals:
    void reboot();
    void cancel();
    void formatting();

protected:
    bool handle() override;
};

}

#endif // PARTITION_TABLE_WARNING_FRAME_H
