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

    friend InstallResultsFramePrivate;

public:
    InstallResultsFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent = nullptr);
    ~InstallResultsFrame() override;

    void init() override;
    void finished() override;
    bool shouldDisplay() const override;
    QString returnFrameName() const override;
    bool allowPrevious() const override;

    void showInstallFailedFrame();

protected:
    void showEvent(QShowEvent *event) override;
    bool doSelect() override;

signals:
    void successFinished();
    void failedFinished();
    void saveFailedLog();
    void closeButtionChange(bool change);
    void updateQuitFrameTs(bool result);

private:
    bool m_result = false;

    QScopedPointer<InstallResultsFramePrivate> m_private;
};

}  // namespace installer

#endif // INSTALL_RESULTS_FRAME_H
