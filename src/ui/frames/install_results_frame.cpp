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
#include "service/settings_name.h"
#include "ui/frames/saveinstallfailedlogframe.h"
#include "ui/main_window.h"

#include <QButtonGroup>

namespace installer {

class InstallResultsFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT

public:
    explicit InstallResultsFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate (parent)
        , m_frame_layout(new QStackedLayout)
        , q_ptr(qobject_cast<InstallResultsFrame* >(parent))
        , m_installSuccessFrame(new InstallSuccessFrame)
        , m_installFailedFrame(new InstallFailedFrame)
        , save_failedLog_frame_(new SaveInstallFailedLogFrame(q_ptr->m_proxy))
    {}

    void initUI();
    void initConnection();
    void showNextFrame();
    void showInstallSuccessFrame();
    void showInstallFailedFrame();


    QStackedLayout* m_frame_layout = nullptr;
    InstallResultsFrame* q_ptr = nullptr;
    InstallSuccessFrame* m_installSuccessFrame = nullptr;
    InstallFailedFrame* m_installFailedFrame = nullptr;
    SaveInstallFailedLogFrame* save_failedLog_frame_ = nullptr;
};

InstallResultsFrame::InstallResultsFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new InstallResultsFramePrivate(this))
{
    m_private->initUI();
    m_private->initConnection();
}

void InstallResultsFrame::init()
{
#ifdef QT_DEBUG
    const bool m_result = true;
#else
    const bool m_result = GetSettingsBool("DI_INSTALL_SUCCESSED");
#endif // QT_DEBUG

    if (m_result) {
        m_private->showInstallSuccessFrame();
    } else {
        m_private->m_installFailedFrame->updateMessage();
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

QString InstallResultsFrame::returnFrameName() const
{
    return ::QObject::tr("Done");
}

bool InstallResultsFrame::allowPrevious() const
{
    return false;
}

void InstallResultsFrame::showEvent(QShowEvent *event)
{
    Q_EMIT closeButtionChange(false);
    return FrameInterface::showEvent(event);
}

InstallResultsFrame::~InstallResultsFrame()
{

}

void InstallResultsFramePrivate::initUI()
{
    m_frame_layout->setContentsMargins(0, 0, 0, 0);
    m_frame_layout->setSpacing(0);
    m_frame_layout->addWidget(m_installSuccessFrame);
    m_frame_layout->addWidget(m_installFailedFrame);

    nextButton->hide();
    centerLayout->addLayout(m_frame_layout);

    save_failedLog_frame_->hide();
}

void InstallResultsFramePrivate::initConnection()
{
    connect(m_installSuccessFrame, &InstallSuccessFrame::finished, this, [=] {
        emit q_ptr->successFinished();
    });
    connect(m_installFailedFrame, &InstallFailedFrame::showSaveLogFrame, this, [=] {
        save_failedLog_frame_->startDeviceWatch(true);
        q_ptr->m_proxy->showChildFrame(save_failedLog_frame_);
    });
    connect(m_installFailedFrame, &InstallFailedFrame::finished, this, [=] {
        emit q_ptr->failedFinished();
    });
    connect(save_failedLog_frame_, &SaveInstallFailedLogFrame::requestBack, this, [=] {
        q_ptr->m_proxy->hideChildFrame();
    });
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
