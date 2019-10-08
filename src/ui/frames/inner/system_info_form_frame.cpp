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

#include "ui/frames/inner/system_info_form_frame.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QProcess>
#include <QDebug>

#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "sysinfo/validate_hostname.h"
#include "sysinfo/validate_password.h"
#include "sysinfo/validate_username.h"
#include "ui/frames/consts.h"
#include "ui/utils/keyboardmonitor.h"
#include "ui/widgets/avatar_button.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/line_edit.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/system_info_tip.h"
#include "ui/widgets/title_label.h"

namespace installer {

namespace {
    const int kSetRootPasswordCheckBoxWidth = 310;
    const int kSetRootPasswordCheckBoxHeight = 36;
}

SystemInfoFormFrame::SystemInfoFormFrame(QWidget* parent)
    : QFrame(parent)
    , is_username_edited_(false)
    , is_hostname_edited_(false)
    , is_hostname_edited_manually_(false)
    , is_password_edited_(false)
    , is_password2_edited_(false)
    , m_isRootPasswordEdited(false)
    , m_isRootPasswordCheckEdited(false)
{
    this->setObjectName("system_info_form_frame");

    this->initUI();
    this->initConnections();

    KeyboardMonitor::instance()->start();
}

void SystemInfoFormFrame::updateAvatar(const QString& avatar)
{
    avatar_button_->updateIcon(avatar);
}

void SystemInfoFormFrame::writeConf()
{
    WriteUsername(username_edit_->text());
    WriteHostname(hostname_edit_->text());
    WritePassword(password_edit_->text());
    WriteRootPassword(m_rootPasswordEdit->text());
}

void SystemInfoFormFrame::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        title_label_->setText(tr("Create User Account"));
        comment_label_->setText(tr("Fill in the username, computer name and your password"));
        username_edit_->setPlaceholderText(tr("Username"));
        hostname_edit_->setPlaceholderText(tr("Computer name"));
        password_edit_->setPlaceholderText(tr("Password"));
        password_check_edit_->setPlaceholderText(tr("Repeat password"));
        m_setRootPasswordCheck->setText(tr("Set root password"));
        m_rootPasswordEdit->setPlaceholderText(tr("Root password"));
        m_rootPasswordCheckEdit->setPlaceholderText(tr("Repeat root password"));
        next_button_->setText(tr("Next"));
        grub_password_check_->setText(tr("Use that password to edit boot menu"));
    }
    else {
        QFrame::changeEvent(event);
    }
}

void SystemInfoFormFrame::showEvent(QShowEvent* event)
{
    QFrame::showEvent(event);
    username_edit_->setFocus();
    tooltip_->hide();
}

void SystemInfoFormFrame::initConnections()
{
    connect(avatar_button_, &QPushButton::clicked, this,
            &SystemInfoFormFrame::avatarClicked);
    connect(next_button_, &QPushButton::clicked, this,
            &SystemInfoFormFrame::onNextButtonClicked);

    connect(username_edit_, &LineEdit::editingFinished, this,
            &SystemInfoFormFrame::onUsernameEditingFinished);
    connect(hostname_edit_, &LineEdit::editingFinished, this,
            &SystemInfoFormFrame::onHostnameEditingFinished);
    connect(password_edit_, &LineEdit::editingFinished, this,
            &SystemInfoFormFrame::onPasswordEditingFinished);
    connect(password_check_edit_, &LineEdit::editingFinished, this,
            &SystemInfoFormFrame::onPassword2EditingFinished);
    connect(m_rootPasswordEdit, &LineEdit::editingFinished, this
            , &SystemInfoFormFrame::onRootPasswordEditingFinished);
    connect(m_rootPasswordCheckEdit, &LineEdit::editingFinished, this
            , &SystemInfoFormFrame::onRootPasswordCheckEditingFinished);

    connect(username_edit_, SIGNAL(returnPressed()), hostname_edit_,
            SLOT(setFocus()));
    connect(hostname_edit_, SIGNAL(returnPressed()), password_edit_,
            SLOT(setFocus()));
    connect(password_edit_, SIGNAL(returnPressed()), password_check_edit_,
            SLOT(setFocus()));
    connect(password_check_edit_, SIGNAL(returnPressed()), next_button_,
            SIGNAL(clicked()));
    connect(m_setRootPasswordCheck, &QCheckBox::clicked, this
            , &SystemInfoFormFrame::onSetRootPasswordCheckChanged);
    connect(m_rootPasswordEdit, SIGNAL(returnPressed()), m_rootPasswordCheckEdit
            , SLOT(setFocus()));
    connect(m_rootPasswordCheckEdit, SIGNAL(returnPressed()), next_button_
            , SIGNAL(clicked()));

    connect(username_edit_, &LineEdit::textEdited, this,
            &SystemInfoFormFrame::onEditingLineEdit);
    connect(username_edit_, &LineEdit::textEdited, this,
            &SystemInfoFormFrame::onUsernameEdited);
    connect(hostname_edit_, &LineEdit::textEdited, this,
            &SystemInfoFormFrame::onEditingLineEdit);
    connect(hostname_edit_, &LineEdit::textEdited, this,
            &SystemInfoFormFrame::onHostnameEdited);
    connect(password_edit_, &LineEdit::textEdited, this,
            &SystemInfoFormFrame::onEditingLineEdit);
    connect(password_edit_, &LineEdit::textEdited, this,
            &SystemInfoFormFrame::onPasswordEdited);
    connect(password_check_edit_, &LineEdit::textEdited, this,
            &SystemInfoFormFrame::onEditingLineEdit);
    connect(password_check_edit_, &LineEdit::textEdited, this,
            &SystemInfoFormFrame::onPassword2Edited);
    connect(m_rootPasswordEdit, &LineEdit::textEdited, this
            , &SystemInfoFormFrame::onEditingLineEdit);
    connect(m_rootPasswordEdit, &LineEdit::textEdited, this
            , &SystemInfoFormFrame::onRootPasswordEdited);
    connect(m_rootPasswordCheckEdit, &LineEdit::textEdited, this
            , &SystemInfoFormFrame::onEditingLineEdit);
    connect(m_rootPasswordCheckEdit, &LineEdit::textEdited, this
            , &SystemInfoFormFrame::onRootPasswordCheckEdited);

    connect(KeyboardMonitor::instance(),
            &KeyboardMonitor::capslockStatusChanged, this,
            &SystemInfoFormFrame::updateCapsLockState);
}

void SystemInfoFormFrame::initUI()
{
    title_label_   = new TitleLabel(tr("Create User Account"));
    comment_label_ = new CommentLabel(tr("Fill in the username, computer name and your password"));
    QHBoxLayout* comment_layout = new QHBoxLayout();
    comment_layout->setContentsMargins(0, 0, 0, 0);
    comment_layout->setSpacing(0);
    comment_layout->addWidget(comment_label_);

    avatar_button_ = new AvatarButton();

    username_edit_ = new LineEdit(":/images/username_12.svg");
    username_edit_->setPlaceholderText(tr("Username"));
    username_edit_->setText(GetSettingsString(kSystemInfoDefaultUsername));
    username_edit_->setReadOnly(GetSettingsBool(kSystemInfoLockUsername));

    hostname_edit_ = new LineEdit(":/images/hostname_12.svg");
    hostname_edit_->setPlaceholderText(tr("Computer name"));
    hostname_edit_->setText(GetSettingsString(kSystemInfoDefaultHostname));
    hostname_edit_->setReadOnly(GetSettingsBool(kSystemInfoLockHostname));

    password_edit_ = new LineEdit(":/images/password_12.svg");
    password_edit_->setPlaceholderText(tr("Password"));
    password_edit_->setEchoMode(QLineEdit::Password);
    password_edit_->setText(GetSettingsString(kSystemInfoDefaultPassword));
    password_edit_->setReadOnly(GetSettingsBool(kSystemInfoLockPassword));

    password_check_edit_ = new LineEdit(":/images/password_12.svg");
    password_check_edit_->setPlaceholderText(tr("Repeat password"));
    password_check_edit_->setEchoMode(QLineEdit::Password);
    password_check_edit_->setText(password_edit_->text());
    password_check_edit_->setReadOnly(password_edit_->isReadOnly());

    m_setRootPasswordCheck = new QCheckBox(tr("Set root password"), this);
    m_setRootPasswordCheck->setCheckable(true);
    m_setRootPasswordCheck->setChecked(false);
    m_setRootPasswordCheck->setObjectName("RootPasswordCheckBox");

    m_rootPasswordEdit = new LineEdit(":/images/password_12.svg");
    m_rootPasswordEdit->setPlaceholderText(tr("Root password"));
    m_rootPasswordEdit->setEchoMode(QLineEdit::Password);
    m_rootPasswordEdit->setText(GetSettingsString(kSystemInfoDefaultPassword));
    m_rootPasswordEdit->setReadOnly(GetSettingsBool(kSystemInfoLockPassword));
    QSizePolicy sp_retain = m_rootPasswordEdit->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    m_rootPasswordEdit->setSizePolicy(sp_retain);
    m_rootPasswordEdit->hide();

    m_rootPasswordCheckEdit = new LineEdit(":/images/password_12.svg");
    m_rootPasswordCheckEdit->setPlaceholderText(tr("Repeat root password"));
    m_rootPasswordCheckEdit->setEchoMode(QLineEdit::Password);
    m_rootPasswordCheckEdit->setText(m_rootPasswordEdit->text());
    m_rootPasswordCheckEdit->setReadOnly(m_rootPasswordEdit->isReadOnly());
    sp_retain = m_rootPasswordCheckEdit->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    m_rootPasswordCheckEdit->setSizePolicy(sp_retain);
    m_rootPasswordCheckEdit->hide();

    m_editList.push_back(username_edit_);
    m_editList.push_back(hostname_edit_);
    m_editList.push_back(password_edit_);
    m_editList.push_back(password_check_edit_);
    m_editList.push_back(m_rootPasswordEdit);
    m_editList.push_back(m_rootPasswordCheckEdit);

    tooltip_ = new SystemInfoTip(this);
    tooltip_->hide();

    grub_password_check_ = new QCheckBox(tr("Use that password to edit boot menu"), this);
    grub_password_check_->setCheckable(true);
    grub_password_check_->setChecked(false);
    grub_password_check_->setObjectName("GrubPasswordCheckBox");
    grub_password_check_->setVisible(GetSettingsBool(kSystemInfoEnableGrubEditPwd));

    next_button_ = new NavButton(tr("Next"));

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kMainLayoutSpacing);
    layout->addSpacing(50);
    layout->addWidget(title_label_, 0, Qt::AlignCenter);
    layout->addLayout(comment_layout);
    layout->addSpacing(40);
    layout->addWidget(avatar_button_, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(username_edit_, 0, Qt::AlignCenter);
    layout->addWidget(hostname_edit_, 0, Qt::AlignCenter);
    layout->addWidget(password_edit_, 0, Qt::AlignCenter);
    layout->addWidget(password_check_edit_, 0, Qt::AlignCenter);
    m_setRootPasswordCheck->setFixedSize(kSetRootPasswordCheckBoxWidth, kSetRootPasswordCheckBoxHeight);
    layout->addWidget(m_setRootPasswordCheck, 0, Qt::AlignCenter);
    layout->addWidget(m_rootPasswordEdit, 0, Qt::AlignCenter);
    layout->addWidget(m_rootPasswordCheckEdit, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(grub_password_check_, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(next_button_, 0, Qt::AlignCenter);

    // username_edit_->setRightIcon(CAPS_LOCK_ICON);
    // hostname_edit_->setRightIcon();

    this->setLayout(layout);
    this->setContentsMargins(0, 0, 0, 0);
    this->setStyleSheet(ReadFile(":/styles/system_info_form_frame.css"));
}

bool SystemInfoFormFrame::validateUsername(QString& msg)
{
    const QString reserved_username_file = GetReservedUsernameFile();
    const int     min_len = GetSettingsInt(kSystemInfoUsernameMinLen);
    const int     max_len = GetSettingsInt(kSystemInfoUsernameMaxLen);
    const ValidateUsernameState state = ValidateUsername(
        username_edit_->text(), reserved_username_file, min_len, max_len);
    switch (state) {
        case ValidateUsernameState::ReservedError: {
            msg = tr("This username already exists");
            return false;
        }
        case ValidateUsernameState::FirstCharError: {
            msg = tr("The first letter must be in lowercase");
            return false;
        }
        case ValidateUsernameState::EmptyError:  // fall through
        case ValidateUsernameState::InvalidCharError: {
            msg =
                tr("Username must contain English letters (lowercase), "
                   "numbers or special symbols (_-)");
            return false;
        }
        case ValidateUsernameState::TooLongError:  // fall through
        case ValidateUsernameState::TooShortError: {
            msg = tr("Please input username longer than %1 characters and "
                     "shorter than %2 characters")
                      .arg(min_len)
                      .arg(max_len);
            return false;
        }
        case ValidateUsernameState::Ok: {
            // Pass
            break;
        }
    }
    return true;
}

bool SystemInfoFormFrame::validateHostname(QString& msg)
{
    const QStringList reserved =
        GetSettingsStringList(kSystemInfoHostnameReserved);
    const ValidateHostnameState state =
        ValidateHostname(hostname_edit_->text(), reserved);
    switch (state) {
        case ValidateHostnameState::EmptyError: {
            msg = tr("Please input computer name");
            return false;
        }
        case ValidateHostnameState::InvalidChar: {
            msg = tr("Computer name is invalid");
            return false;
        }
        case ValidateHostnameState::ReservedError: {
            msg = tr("Computer name already exists, please input another one");
            return false;
        }
        case ValidateHostnameState::TooLongError:  // fall through
        case ValidateHostnameState::TooShortError: {
            msg = tr("Please input computer name longer than %1 characters and "
                     "shorter than %2 characters")
                      .arg(kHostnameMinLen)
                      .arg(kHostnameMaxLen);
            return false;
        }
        case ValidateHostnameState::Ok: {
            // Pass
            break;
        }
    }
    return true;
}

bool SystemInfoFormFrame::validatePassword(LineEdit* passwordEdit, QString& msg)
{
#ifndef QT_DEBUG
    const bool strong_pwd_check = GetSettingsBool(kSystemInfoPasswordStrongCheck);
#else
    const bool strong_pwd_check = true;
#endif // !QT_DEBUG

    int min_len = 1;
    int max_len = 16;
    if (strong_pwd_check) {
        if (passwordEdit->text().toLower() == username_edit_->text().toLower()) {
            msg = tr("The password should be different from the username");
            return false;
        }
        min_len = GetSettingsInt(kSystemInfoPasswordMinLen);
        max_len = GetSettingsInt(kSystemInfoPasswordMaxLen);
    }

    const ValidatePasswordState state = ValidatePassword(
        passwordEdit->text(), min_len, max_len, strong_pwd_check);

    switch (state) {
        case ValidatePasswordState::EmptyError: {
            msg = tr("Please input password longer than %1 characters and "
                     "shorter than %2 characters")
                      .arg(min_len)
                      .arg(max_len);
            return false;
        }
        case ValidatePasswordState::StrongError: {  // fall through
            msg = tr(
                "The password must contain English letters (case-sensitive), numbers or special symbols (~!@#$%^&*()[]{}\\|/?,.<>)");
            return false;
        }
        case ValidatePasswordState::TooShortError:  // fall through
        case ValidatePasswordState::TooLongError: {
            msg = tr("Please input password longer than %1 characters and "
                     "shorter than %2 characters")
                      .arg(min_len)
                      .arg(max_len);
            return false;
        }
        case ValidatePasswordState::Ok: {
            // Pass
            break;
        }
        default:
            break;
    }
    return true;
}

void SystemInfoFormFrame::updateCapsLockState(bool capsLock)
{
    for (LineEdit* edit : m_editList) {
        edit->setCapsLockVisible(edit->hasFocus() && capsLock);
    }
}

bool SystemInfoFormFrame::validatePassword2(LineEdit* passwordEdit, LineEdit* passwordCheckEdit, QString& msg)
{
    if (passwordEdit->text() != passwordCheckEdit->text()) {
        msg = tr("Passwords do not match");
        return false;
    }
    else {
        return true;
    }
}

void SystemInfoFormFrame::systemInfoFrameFinish()
{
    tooltip_->hide();

    // save config
    WritePasswordStrong(GetSettingsBool(kSystemInfoPasswordStrongCheck));

    if (grub_password_check_->isChecked()) {
        QProcess process;
        process.setProgram("grub-mkpasswd-pbkdf2");
        process.start();
        const QString password{ password_edit_->text() };
        process.write(QString("%1\n%1\n").arg(password).toLatin1());
        process.closeWriteChannel();
        process.waitForFinished();

        const QString& result = process.readAllStandardOutput();

        QRegularExpression re("(?<=password is).*");
        auto               match = re.match(result);

        if (!match.isValid()) {
            qWarning() << "not match grub password !!!!!!";
        }

        WriteGrubPassword(match.captured(0).replace(" ", ""));
    }

    // Emit finished signal when all form inputs are ok.
    emit this->finished();
}

void SystemInfoFormFrame::onNextButtonClicked()
{
    QString msg;
    if (!validateUsername(msg)) {
        tooltip_->setText(msg);
        tooltip_->showBottom(username_edit_);
    }
    else if (!validateHostname(msg)) {
        tooltip_->setText(msg);
        tooltip_->showBottom(hostname_edit_);
    }
    else if (!validatePassword(password_edit_, msg)) {
        tooltip_->setText(msg);
        tooltip_->showBottom(password_edit_);
    }
    else if (!validatePassword2(password_edit_, password_check_edit_, msg)) {
        tooltip_->setText(msg);
        tooltip_->showBottom(password_check_edit_);
    }
    else if (m_setRootPasswordCheck->isChecked()) {
        if (!validatePassword(m_rootPasswordEdit, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_rootPasswordEdit);
        }
        else if (!validatePassword2(m_rootPasswordEdit, m_rootPasswordCheckEdit, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_rootPasswordCheckEdit);
        }
        else{
            systemInfoFrameFinish();
        }
    }
    else {
        systemInfoFrameFinish();
    }
}

void SystemInfoFormFrame::onEditingLineEdit()
{
    if (tooltip_->isVisible()) {
        tooltip_->hide();
    }
}

void SystemInfoFormFrame::onUsernameEdited()
{
    is_username_edited_ = true;
    if (!is_hostname_edited_manually_ &&
        !GetSettingsBool(kSystemInfoLockHostname)) {
        // Update hostname based on username.
        const QString username = username_edit_->text();
        if (username.isEmpty()) {
            hostname_edit_->setText("");
        }
        else {
            // Add suffix to username
            hostname_edit_->setText(
                username + GetSettingsString(kSystemInfoHostnameAutoSuffix));
        }
    }
}

void SystemInfoFormFrame::onUsernameEditingFinished()
{
    // When line-edit loses focus, validate it, and check its results.
    // If error occurs, popup tooltip frame.
    if (is_username_edited_) {
        is_username_edited_ = false;
        QString msg;
        if (!validateUsername(msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(username_edit_);
        }
    }
}

void SystemInfoFormFrame::onHostnameEdited()
{
    is_hostname_edited_          = true;
    is_hostname_edited_manually_ = true;
}

void SystemInfoFormFrame::onHostnameEditingFinished()
{
    if (is_hostname_edited_) {
        is_hostname_edited_ = false;
        QString msg;
        if (!validateHostname(msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(hostname_edit_);
        }
    }
}

void SystemInfoFormFrame::onPasswordEdited()
{
    is_password_edited_ = true;
}

void SystemInfoFormFrame::onPasswordEditingFinished()
{
    if (is_password_edited_) {
        is_password_edited_ = false;
        QString msg;
        if (!validatePassword(password_edit_, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(password_edit_);
        }
    }
}

void SystemInfoFormFrame::onPassword2Edited()
{
    is_password2_edited_ = true;
}

void SystemInfoFormFrame::onPassword2EditingFinished()
{
    if (is_password2_edited_) {
        is_password2_edited_ = false;
        QString msg;
        if (!validatePassword(password_edit_, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(password_edit_);
        }
        else if (!validatePassword2(password_edit_, password_check_edit_, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(password_check_edit_);
        }
    }
}

void SystemInfoFormFrame::onRootPasswordEdited()
{
    m_isRootPasswordEdited = true;
}

void SystemInfoFormFrame::onRootPasswordEditingFinished()
{
    if (m_isRootPasswordEdited) {
        m_isRootPasswordEdited = false;
        QString msg;
        if (!validatePassword(m_rootPasswordEdit, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_rootPasswordEdit);
        }
    }
}

void SystemInfoFormFrame::onRootPasswordCheckEdited()
{
    m_isRootPasswordCheckEdited = true;
}

void SystemInfoFormFrame::onRootPasswordCheckEditingFinished()
{
    if (m_isRootPasswordCheckEdited) {
        m_isRootPasswordCheckEdited = false;
        QString msg;
        if (!validatePassword(m_rootPasswordEdit, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_rootPasswordEdit);
        }
        else if (!validatePassword2(m_rootPasswordEdit, m_rootPasswordCheckEdit, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_rootPasswordCheckEdit);
        }
    }
}

void SystemInfoFormFrame::onSetRootPasswordCheckChanged(bool enable)
{
    if (enable) {
        m_rootPasswordEdit->setFocus();
        m_rootPasswordEdit->show();
        m_rootPasswordCheckEdit->show();
    }
    else {
        m_rootPasswordEdit->hide();
        m_rootPasswordCheckEdit->hide();

        if (tooltip_->isVisible()) {
            tooltip_->hide();
        }
    }
}

}  // namespace installer
