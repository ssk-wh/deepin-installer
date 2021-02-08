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

#include "ui/frames/system_info_frame.h"

#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/frames/inner/system_info_avatar_frame.h"
#include "ui/frames/inner/system_info_form_frame.h"
#include "timezone_frame.h"
#include "ui/widgets/pointer_button.h"
#include "ui/interfaces/frameinterfaceprivate.h"

#include <QHBoxLayout>
#include <QStackedLayout>
#include <DFrame>
#include <DPalette>
#include <QEvent>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace installer {

class SystemInfoFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT

    friend SystemInfoFrame;
public:
    explicit SystemInfoFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate(parent)
        , q_ptr(qobject_cast<SystemInfoFrame* >(parent))
    {}
    SystemInfoFrame* q_ptr;

    void initConnections();
    void initUI();

    // Update visibility of buttons in header bar based on current page.
    void updateHeadBar();

    QHBoxLayout* bottom_layout_ = nullptr;
    QStackedLayout* stacked_layout_ = nullptr;
    SystemInfoFormFrame* form_frame_ = nullptr;

    // Update text in keyboard button.
    void updateLayout(const QString& layout);

    bool validate() const override;

    void updateNextButton() const override;
};

SystemInfoFrame::SystemInfoFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new SystemInfoFramePrivate(this))
{
  setObjectName("system_info_frame");

  m_private->initUI();
  m_private->initConnections();

  this->setCurentFocus(m_private->form_frame_);
}

SystemInfoFrame::~SystemInfoFrame()
{

}

void SystemInfoFrame::init() {
  // Read default avatar explicitly.
  m_private->form_frame_->readConf();
}

void SystemInfoFrame::finished() {
  // Notify sub-pages to save settings.
  m_private->form_frame_->writeConf();
}

bool SystemInfoFrame::shouldDisplay() const
{
    return !(GetSettingsBool(kSystemInfoSetupAfterReboot) || GetSettingsBool(kSkipSystemInfoPage) || GetSettingsBool(kPartitionDoAutoPart));
}

QString SystemInfoFrame::returnFrameName() const
{
    return ::QObject::tr("Create Accounts");
}

void SystemInfoFrame::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        m_private->nextButton->setText(::QObject::tr("Next"));
    }

    return FrameInterface::changeEvent(event);
}

bool SystemInfoFrame::focusSwitch()
{
    if (m_current_focus_widget == nullptr) {
        if (m_private->nextButton == m_current_focus_widget) {
            this->setCurentFocus(m_private->form_frame_);
        } else {
            this->setCurentFocus(m_private->nextButton);
        }
    } else if(m_private->nextButton == m_current_focus_widget) {
        this->setCurentFocus(m_private->form_frame_);
    } else if (m_private->form_frame_ == m_current_focus_widget) {
        if (!m_private->form_frame_->focusSwitch()) {
            if (m_private->nextButton->isEnabled()) {
                this->setCurentFocus(m_private->nextButton);
            } else {
                this->setCurentFocus(m_private->form_frame_);
            }
        }
    }
    return true;
}

bool SystemInfoFrame::doSpace()
{
    if (m_private->form_frame_ == m_current_focus_widget) {
        m_private->form_frame_->doSpace();
    }
    return true;
}

bool SystemInfoFrame::doSelect()
{
    if(m_private->nextButton == m_current_focus_widget) {
        emit m_private->nextButton->clicked();
    }
    return true;
}

bool SystemInfoFrame::directionKey(int keyvalue)
{
    if (m_private->form_frame_ == m_current_focus_widget) {
        m_private->form_frame_->directionKey(keyvalue);
    }

    return true;
}

void SystemInfoFramePrivate::initConnections() {
  connect(form_frame_, &SystemInfoFormFrame::systemInfoFormDone, this, [=] {
      emit nextButton->clicked();
  });
  connect(form_frame_, &SystemInfoFormFrame::requestNextButtonEnable, nextButton, &QPushButton::setEnabled);
}

void SystemInfoFramePrivate::initUI() {
  form_frame_ = new SystemInfoFormFrame();

  stacked_layout_ = new QStackedLayout();
  stacked_layout_->setContentsMargins(0, 0, 0, 0);
  stacked_layout_->setSpacing(0);
  stacked_layout_->addWidget(form_frame_);

  centerLayout->addLayout(stacked_layout_);

  q_ptr->setContentsMargins(0, 0, 0, 0);
}

void SystemInfoFramePrivate::updateLayout(const QString& layout) {
  nextButton->setText(::QObject::tr("Next"));
}

bool SystemInfoFramePrivate::validate() const
{
    return form_frame_->validateUserInfo();
}

void SystemInfoFramePrivate::updateNextButton() const
{
    form_frame_->checkNextButtonEnable();
}

}  // namespace installer

#include "system_info_frame.moc"
