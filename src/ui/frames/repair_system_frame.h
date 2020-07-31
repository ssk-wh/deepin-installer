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

#include "ui/interfaces/frameinterface.h"

#include <QScopedPointer>

namespace installer {
enum class LanguageType {
    Chinese = 0,
    English,
};

class RepairSystemFramePrivate;

class RepairSystemFrame : public FrameInterface
{
    Q_OBJECT

    friend class RepairSystemPrivate;

public:
    explicit RepairSystemFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
    ~RepairSystemFrame() override;

signals:
    void repair() const;

public:
    void init() override;
    void finished() override;
    bool shouldDisplay() const override;
    QString returnFrameName() const override;
    void repairSystem() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent* event) override;

private:
    bool isRepair() const;

private:
    QScopedPointer<RepairSystemFramePrivate> m_private;
    Q_DECLARE_PRIVATE_D(m_private, RepairSystemFrame)
};

}

Q_DECLARE_METATYPE(installer::LanguageType)

#endif // REPAIR_SYSTEM_FRAME_H
