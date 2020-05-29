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

#include "ui/frames/language_frame.h"
#include "ui/frames/inner/select_language_frame.h"
#include "ui/frames/inner/user_agreement_frame.h"
#include "service/settings_manager.h"
#include "ui/delegates/user_agreement_delegate.h"
#include "service/settings_name.h"
#include "ui/interfaces/frameinterfaceprivate.h"
#include "ui/utils/widget_util.h"

#include <QApplication>

namespace installer {

#ifdef PROFESSIONAL
const QString zh_CN_license { ":/license/deepin-end-user-license-agreement_zh_CN.txt" };
const QString en_US_license{ ":/license/deepin-end-user-license-agreement_en_US.txt" };
#else
const QString zh_CN_license { ":/license/deepin-end-user-license-agreement_community_zh_CN.txt" };
const QString en_US_license{ ":/license/deepin-end-user-license-agreement_community_en_US.txt" };
#endif  // PROFESSIONAL

const QString zh_CN_experience { ":/license/deepin-end-user-experience-agreement_zh_CN.txt" };
const QString en_US_experience { ":/license/deepin-end-user-experience-agreement_en_US.txt" };

class LanguageFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    explicit LanguageFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate (parent)
        , q_ptr(qobject_cast<LanguageFrame* > (parent))
        , m_frame_layout(new QStackedLayout)
        , m_user_license_delegate(new UserAgreementDelegate())
        , m_select_language_frame(new SelectLanguageFrame(m_user_license_delegate))
        , m_user_license_frame(new UserAgreementFrame)
        , m_user_experience_frame(new UserAgreementFrame)
    {}

    void initUI();
    void initConnect();
    void showUserLicense();
    void showLanguage();
    void showOemUserLicense();
    void showUserExperience();

    void onNextButtonClickHandle() const override {
        FrameInterfacePrivate::onNextButtonClickHandle();
        nextButton->setEnabled(false);
    }

    void updateNextButton() const override {
        if (m_select_language_frame->isChecked()) {
            nextButton->setEnabled(true);
        }
    }

    LanguageFrame*       q_ptr                   = nullptr;
    QStackedLayout*      m_frame_layout          = nullptr;
    UserAgreementDelegate* m_user_license_delegate = nullptr;
    SelectLanguageFrame* m_select_language_frame = nullptr;
    UserAgreementFrame*  m_user_license_frame    = nullptr;
    UserAgreementFrame*  m_user_experience_frame = nullptr;
    std::list<std::pair<std::function<void (QString)>, QString>> m_trList;
};
LanguageFrame::LanguageFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new LanguageFramePrivate(this))
{
    m_private->initUI();
    m_private->initConnect();
}

LanguageFrame::~LanguageFrame() {}

bool LanguageFrame::shouldDisplay() const
{
    return !GetSettingsBool(kSkipSelectLanguagePage);
}

QString LanguageFrame::returnFrameName() const
{
    return "Select language";
}

void LanguageFrame::changeEvent(QEvent *event)
{
    Q_D(LanguageFrame);

    if (event->type() == QEvent::LanguageChange) {
        for (auto it = d->m_trList.begin(); it != d->m_trList.end(); ++it) {
            it->first(qApp->translate("installer::LanguageFramePrivate", it->second.toUtf8()));
        }
    }
    else {
        FrameInterface::changeEvent(event);
    }
}

void LanguageFrame::init() {
    m_private->m_select_language_frame->readConf();
}

void LanguageFrame::finished() {
    m_private->m_select_language_frame->writeConf();
}

void LanguageFramePrivate::initUI() {
    m_frame_layout->setMargin(0);
    m_frame_layout->addWidget(m_select_language_frame);
    m_frame_layout->addWidget(m_user_experience_frame);
    m_frame_layout->addWidget(m_user_license_frame);
    m_user_experience_frame->setTitle(tr("The user experience program license agreement"));
    addTransLate(m_trList, std::bind(&UserAgreementFrame::setTitle, m_user_experience_frame, std::placeholders::_1), QString(tr("The user experience program license agreement")));

    m_user_license_frame->setTitle(tr("End User License Agreement"));
    addTransLate(m_trList, std::bind(&UserAgreementFrame::setTitle, m_user_license_frame, std::placeholders::_1), QString(tr("End User License Agreement")));

    nextButton->setEnabled(false);
    centerLayout->addLayout(m_frame_layout);
}

void LanguageFramePrivate::initConnect() {
    connect(m_select_language_frame, &SelectLanguageFrame::requestApplyLanguage, this, [=] {
        emit nextButton->clicked();
    });
    connect(m_select_language_frame, &SelectLanguageFrame::timezoneUpdated, q_ptr,
            &LanguageFrame::timezoneUpdated);
    connect(m_select_language_frame, &SelectLanguageFrame::requestShowUserLicense, this,
            &LanguageFramePrivate::showUserLicense);
    connect(m_user_license_frame, &UserAgreementFrame::back, this,
            &LanguageFramePrivate::showLanguage);
    connect(m_select_language_frame, &SelectLanguageFrame::requestShowUserExperience, this,
            &LanguageFramePrivate::showUserExperience);
    connect(m_user_experience_frame, &UserAgreementFrame::back, this,
            &LanguageFramePrivate::showLanguage);
    if (m_user_license_delegate->licenseCount() > 0) {
        connect(m_select_language_frame, &SelectLanguageFrame::requestShowOemUserLicense, this,
            &LanguageFramePrivate::showOemUserLicense);
    }

    connect(m_select_language_frame, &SelectLanguageFrame::requestNextButtonEnable, nextButton, &QPushButton::setEnabled);
}

void LanguageFramePrivate::showUserLicense() {
    if (installer::ReadLocale() == "zh_CN") {
        m_user_license_frame->setUserAgreement(zh_CN_license, en_US_license);
        m_user_license_frame->setCheckedButton(kChineseToggleButtonId);
    } else {
        m_user_license_frame->setUserAgreement(en_US_license, zh_CN_license);
        m_user_license_frame->setCheckedButton(kEnglishToggleButtonId);
    }
    m_frame_layout->setCurrentWidget(m_user_license_frame);

    nextButton->hide();
}

void LanguageFramePrivate::showLanguage() {
    LanguageManager::translator(nextButton, &QPushButton::setText, TranslatorType::NextButton);
    nextButton->show();

    m_frame_layout->setCurrentWidget(m_select_language_frame);
}

void LanguageFramePrivate::showOemUserLicense() {
    LicenseItem primaryLicense;
    primaryLicense = m_user_license_delegate->getPrimaryAdaptiveLicense(installer::ReadLocale());
    m_user_license_frame->setUserAgreement(primaryLicense.fileName());
    m_frame_layout->setCurrentWidget(m_user_license_frame);
}

void LanguageFramePrivate::showUserExperience()
{
    if (installer::ReadLocale() == "zh_CN") {
        m_user_experience_frame->setUserAgreement(zh_CN_experience, en_US_experience);
        m_user_experience_frame->setCheckedButton(kChineseToggleButtonId);
    } else {
        m_user_experience_frame->setUserAgreement(en_US_experience, zh_CN_experience);
        m_user_experience_frame->setCheckedButton(kEnglishToggleButtonId);
    }
    m_frame_layout->setCurrentWidget(m_user_experience_frame);

    nextButton->hide();
}

}  // namespace installer

#include "language_frame.moc"
