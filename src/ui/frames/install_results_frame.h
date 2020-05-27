/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     justforlxz <zhangdingyuan@deepin.com>
 *
 * Maintainer: justforlxz <zhangdingyuan@deepin.com>
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


#ifndef INSTALL_RESULTS_FRAME_H
#define INSTALL_RESULTS_FRAME_H

#include "ui/interfaces/frameinterface.h"
#include "ui/interfaces/frameinterfaceprivate.h"

#include <QWidget>
#include <QStackedLayout>
#include <QScopedPointer>

namespace installer {
class InstallFailedFrame;
class InstallSuccessFrame;
class InstallResultsFramePrivate;

class InstallResultsFrame : public FrameInterface
{
    Q_OBJECT
public:
    InstallResultsFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
    ~InstallResultsFrame() override;

    void init() override;
    void finished() override;
    bool shouldDisplay() const override;
    QString returnFrameName() const override;

private:
    QScopedPointer<InstallResultsFramePrivate> m_private;
};

}  // namespace installer

#endif // INSTALL_RESULTS_FRAME_H
