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
#include "ui/frames/inner/system_info_keyboard_frame.h"
#include "timezone_frame.h"
#include "ui/widgets/pointer_button.h"
#include "ui/interfaces/frameinterfaceprivate.h"

#include <QHBoxLayout>
#include <QStackedLayout>
#include <DFrame>
#include <DPalette>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace installer {

namespace {

const int kInvalidPageId = -1;
const int kAvatarPageId = 0;
const int kFormPageId = 1;

}  // namespace

class SystemInfoFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    explicit SystemInfoFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate(parent)
        , q_ptr(qobject_cast<SystemInfoFrame* >(parent))
        , last_page_(kInvalidPageId)
        , disable_keyboard_(GetSettingsBool(kSystemInfoDisableKeyboardPage))
    {}
    SystemInfoFrame* q_ptr;

    void initConnections();
    void initUI();

    // Update visibility of buttons in header bar based on current page.
    void updateHeadBar();

    QPushButton* keyboard_button_ = nullptr;
    QHBoxLayout* bottom_layout_ = nullptr;
    QStackedLayout* stacked_layout_ = nullptr;
    SystemInfoAvatarFrame* avatar_frame_ = nullptr;
    SystemInfoFormFrame* form_frame_ = nullptr;
    SystemInfoKeyboardFrame* keyboard_frame_ = nullptr;

    // To mark current page before switching to timezone page.
    int last_page_;

    // Do not show keyboard frame if this flag is true.
    bool disable_keyboard_;

    // Restore last page when timezone page is finished.
    void restoreLastPage();

    void showAvatarPage();
    void showFormPage();
    void showKeyboardPage();

    // Update text in keyboard button.
    void updateLayout(const QString& layout);

    bool validate() const;
};

SystemInfoFrame::SystemInfoFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new SystemInfoFramePrivate(this))
{
  setObjectName("system_info_frame");

  m_private->initUI();
  m_private->initConnections();

  m_private->showFormPage();
}

SystemInfoFrame::~SystemInfoFrame()
{

}

void SystemInfoFrame::init() {
  // Read default avatar explicitly.
  m_private->avatar_frame_->readConf();

  m_private->form_frame_->readConf();

  // Read default keyboard layout.
  m_private->keyboard_frame_->readConf();
}

void SystemInfoFrame::finished() {
  // Notify sub-pages to save settings.
  m_private->avatar_frame_->writeConf();
  m_private->form_frame_->writeConf();
  m_private->keyboard_frame_->writeConf();
}

bool SystemInfoFrame::shouldDisplay() const
{
    return !(GetSettingsBool(kSystemInfoSetupAfterReboot) || GetSettingsBool(kSkipSystemInfoPage) || GetSettingsBool(kPartitionDoAutoPart));
}

QString SystemInfoFrame::returnFrameName() const
{
    return "Create Account";
}

void SystemInfoFramePrivate::initConnections() {
  connect(form_frame_, &SystemInfoFormFrame::systemInfoFormDone, this, [=] {
      emit nextButton->clicked();
  });
  connect(avatar_frame_, &SystemInfoAvatarFrame::finished,
          this, &SystemInfoFramePrivate::showFormPage);
  connect(avatar_frame_, &SystemInfoAvatarFrame::avatarUpdated,
          form_frame_, &SystemInfoFormFrame::updateAvatar);

  // Save settings when finished signal is emitted.
  connect(form_frame_, &SystemInfoFormFrame::avatarClicked,
          this, &SystemInfoFramePrivate::showAvatarPage);
  connect(keyboard_frame_, &SystemInfoKeyboardFrame::finished,
          this, &SystemInfoFramePrivate::restoreLastPage);
  connect(keyboard_frame_, &SystemInfoKeyboardFrame::layoutUpdated,
          this, &SystemInfoFramePrivate::updateLayout);

  connect(keyboard_button_, &QPushButton::clicked,
          this, &SystemInfoFramePrivate::showKeyboardPage);

  connect(form_frame_, &SystemInfoFormFrame::requestNextButtonEnable, nextButton, &QPushButton::setEnabled);
}

void SystemInfoFramePrivate::initUI() {
  keyboard_button_ = new QPushButton();
  keyboard_button_->setFixedWidth(250);
  keyboard_button_->setFixedHeight(30);

  bottom_layout_ = new QHBoxLayout();
  bottom_layout_->setContentsMargins(30, 0, 0, 0);
  bottom_layout_->setSpacing(30);
  bottom_layout_->addWidget(keyboard_button_);
  bottom_layout_->addStretch();

  avatar_frame_ = new SystemInfoAvatarFrame();
  form_frame_ = new SystemInfoFormFrame();
  keyboard_frame_ = new SystemInfoKeyboardFrame();

  stacked_layout_ = new QStackedLayout();
  stacked_layout_->setContentsMargins(0, 0, 0, 0);
  stacked_layout_->setSpacing(0);
  stacked_layout_->addWidget(avatar_frame_);
  stacked_layout_->addWidget(form_frame_);
  stacked_layout_->addWidget(keyboard_frame_);

  centerLayout->addLayout(stacked_layout_);
  centerLayout->addLayout(bottom_layout_);

  nextButton->setText(tr("Next"));

  q_ptr->setContentsMargins(0, 0, 0, 0);
}

void SystemInfoFramePrivate::updateHeadBar() {
  const QString name = stacked_layout_->currentWidget()->objectName();
  const int page = stacked_layout_->currentIndex();

  // Only show header bar in avatar page and form page.
  if (page == kAvatarPageId || page == kFormPageId) {
    keyboard_button_->setVisible(!disable_keyboard_);
  } else {
    keyboard_button_->hide();
  }
}

void SystemInfoFramePrivate::restoreLastPage() {
  if (last_page_ != kInvalidPageId) {
    stacked_layout_->setCurrentIndex(last_page_);
  } else {
    // Displays default page if last_page_ is Rnot set.
    stacked_layout_->setCurrentWidget(form_frame_);
  }
  nextButton->show();
  updateHeadBar();
}

void SystemInfoFramePrivate::showAvatarPage() {
  if (!GetSettingsBool(kSystemInfoDisableAvatorPage)) {
    stacked_layout_->setCurrentWidget(avatar_frame_);
    updateHeadBar();
  }
}

void SystemInfoFramePrivate::showFormPage() {
  stacked_layout_->setCurrentWidget(form_frame_);
  nextButton->show();
  updateHeadBar();
}

void SystemInfoFramePrivate::showKeyboardPage() {
  if (!disable_keyboard_) {
    last_page_ = stacked_layout_->currentIndex();
    stacked_layout_->setCurrentWidget(keyboard_frame_);
    nextButton->hide();
    updateHeadBar();
  }
}

void SystemInfoFramePrivate::updateLayout(const QString& layout) {
  keyboard_button_->setText(layout);
}

bool SystemInfoFramePrivate::validate() const
{
    return form_frame_->validateUserInfo();
}

}  // namespace installer

#include "system_info_frame.moc"
