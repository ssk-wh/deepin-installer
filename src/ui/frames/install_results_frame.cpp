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

#include "install_results_frame.h"
#include "ui/frames/install_success_frame.h"
#include "ui/frames/install_failed_frame.h"
#include "ui/interfaces/frameinterfaceprivate.h"
#include "ui/widgets/pointer_button.h"
#include "ui/frames/install_progress_frame.h"
#include "service/settings_manager.h"
#include <QButtonGroup>

namespace installer {

class InstallResultsFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    explicit InstallResultsFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate (parent)
        , q_ptr(qobject_cast<InstallResultsFrame* > (parent))
        , m_frame_layout(new QStackedLayout)
        , m_installSuccessFrame(new InstallSuccessFrame)
        , m_installFailedFrame(new InstallFailedFrame)
    {}

    void initUI();
    void showNextFrame();
    void showInstallSuccessFrame();
    void showInstallFailedFrame();


    QStackedLayout* m_frame_layout = nullptr;
    InstallResultsFrame* q_ptr = nullptr;
    InstallSuccessFrame* m_installSuccessFrame = nullptr;
    InstallFailedFrame* m_installFailedFrame = nullptr;
};

InstallResultsFrame::InstallResultsFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(FrameType::Frame, frameProxyInterface, parent)
    , m_private(new InstallResultsFramePrivate(this))
{
    m_private->initUI();
}

void InstallResultsFrame::init()
{
    const bool m_result = GetSettingsBool("DI_INSTALL_SUCCESSED");

    if (m_result) {
        m_private->showInstallSuccessFrame();
    } else {
        m_private->showInstallFailedFrame();
    }
}

void InstallResultsFrame::finished()
{

}

bool InstallResultsFrame::shouldDisplay() const
{
    return true;
}

InstallResultsFrame::~InstallResultsFrame()
{

}

void InstallResultsFramePrivate::initUI()
{
    m_frame_layout->setMargin(0);
    m_frame_layout->addWidget(m_installSuccessFrame);
    m_frame_layout->addWidget(m_installFailedFrame);

    centerLayout->addLayout(m_frame_layout);
}

void InstallResultsFramePrivate::showNextFrame()
{

}

void InstallResultsFramePrivate::showInstallSuccessFrame()
{
    m_frame_layout->setCurrentWidget(m_installSuccessFrame);
}

void InstallResultsFramePrivate::showInstallFailedFrame()
{
    m_frame_layout->setCurrentWidget(m_installFailedFrame);
}

}  // namespace installer

#include "install_results_frame.moc"
