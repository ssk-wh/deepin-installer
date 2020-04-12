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

#include <DSysInfo>

#include "ui/frames/language_frame.h"
#include "ui/frames/inner/select_language_frame.h"
#include "ui/frames/inner/user_agreement_frame.h"
#include "service/settings_manager.h"
#include "ui/delegates/user_agreement_delegate.h"

DCORE_USE_NAMESPACE

namespace installer {

LanguageFrame::LanguageFrame(QWidget *parent)
    : QWidget(parent)
    , m_frame_layout(new QStackedLayout)
    , m_user_license_delegate(new UserAgreementDelegate())
    , m_select_language_frame(new SelectLanguageFrame(m_user_license_delegate))
    , m_user_license_frame(new UserAgreementFrame)
{
    initUI();
    initConnect();
}

LanguageFrame::~LanguageFrame() {}

void LanguageFrame::readConf() {
    m_select_language_frame->readConf();
}

void LanguageFrame::writeConf() {
    m_select_language_frame->writeConf();
}

void LanguageFrame::initUI() {
    m_frame_layout->setMargin(0);
    m_frame_layout->addWidget(m_select_language_frame);
    m_frame_layout->addWidget(m_user_license_frame);

    setLayout(m_frame_layout);
}

void LanguageFrame::initConnect() {
    connect(m_select_language_frame, &SelectLanguageFrame::finished, this,
            &LanguageFrame::finished);
    connect(m_select_language_frame, &SelectLanguageFrame::timezoneUpdated, this,
            &LanguageFrame::timezoneUpdated);
    connect(m_select_language_frame, &SelectLanguageFrame::requestShowUserLicense, this,
            &LanguageFrame::showUserLicense);
    connect(m_user_license_frame, &UserAgreementFrame::back, this,
            &LanguageFrame::showLanguage);
    if (m_user_license_delegate->licenseCount() > 0) {
        connect(m_select_language_frame, &SelectLanguageFrame::requestShowOemUserLicense, this,
            &LanguageFrame::showOemUserLicense);
    }
}

void LanguageFrame::showUserLicense() {
    QString zh_CN_license;
    QString en_US_license;

    if (DSysInfo::deepinType() == DSysInfo::DeepinDesktop) {
      zh_CN_license = ":/license/deepin-end-user-license-agreement_zh_CN.txt";
      en_US_license = ":/license/deepin-end-user-license-agreement_en_US.txt";
    }
    else {
      zh_CN_license = ":/license/deepin-end-user-license-agreement_community_zh_CN.txt";
      en_US_license = ":/license/deepin-end-user-license-agreement_community_en_US.txt";
    }

    if (installer::ReadLocale() == "zh_CN") {
        m_user_license_frame->setUserAgreement(zh_CN_license, en_US_license);
        m_user_license_frame->setCheckedButton(kChineseToggleButtonId);
    } else {
        m_user_license_frame->setUserAgreement(en_US_license, zh_CN_license);
        m_user_license_frame->setCheckedButton(kEnglishToggleButtonId);
    }

    m_frame_layout->setCurrentWidget(m_user_license_frame);
}

void LanguageFrame::showLanguage() {
    m_frame_layout->setCurrentWidget(m_select_language_frame);
}

void LanguageFrame::showOemUserLicense() {
    LicenseItem primaryLicense;
    primaryLicense = m_user_license_delegate->getPrimaryAdaptiveLicense(installer::ReadLocale());
    m_user_license_frame->setUserAgreement(primaryLicense.fileName());
    m_frame_layout->setCurrentWidget(m_user_license_frame);
}

}  // namespace installer
