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
#include "ui/widgets/system_info_tip.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/di_scrollarea.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QProcess>
#include <vector>
#include <QCheckBox>
#include <QDebug>
#include <QPushButton>

namespace installer {

namespace {
const int kSetRootPasswordCheckBoxWidth = 310;
const int kSetRootPasswordCheckBoxHeight = 36;
}//namespace

class SystemInfoFormFramePrivate : public QObject
{
    Q_OBJECT
public:
    SystemInfoFormFramePrivate(SystemInfoFormFrame *qq) : q_ptr(qq){}
    SystemInfoFormFrame* q_ptr;
    Q_DECLARE_PUBLIC(SystemInfoFormFrame)

    void updateDevice();
private:
    void initConnections();
    void initUI();
    void updateTex();
    void initBoolvariable();

    // Validate line-edit. If failed, write tooltip to |msg| and returns false.
    bool validateUsername(QString& msg);
    bool validateHostname(QString& msg);
    bool validatePassword(LineEdit* passwordEdit, QString& msg);
    bool validatePassword2(LineEdit* passwordEdit, LineEdit* passwordCheckEdit, QString& msg);

    void updateCapsLockState(bool capslock);
    void systemInfoFrameFinish();

    // To mark whether content is edited by user.

    // Validate form content.
    void onNextButtonClicked();

    // Hide tooltip frame when line-edit is being edited.
    void onEditingLineEdit();

    // Automatically change hostname when username is changed by user.
    void onUsernameEdited();
    void onUsernameEditingFinished();
    void onHostnameEdited();
    void onHostnameEditingFinished();
    void onPasswordEdited();
    void onPasswordEditingFinished();
    void onPassword2Edited();
    void onPassword2EditingFinished();
    void onRootPasswordEdited();
    void onRootPasswordEditingFinished();
    void onRootPasswordCheckEdited();
    void onRootPasswordCheckEditingFinished();
    void onSetRootPasswordCheckChanged(bool enable);

    bool searchDevice();
private:
    bool m_isUsernameEdited_ = false;
    bool m_isHostnameEdited_ = false;
    // If hostname is edited by user, do not generate new hostname based on
    // current username.
    bool m_isHostnameEditedManually_ = false;
    bool m_isPasswordEdited_ = false;
    bool m_isPassword2Edited_ = false;
    bool m_isRootPasswordEdited = false;
    bool m_isRootPasswordCheckEdited =false;

    TitleLabel*   m_titleLabel_         = nullptr;
    CommentLabel* m_commentLabel_       = nullptr;
    AvatarButton* m_avatarButton_       = nullptr;
    LineEdit*     m_usernameEdit_       = nullptr;
    LineEdit*     m_hostnameEdit_       = nullptr;
    LineEdit*     m_passwordEdit_       = nullptr;
    LineEdit*     m_passwordCheckEdit_ = nullptr;
    QCheckBox*    m_grubPasswordCheck_ = nullptr;
    QCheckBox*    m_setRootPasswordCheck = nullptr;
    LineEdit*     m_rootPasswordEdit = nullptr;
    LineEdit*     m_rootPasswordCheckEdit = nullptr;

    // Display tooltip error message.
    SystemInfoTip*         tooltip_     = nullptr;
    QPushButton*           next_button_ = nullptr;

    std::vector<LineEdit*> m_editList;

};

SystemInfoFormFrame::SystemInfoFormFrame(QWidget* parent) : QFrame(parent)
  , d_private(new SystemInfoFormFramePrivate(this))
{
    this->setObjectName("system_info_form_frame");
    d_private->initBoolvariable();
    d_private->initUI();
    d_private->updateTex();
    d_private->initConnections();

    KeyboardMonitor::instance()->start();
}

SystemInfoFormFrame::~SystemInfoFormFrame()
{
}

void SystemInfoFormFrame::updateAvatar(const QString& avatar)
{
    Q_D(SystemInfoFormFrame);

    d->m_avatarButton_->updateIcon(avatar);
}

void SystemInfoFormFrame::readConf() {
    Q_D(SystemInfoFormFrame);

    d->m_usernameEdit_->setText(GetSettingsString(kSystemInfoDefaultUsername));
    d->m_hostnameEdit_->setText(GetSettingsString(kSystemInfoDefaultHostname));
    d->m_passwordEdit_->setText(GetSettingsString(kSystemInfoDefaultPassword));
    d->m_passwordCheckEdit_->setText(GetSettingsString(kSystemInfoDefaultPassword));
}

void SystemInfoFormFrame::writeConf()
{
    Q_D(SystemInfoFormFrame);

    WriteUsername(d->m_usernameEdit_->text());
    WriteHostname(d->m_hostnameEdit_->text());
    WritePassword(d->m_passwordEdit_->text());
    WriteRootPassword(GetSettingsBool(kSetRootPasswordFromUser)
                      ? d->m_rootPasswordEdit->text()
                      : d->m_passwordEdit_->text());
}

void SystemInfoFormFrame::changeEvent(QEvent* event)
{
    Q_D(SystemInfoFormFrame);

    if (event->type() == QEvent::LanguageChange) {


        d->m_usernameEdit_->setPlaceholderText(tr("Username"));
        d->m_hostnameEdit_->setPlaceholderText(tr("Computer name"));
        d->m_passwordEdit_->setPlaceholderText(tr("Password"));
        d->m_passwordCheckEdit_->setPlaceholderText(tr("Repeat password"));
        d->m_rootPasswordEdit->setPlaceholderText(tr("Root password"));
        d->m_rootPasswordCheckEdit->setPlaceholderText(tr("Repeat root password"));
        d->updateTex();
    }
    else {
        QFrame::changeEvent(event);
    }
}

void SystemInfoFormFrame::showEvent(QShowEvent* event)
{
    Q_D(SystemInfoFormFrame);

    QFrame::showEvent(event);
    d->m_usernameEdit_->setFocus();
    d->tooltip_->hide();
    d->updateDevice();

}

void SystemInfoFormFramePrivate::initConnections()
{
    Q_Q(SystemInfoFormFrame);

    connect(m_avatarButton_, &QPushButton::clicked, q,
            &SystemInfoFormFrame::avatarClicked);
    connect(next_button_, &QPushButton::clicked, this,
            &SystemInfoFormFramePrivate::onNextButtonClicked);

    connect(m_usernameEdit_, &LineEdit::editingFinished, this,
            &SystemInfoFormFramePrivate::onUsernameEditingFinished);
    connect(m_hostnameEdit_, &LineEdit::editingFinished, this,
            &SystemInfoFormFramePrivate::onHostnameEditingFinished);
    connect(m_passwordEdit_, &LineEdit::editingFinished, this,
            &SystemInfoFormFramePrivate::onPasswordEditingFinished);
    connect(m_passwordCheckEdit_, &LineEdit::editingFinished, this,
            &SystemInfoFormFramePrivate::onPassword2EditingFinished);
    connect(m_rootPasswordEdit, &LineEdit::editingFinished, this
            , &SystemInfoFormFramePrivate::onRootPasswordEditingFinished);
    connect(m_rootPasswordCheckEdit, &LineEdit::editingFinished, this
            , &SystemInfoFormFramePrivate::onRootPasswordCheckEditingFinished);

    connect(m_usernameEdit_, SIGNAL(returnPressed()), m_hostnameEdit_,
            SLOT(setFocus()));
    connect(m_hostnameEdit_, SIGNAL(returnPressed()), m_passwordEdit_,
            SLOT(setFocus()));
    connect(m_passwordEdit_, SIGNAL(returnPressed()), m_passwordCheckEdit_,
            SLOT(setFocus()));
    connect(m_passwordCheckEdit_, SIGNAL(returnPressed()), next_button_,
            SIGNAL(clicked()));
    connect(m_setRootPasswordCheck, &QCheckBox::clicked, this
            , &SystemInfoFormFramePrivate::onSetRootPasswordCheckChanged);
    connect(m_rootPasswordEdit, SIGNAL(returnPressed()), m_rootPasswordCheckEdit
            , SLOT(setFocus()));
    connect(m_rootPasswordCheckEdit, SIGNAL(returnPressed()), next_button_
            , SIGNAL(clicked()));

    QList<LineEdit*> list {
        m_usernameEdit_,
        m_hostnameEdit_,
        m_passwordEdit_,
        m_passwordCheckEdit_,
        m_rootPasswordEdit,
        m_rootPasswordCheckEdit
    };

    for (LineEdit* edit : list) {
        connect(edit, &LineEdit::textEdited, this,
                &SystemInfoFormFramePrivate::onEditingLineEdit);
        connect(edit, &LineEdit::gotFocus, this, [=] {
            updateCapsLockState(KeyboardMonitor::instance()->isCapslockOn());
        });
    }

    connect(m_usernameEdit_, &LineEdit::textEdited, this,
            &SystemInfoFormFramePrivate::onUsernameEdited);
    connect(m_hostnameEdit_, &LineEdit::textEdited, this,
            &SystemInfoFormFramePrivate::onHostnameEdited);
    connect(m_passwordEdit_, &LineEdit::textEdited, this,
            &SystemInfoFormFramePrivate::onPasswordEdited);
    connect(m_passwordCheckEdit_, &LineEdit::textEdited, this,
            &SystemInfoFormFramePrivate::onPassword2Edited);
    connect(m_rootPasswordEdit, &LineEdit::textEdited, this
            , &SystemInfoFormFramePrivate::onRootPasswordEdited);
    connect(m_rootPasswordCheckEdit, &LineEdit::textEdited, this
            , &SystemInfoFormFramePrivate::onRootPasswordCheckEdited);

    connect(KeyboardMonitor::instance(),
            &KeyboardMonitor::capslockStatusChanged, this,
            &SystemInfoFormFramePrivate::updateCapsLockState);
}

void SystemInfoFormFramePrivate::initUI()
{
    Q_Q(SystemInfoFormFrame);

    m_titleLabel_   = new TitleLabel("");
    m_commentLabel_ = new CommentLabel;
    m_avatarButton_ = new AvatarButton;

    m_usernameEdit_ = new LineEdit(":/images/username_12.svg");
    m_usernameEdit_->setReadOnly(GetSettingsBool(kSystemInfoLockUsername));

    QString str = GetSettingsString(kSystemInfoDefaultUsername);
    if (!str.isEmpty()) {
        m_usernameEdit_->setText(str);
    }
    else {
        m_usernameEdit_->setPlaceholderText(tr("Username"));
    }

    m_hostnameEdit_ = new LineEdit(":/images/username_12.svg");
    m_hostnameEdit_->setReadOnly(GetSettingsBool(kSystemInfoLockHostname));

    str = GetSettingsString(kSystemInfoDefaultHostname);
    if (!str.isEmpty()) {
        m_hostnameEdit_->setText(str);
    }
    else {
        m_hostnameEdit_->setPlaceholderText(tr("Computer name"));
    }

    m_passwordEdit_ = new LineEdit(":/images/username_12.svg");
    m_passwordEdit_->setEchoMode(QLineEdit::Password);
    m_passwordEdit_->setReadOnly(GetSettingsBool(kSystemInfoLockPassword));

    m_passwordCheckEdit_ = new LineEdit(":/images/password_12.svg");
    m_passwordCheckEdit_->setEchoMode(QLineEdit::Password);
    m_passwordCheckEdit_->setReadOnly(m_passwordEdit_->isReadOnly());

    str = GetSettingsString(kSystemInfoDefaultPassword);
    if (!str.isEmpty()) {
        m_passwordEdit_->setText(str);
        m_passwordCheckEdit_->setText(str);
    }
    else {
        m_passwordEdit_->setPlaceholderText(tr("Password"));
        m_passwordCheckEdit_->setPlaceholderText(tr("Repeat password"));
    }

    m_setRootPasswordCheck = new QCheckBox;
    m_setRootPasswordCheck->setCheckable(true);
    m_setRootPasswordCheck->setChecked(false);
    m_setRootPasswordCheck->setObjectName("RootPasswordCheckBox");
    m_setRootPasswordCheck->setVisible(GetSettingsBool(kSetRootPasswordFromUser));

    m_rootPasswordEdit = new LineEdit(":/images/username_12.svg");
    m_rootPasswordEdit->setPlaceholderText(tr("Root password"));
    m_rootPasswordEdit->setEchoMode(QLineEdit::Password);
    m_rootPasswordEdit->setReadOnly(GetSettingsBool(kSystemInfoLockPassword));
    m_rootPasswordEdit->hide();

    m_rootPasswordCheckEdit = new LineEdit(":/images/username_12.svg");
    m_rootPasswordCheckEdit->setPlaceholderText(tr("Repeat root password"));
    m_rootPasswordCheckEdit->setEchoMode(QLineEdit::Password);
    m_rootPasswordCheckEdit->setReadOnly(m_rootPasswordEdit->isReadOnly());
    m_rootPasswordCheckEdit->hide();

    m_editList.push_back(m_usernameEdit_);
    m_editList.push_back(m_hostnameEdit_);
    m_editList.push_back(m_passwordEdit_);
    m_editList.push_back(m_passwordCheckEdit_);
    m_editList.push_back(m_rootPasswordEdit);
    m_editList.push_back(m_rootPasswordCheckEdit);

    m_grubPasswordCheck_ = new QCheckBox;
    m_grubPasswordCheck_->setCheckable(true);
    m_grubPasswordCheck_->setChecked(false);
    m_grubPasswordCheck_->setObjectName("GrubPasswordCheckBox");
    m_grubPasswordCheck_->setVisible(GetSettingsBool(kSystemInfoEnableGrubEditPwd));

    next_button_ = new QPushButton;

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kMainLayoutSpacing);
    layout->addWidget(m_usernameEdit_, 0, Qt::AlignCenter);
    layout->addWidget(m_hostnameEdit_, 0, Qt::AlignCenter);
    layout->addWidget(m_passwordEdit_, 0, Qt::AlignCenter);
    layout->addWidget(m_passwordCheckEdit_, 0, Qt::AlignCenter);
    m_setRootPasswordCheck->setFixedSize(kSetRootPasswordCheckBoxWidth, kSetRootPasswordCheckBoxHeight);
    layout->addWidget(m_setRootPasswordCheck, 0, Qt::AlignCenter);
    layout->addWidget(m_rootPasswordEdit, 0, Qt::AlignCenter);
    layout->addWidget(m_rootPasswordCheckEdit, 0, Qt::AlignCenter);
    layout->addWidget(m_grubPasswordCheck_, 0, Qt::AlignCenter);

    QFrame* content = new QFrame;
    content->setAutoFillBackground(false);
    content->setAttribute(Qt::WA_TranslucentBackground);
    content->setLayout(layout);

    QScrollArea* area = new QScrollArea(q);
    area->setWidget(content);
    area->setAutoFillBackground(false);
    area->viewport()->setAutoFillBackground(false);
    area->setWidgetResizable(true);
    area->setFrameStyle(QScrollArea::NoFrame);
    area->setFixedWidth(kSetRootPasswordCheckBoxWidth + 20);
    area->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    tooltip_ = new SystemInfoTip(content);
    tooltip_->hide();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(kMainLayoutSpacing);
    mainLayout->addStretch();
    mainLayout->addWidget(m_titleLabel_, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_commentLabel_, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_avatarButton_, 0, Qt::AlignHCenter);
    mainLayout->addStretch();
    mainLayout->addWidget(area, 0, Qt::AlignCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(next_button_, 0, Qt::AlignCenter);

    q->setLayout(mainLayout);
    q->setContentsMargins(0, 0, 0, 0);
    q->setStyleSheet(ReadFile(":/styles/system_info_form_frame.css"));

}

void SystemInfoFormFramePrivate::updateTex()
{
    m_titleLabel_->setText(tr("Create User Account"));
    m_commentLabel_->setText(tr("Fill in the username, computer name and your password"));
    m_grubPasswordCheck_->setText(tr("Use that password to edit boot menu"));
    m_setRootPasswordCheck->setText(tr("Set root password"));
    next_button_->setText(tr("Next"));
    tooltip_->setText("");
}

void SystemInfoFormFramePrivate::initBoolvariable()
{
    m_isUsernameEdited_ = false;
    m_isHostnameEdited_ = false;
    m_isHostnameEditedManually_= false;
    m_isPasswordEdited_ = false;
    m_isPassword2Edited_ = false;
    m_isRootPasswordEdited = false;
    m_isRootPasswordCheckEdited = false;
}

bool SystemInfoFormFramePrivate::validateUsername(QString& msg)
{
    const QString reserved_username_file = GetReservedUsernameFile();
    const int     min_len = GetSettingsInt(kSystemInfoUsernameMinLen);
    const int     max_len = GetSettingsInt(kSystemInfoUsernameMaxLen);
    const ValidateUsernameState state = ValidateUsername(
                m_usernameEdit_->text(), reserved_username_file, min_len, max_len);
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

bool SystemInfoFormFramePrivate::validateHostname(QString& msg)
{
    const QStringList reserved =
            GetSettingsStringList(kSystemInfoHostnameReserved);
    const ValidateHostnameState state =
            ValidateHostname(m_hostnameEdit_->text(), reserved);
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

bool SystemInfoFormFramePrivate::validatePassword(LineEdit* passwordEdit, QString& msg)
{
#ifndef QT_DEBUG
    const bool strong_pwd_check = GetSettingsBool(kSystemInfoPasswordStrongCheck);
#else
    const bool strong_pwd_check = true;
#endif // !QT_DEBUG

    int min_len = 1;
    int max_len = 16;

    if (strong_pwd_check) {
        if (passwordEdit->text().toLower() == m_usernameEdit_->text().toLower()) {
            msg = tr("The password should be different from the username");
            return false;
        }
        min_len = GetSettingsInt(kSystemInfoPasswordMinLen);
        max_len = GetSettingsInt(kSystemInfoPasswordMaxLen);
    }

    const QStringList validate = GetSettingsStringList(kSystemInfoPasswordValidate);
    const int         required_num{ GetSettingsInt(kSystemInfoPasswordValidateRequired) };
    ValidatePasswordState state = ValidatePassword(
        passwordEdit->text(), min_len, max_len, strong_pwd_check, validate, required_num);

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

void SystemInfoFormFramePrivate::updateCapsLockState(bool capsLock)
{
    for (LineEdit* edit : m_editList) {
        edit->setCapsLockVisible(edit->hasFocus() && capsLock);
    }
}

bool SystemInfoFormFramePrivate::validatePassword2(LineEdit* passwordEdit, LineEdit* passwordCheckEdit, QString& msg)
{
    if (passwordEdit->text() != passwordCheckEdit->text()) {
        msg = tr("Passwords do not match");
        return false;
    }
    else {
        return true;
    }
}

void SystemInfoFormFramePrivate::systemInfoFrameFinish()
{
    Q_Q(SystemInfoFormFrame);

    tooltip_->hide();

    // save config
    WritePasswordStrong(GetSettingsBool(kSystemInfoPasswordStrongCheck));

    if (m_grubPasswordCheck_->isChecked()) {
        QProcess process;
        process.setProgram("grub-mkpasswd-pbkdf2");
        process.start();
        const QString password{ m_passwordEdit_->text() };
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
    emit q->finished();
}

void SystemInfoFormFramePrivate::onNextButtonClicked()
{
    QString msg;
    if (!validateUsername(msg)) {
        tooltip_->setText(msg);
        tooltip_->showBottom(m_usernameEdit_);
    }
    else if (!validateHostname(msg)) {
        tooltip_->setText(msg);
        tooltip_->showBottom(m_hostnameEdit_);
    }
    else if (!validatePassword(m_passwordEdit_, msg)) {
        tooltip_->setText(msg);
        tooltip_->showBottom(m_passwordEdit_);
    }
    else if (!validatePassword2(m_passwordEdit_, m_passwordCheckEdit_, msg)) {
        tooltip_->setText(msg);
        tooltip_->showBottom(m_passwordCheckEdit_);
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

void SystemInfoFormFramePrivate::onEditingLineEdit()
{
    if (tooltip_->isVisible()) {
        tooltip_->hide();
    }
}

void SystemInfoFormFramePrivate::onUsernameEdited()
{
    m_isUsernameEdited_ = true;
    if (!m_isHostnameEditedManually_ &&
            !GetSettingsBool(kSystemInfoLockHostname)) {
        // Update hostname based on username.
        const QString username = m_usernameEdit_->text();
        if (username.isEmpty()) {
            m_hostnameEdit_->setText("");
        }
        else {
            // Add suffix to username
            m_hostnameEdit_->setText(
                        username + GetSettingsString(kSystemInfoHostnameAutoSuffix));
        }
    }
}

void SystemInfoFormFramePrivate::onUsernameEditingFinished()
{
    // When line-edit loses focus, validate it, and check its results.
    // If error occurs, popup tooltip frame.
    if (m_isUsernameEdited_) {
        m_isUsernameEdited_ = false;
        QString msg;
        if (!validateUsername(msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_usernameEdit_);
        }
    }
}

void SystemInfoFormFramePrivate::onHostnameEdited()
{
    m_isHostnameEdited_          = true;
    m_isHostnameEditedManually_ = true;
}

void SystemInfoFormFramePrivate::onHostnameEditingFinished()
{
    if (m_isHostnameEdited_) {
        m_isHostnameEdited_ = false;
        QString msg;
        if (!validateHostname(msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_hostnameEdit_);
        }
    }
}

void SystemInfoFormFramePrivate::onPasswordEdited()
{
    m_isPasswordEdited_ = true;
}

void SystemInfoFormFramePrivate::onPasswordEditingFinished()
{
    if (m_isPasswordEdited_) {
        m_isPasswordEdited_ = false;
        QString msg;
        if (!validatePassword(m_passwordEdit_, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_passwordEdit_);
        }
    }
}

void SystemInfoFormFramePrivate::onPassword2Edited()
{
    m_isPassword2Edited_ = true;
}

void SystemInfoFormFramePrivate::onPassword2EditingFinished()
{
    if (m_isPassword2Edited_) {
        m_isPassword2Edited_ = false;
        QString msg;
        if (!validatePassword(m_passwordEdit_, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_passwordEdit_);
        }
        else if (!validatePassword2(m_passwordEdit_, m_passwordCheckEdit_, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_passwordCheckEdit_);
        }
    }
}

void SystemInfoFormFramePrivate::onRootPasswordEdited()
{
    m_isRootPasswordEdited = true;
}

void SystemInfoFormFramePrivate::onRootPasswordEditingFinished()
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

void SystemInfoFormFramePrivate::onRootPasswordCheckEdited()
{
    m_isRootPasswordCheckEdited = true;
}

void SystemInfoFormFramePrivate::onRootPasswordCheckEditingFinished()
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

void SystemInfoFormFramePrivate::onSetRootPasswordCheckChanged(bool enable)
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

bool SystemInfoFormFramePrivate::searchDevice() {
    QDBusInterface fingerprintInterface("com.deepin.daemon.Authenticate",
                                                                "/com/deepin/daemon/Authenticate/Fingerprint",
                                                                "com.deepin.daemon.Authenticate.Fingerprint",
                                                                QDBusConnection::systemBus());
    QVariant  defaultDevice(fingerprintInterface.property("DefaultDevice"));
    if (defaultDevice.type() != QVariant::Type::String) return false;
    QString deviceName(defaultDevice.toString());

    return !deviceName.isEmpty();
}

void SystemInfoFormFramePrivate::updateDevice() {
    if (searchDevice()) {
       tooltip_->setText(tr("Your PC supports fingerprint identification, so you can add fingerprint password in Control Center > Account, and then use the fingerprint to unlock and authenticate"));
       tooltip_->showBottom(m_passwordEdit_);
    }
}

}  // namespace installer

#include "system_info_form_frame.moc"
