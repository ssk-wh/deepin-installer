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
#include "ui/delegates/license_delegate.h"

#include <QApplication>
#include <DSysInfo>
#include <QDebug>

DCORE_USE_NAMESPACE

namespace installer {

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
        , m_privacy_license_frame(new UserAgreementFrame)
    {}

    void initUI();
    void initConnect();
    void showUserLicense();
    void showLanguage();
    void showOemUserLicense();
    void showUserExperience();
    void showPrivacyLicense();
    QString agreementLocale();

    void setupTs();

    void updateNextButton() const override {
        if (m_select_language_frame->isChecked()) {
            nextButton->setEnabled(true);
        }
    }

    LanguageFrame*       q_ptr                   = nullptr;
    Q_DECLARE_PUBLIC(LanguageFrame)
    QStackedLayout*      m_frame_layout          = nullptr;
    UserAgreementDelegate* m_user_license_delegate = nullptr;
    SelectLanguageFrame* m_select_language_frame = nullptr;
    UserAgreementFrame*  m_user_license_frame    = nullptr;
    UserAgreementFrame*  m_user_experience_frame = nullptr;
    UserAgreementFrame*  m_privacy_license_frame = nullptr;
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
    return ((!GetSettingsBool(kSkipSelectLanguagePage) && !GetSettingsBool("DI_IS_FIRST_BOOT"))                 // 前配置的配置选项生效
            || (!GetSettingsBool(kSkipSelectLanguagePageOnFirstBoot) && GetSettingsBool("DI_IS_FIRST_BOOT")))   // 后配置的配置选项生效
            && !GetSettingsBool("DI_LUPIN");
}

QString LanguageFrame::returnFrameName() const
{
    return ::QObject::tr("Select Language");
}

void LanguageFrame::acceptLicense(bool accept) const
{
    m_private->m_select_language_frame->acceptLicense(accept);
}

void LanguageFrame::changeEvent(QEvent *event)
{
    Q_D(LanguageFrame);

    if (event->type() == QEvent::LanguageChange) {
        for (auto it = d->m_trList.begin(); it != d->m_trList.end(); ++it) {
            it->first(qApp->translate("QObject", it->second.toUtf8()));
        }

        d->setupTs();
    }
    else {
        FrameInterface::changeEvent(event);
    }
}

void LanguageFrame::showEvent(QShowEvent *event)
{
    Q_D(LanguageFrame);
    this->setCurentFocus(d->m_select_language_frame->getLanguageView());
    return FrameInterface::showEvent(event);
}

bool LanguageFrame::focusSwitch()
{
    Q_D(LanguageFrame);

    if (d->m_select_language_frame->getLanguageView()->hasFocus()) {
        if (d->m_select_language_frame->getLanguageView() != m_current_focus_widget) {
            this->setCurentFocus(d->m_select_language_frame->getLanguageView());
        }
    } else if (d->m_select_language_frame->getAcceptexperience()->hasFocus()) {
        if (d->m_select_language_frame->getAcceptexperience() != m_current_focus_widget) {
            this->setCurentFocus(d->m_select_language_frame->getAcceptexperience());
        }
    } else if (d->m_select_language_frame->getExperiencelabel()->hasFocus()) {
        if (d->m_select_language_frame->getExperiencelabel() != m_current_focus_widget) {
            this->setCurentFocus(d->m_select_language_frame->getExperiencelabel());
        }
    } else if (d->m_select_language_frame->getAcceptlicense()->hasFocus()) {
        if (d->m_select_language_frame->getAcceptlicense() != m_current_focus_widget) {
            this->setCurentFocus(d->m_select_language_frame->getAcceptlicense());
        }
    } else if(d->m_select_language_frame->getLicenselabel()->hasFocus()) {
        if (d->m_select_language_frame->getLicenselabel() != m_current_focus_widget) {
            this->setCurentFocus(d->m_select_language_frame->getLicenselabel());
        }
    } else if(d->m_select_language_frame->getPrivacylicenselabel()->hasFocus()) {
        if (d->m_select_language_frame->getPrivacylicenselabel() != m_current_focus_widget) {
            this->setCurentFocus(d->m_select_language_frame->getPrivacylicenselabel());
        }
    }


    if (m_current_focus_widget == nullptr) {
        if(d->nextButton->isEnabled()){
            this->setCurentFocus(d->nextButton);
        } else {
            this->setCurentFocus(d->m_select_language_frame->getLanguageView());
        }
    } else if (d->nextButton == m_current_focus_widget) {
        this->setCurentFocus(d->m_select_language_frame->getLanguageView());
    } else if (d->m_select_language_frame->getLanguageView() == m_current_focus_widget) {
        if (d->m_select_language_frame->getAcceptexperience()->isVisible()) {
            this->setCurentFocus(d->m_select_language_frame->getAcceptexperience());
        } else if (d->m_select_language_frame->getAcceptlicense()->isVisible()) {
            this->setCurentFocus(d->m_select_language_frame->getAcceptlicense());
        }
    } else if (d->m_select_language_frame->getAcceptexperience() == m_current_focus_widget) {
        this->setCurentFocus(d->m_select_language_frame->getExperiencelabel());
    } else if (d->m_select_language_frame->getExperiencelabel() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
            if (d->m_select_language_frame->getAcceptlicense()->isVisible()) {
                this->setCurentFocus(d->m_select_language_frame->getAcceptlicense());
            } else {
                if(d->nextButton->isEnabled()){
                    this->setCurentFocus(d->nextButton);
                } else {
                    this->setCurentFocus(d->m_select_language_frame->getLanguageView());
                }
            }
        } else {
            return d->m_user_experience_frame->focusSwitch();
        }
    } else if (d->m_select_language_frame->getAcceptlicense() == m_current_focus_widget) {
        this->setCurentFocus(d->m_select_language_frame->getLicenselabel());
    } else if (d->m_select_language_frame->getLicenselabel() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
            this->setCurentFocus(d->m_select_language_frame->getPrivacylicenselabel());
        } else {
            return d->m_user_license_frame->focusSwitch();
        }
    } else if (d->m_select_language_frame->getPrivacylicenselabel() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
            if(d->nextButton->isEnabled()){
                this->setCurentFocus(d->nextButton);
            } else {
                this->setCurentFocus(d->m_select_language_frame->getLanguageView());
            }
        } else {
            return d->m_privacy_license_frame->focusSwitch();
        }
    }

    return true;
}

bool LanguageFrame::doSpace()
{
    Q_D(LanguageFrame);

    if (m_current_focus_widget == nullptr) {
    } else if (d->m_select_language_frame->getAcceptexperience() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
            if (d->m_select_language_frame->setAcceptexperience()) {
                this->setCurentFocus(d->m_select_language_frame->getAcceptlicense());
            } else {
                this->setCurentFocus(d->m_select_language_frame->getLanguageView());
            }
        }
    } else if (d->m_select_language_frame->getAcceptlicense() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
            if (d->m_select_language_frame->setAcceptlicense()) {
                this->setCurentFocus(d->nextButton);
            } else {
                this->setCurentFocus(d->m_select_language_frame->getLanguageView());
            }
        }
    }

    return true;
}

bool LanguageFrame::doSelect()
{
    Q_D(LanguageFrame);

    if (d->m_select_language_frame->getLanguageView()->hasFocus()) {
        if (d->m_select_language_frame->getLanguageView() != m_current_focus_widget) {
            this->setCurentFocus(d->m_select_language_frame->getLanguageView());
        }
    }

    if (m_current_focus_widget == nullptr) {
    } else if (d->nextButton == m_current_focus_widget) {
        d->nextButton->click();
    } else if (d->m_select_language_frame->getLanguageView() == m_current_focus_widget) {
        d->m_select_language_frame->doSelect();
        return focusSwitch();
    } else if (d->m_select_language_frame->getLicenselabel() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
            d->m_select_language_frame->requestShowUserLicense();
        } else {
            return d->m_user_license_frame->doSelect();
        }
    } else if (d->m_select_language_frame->getExperiencelabel() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
            d->m_select_language_frame->requestShowUserExperience();
        } else {
            return d->m_user_experience_frame->doSelect();
        }
    } else if (d->m_select_language_frame->getPrivacylicenselabel() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
            d->m_select_language_frame->requestPrivacyLicense();
        } else {
            return d->m_privacy_license_frame->doSelect();
        }
    }

    return true;
}

bool LanguageFrame::directionKey(int keyvalue)
{
    Q_D(LanguageFrame);

    if (m_current_focus_widget == nullptr) {
    } else if (d->nextButton == m_current_focus_widget) {
    } else if (d->m_select_language_frame->getLanguageView() == m_current_focus_widget) {
        d->m_select_language_frame->directionKey(keyvalue);
    } else if (d->m_select_language_frame->getLicenselabel() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
        } else {
            d->m_user_license_frame->directionKey(keyvalue);
        }
    } else if (d->m_select_language_frame->getExperiencelabel() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
        } else {
            d->m_user_experience_frame->directionKey(keyvalue);
        }
    } else if (d->m_select_language_frame->getPrivacylicenselabel() == m_current_focus_widget) {
        if (d->nextButton->isVisible()) {
        } else {
            d->m_privacy_license_frame->directionKey(keyvalue);
        }
    }

    return true;
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
    m_frame_layout->addWidget(m_privacy_license_frame);

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
    connect(m_select_language_frame, &SelectLanguageFrame::coverMainWindowFrameLabelsView, q_ptr
            , &LanguageFrame::coverMainWindowFrameLabelsView);
    connect(m_user_experience_frame, &UserAgreementFrame::back, this,
            &LanguageFramePrivate::showLanguage);
    if (m_user_license_delegate->licenseCount() > 0) {
        connect(m_select_language_frame, &SelectLanguageFrame::requestShowOemUserLicense, this,
            &LanguageFramePrivate::showOemUserLicense);
    }

    connect(m_select_language_frame, &SelectLanguageFrame::requestNextButtonEnable, nextButton, &QPushButton::setEnabled);

    connect(m_privacy_license_frame, &UserAgreementFrame::back, this,
            &LanguageFramePrivate::showLanguage);

    connect(m_select_language_frame, &SelectLanguageFrame::requestPrivacyLicense, this,
            &LanguageFramePrivate::showPrivacyLicense);
}

QString LanguageFramePrivate::agreementLocale() {
    QString locale = installer::ReadLocale();
    if (GetCurrentType() == OSType::Professional) {
        if (locale != "zh_CN" && locale != "zh_HK"
            && locale != "zh_TW" && locale != "bo_CN" && locale != "ug_CN") {
            locale = "en_US";
        }
    } else if (locale == "zh_CN" || locale == "zh_HK"
               || locale == "zh_TW" || locale == "bo_CN" || locale == "ug_CN"){
        locale = "zh_CN";
    } else {
        locale = "en_US";
    }
    return locale;
}

void LanguageFramePrivate::showUserLicense() {
    QString osType = installer::LicenseDelegate::OSType();

    QString userLicenseText = QString(GetOemLicenseDir() + "/end-user-license-agreement-%1_%2.txt")\
        .arg(osType, agreementLocale());

    // 军版最终用户协议txt版路径("/usr/share/protocol/enduser-agreement/End-User-License-Agreement-Professional-Military-%1.txt")
    osType.replace(0, 1, osType[0].toUpper());
    QString militaryLicenseText = QString("/usr/share/protocol/enduser-agreement/End-User-License-Agreement-%1-Military-%2.txt")
            .arg(osType, agreementLocale());
    if (QFile::exists(militaryLicenseText)) {
        userLicenseText = militaryLicenseText;
    }

    m_user_license_frame->setUserAgreement(userLicenseText);

    m_frame_layout->setCurrentWidget(m_user_license_frame);

    nextButton->hide();

    q_ptr->setCurentFocus(m_select_language_frame->getLicenselabel());
}

void LanguageFramePrivate::showLanguage() {
    m_frame_layout->setCurrentWidget(m_select_language_frame);
    nextButton->show();
}

void LanguageFramePrivate::showOemUserLicense() {
    LicenseItem primaryLicense;
    primaryLicense = m_user_license_delegate->getPrimaryAdaptiveLicense(installer::ReadLocale());
    m_user_license_frame->setUserAgreement(primaryLicense.fileName());
    m_frame_layout->setCurrentWidget(m_user_license_frame);
}

void LanguageFramePrivate::showUserExperience()
{
    QString userExperienceText = QString(GetOemLicenseDir() + "/user-experience-agreement-%1_%2.txt")\
            .arg(installer::LicenseDelegate::OSType(), agreementLocale());

    m_user_experience_frame->setUserAgreement(userExperienceText);
    m_frame_layout->setCurrentWidget(m_user_experience_frame);

    nextButton->hide();

    q_ptr->setCurentFocus(m_select_language_frame->getExperiencelabel());
}

void LanguageFramePrivate::showPrivacyLicense()
{
    QString privacyLicenseText = QString(GetOemLicenseDir() + "/privacy-policy-%1_%2.txt")\
            .arg(installer::LicenseDelegate::OSType(), agreementLocale());

    m_privacy_license_frame->setUserAgreement(privacyLicenseText);

    m_frame_layout->setCurrentWidget(m_privacy_license_frame);

    nextButton->hide();

    q_ptr->setCurentFocus(m_select_language_frame->getPrivacylicenselabel());
}

void LanguageFramePrivate::setupTs()
{
    nextButton->setText(::QObject::tr("Next"));
}

}  // namespace installer

#include "language_frame.moc"
