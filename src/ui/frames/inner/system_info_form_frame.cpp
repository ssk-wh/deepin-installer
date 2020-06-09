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
#include "ui/frames/inner/system_info_avatar_frame.h"

#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "service/pwquality_manager.h"
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
#include <QScrollArea>
#include <QScrollBar>
#include <DLineEdit>
#include <DPasswordEdit>
#include <QAction>
#include <QDBusInterface>

DWIDGET_USE_NAMESPACE

namespace installer {

namespace {
const int kSetRootPasswordCheckBoxWidth = 520;
const int kSetRootPasswordCheckBoxHeight = 36;

const int kMainWindowWidth = 520;
const int kHintLabelWidth = 140;
const int kInputWidgetWidth = 340;
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
    bool validatePassword(DPasswordEdit* passwordEdit, QString& msg);
    bool validatePassword2(DPasswordEdit* passwordEdit, DPasswordEdit* passwordCheckEdit, QString& msg);

    void updateCapsLockState(bool capslock);
    void systemInfoFrameFinish();

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
    SystemInfoAvatarFrame* m_avatarButton_       = nullptr;
    AvatarButton *m_currentAvatarButton;
    QLabel* m_usernameLabel = nullptr;
    DLineEdit*     m_usernameEdit       = nullptr;
    QLabel* m_hostnameLabel = nullptr;
    DLineEdit*     m_hostnameEdit       = nullptr;
    QLabel* m_passwordLabel = nullptr;
    DPasswordEdit*     m_passwordEdit       = nullptr;
    QLabel* m_passwordCheckLabel = nullptr;
    DPasswordEdit*     m_passwordCheckEdit = nullptr;
    QCheckBox*    m_grubPasswordCheck_ = nullptr;
    QCheckBox*    m_setRootPasswordCheck = nullptr;
    QLabel* m_rootPasswordLabel = nullptr;
    DPasswordEdit*     m_rootPasswordEdit = nullptr;
    QLabel* m_rootPasswordCheckLabel = nullptr;
    DPasswordEdit*     m_rootPasswordCheckEdit = nullptr;
    QAction *m_capsLock = nullptr;

    QFrame *m_rootPasswordFrame = nullptr;
    QFrame *m_rootPasswordCheckFrame = nullptr;

    // Display tooltip error message.
    SystemInfoTip*         tooltip_     = nullptr;

    std::vector<DLineEdit*> m_editList;

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

bool SystemInfoFormFrame::validateUserInfo()
{
    Q_D(SystemInfoFormFrame);

    QString msg;
    if (!d->validateUsername(msg)) {
        d->tooltip_->setText(msg);
        d->tooltip_->showBottom(d->m_usernameEdit);
    }
    else if (!d->validateHostname(msg)) {
        d->tooltip_->setText(msg);
        d->tooltip_->showBottom(d->m_hostnameEdit);
    }
    else if (!d->validatePassword(d->m_passwordEdit, msg)) {
        d->tooltip_->setText(msg);
        d->tooltip_->showBottom(d->m_passwordEdit);
    }
    else if (!d->validatePassword2(d->m_passwordEdit, d->m_passwordCheckEdit, msg)) {
        d->tooltip_->setText(msg);
        d->tooltip_->showBottom(d->m_passwordCheckEdit);
    }
    else if (d->m_setRootPasswordCheck->isChecked()) {
        if (!d->validatePassword(d->m_rootPasswordEdit, msg)) {
            d->tooltip_->setText(msg);
            d->tooltip_->showBottom(d->m_rootPasswordEdit);
        }
        else if (!d->validatePassword2(d->m_rootPasswordEdit, d->m_rootPasswordCheckEdit, msg)) {
            d->tooltip_->setText(msg);
            d->tooltip_->showBottom(d->m_rootPasswordCheckEdit);
        }
        else{
            return true;
        }
    }
    else {
        return true;
    }

    return false;
}

void SystemInfoFormFrame::updateAvatar(const QString& avatar)
{
    Q_D(SystemInfoFormFrame);

     d->m_currentAvatarButton->updateIcon(avatar);
}

void SystemInfoFormFrame::readConf() {
    Q_D(SystemInfoFormFrame);

    d->m_avatarButton_->readConf();
    d->m_currentAvatarButton->updateIcon(GetSettingsString(kSystemInfoDefaultAvator));
    d->m_usernameEdit->setText(GetSettingsString(kSystemInfoDefaultUsername));
    d->m_hostnameEdit->setText(GetSettingsString(kSystemInfoDefaultHostname));
    d->m_passwordEdit->setText(GetSettingsString(kSystemInfoDefaultPassword));
    d->m_passwordCheckEdit->setText(GetSettingsString(kSystemInfoDefaultPassword));
}

void SystemInfoFormFrame::writeConf()
{
    Q_D(SystemInfoFormFrame);

    d->systemInfoFrameFinish();

    WriteAvatar(d->m_currentAvatarButton->avatar());
    WriteUsername(d->m_usernameEdit->text());
    WriteHostname(d->m_hostnameEdit->text());
    WritePassword(d->m_passwordEdit->text());
    WriteRootPassword(GetSettingsBool(kSetRootPasswordFromUser)
                      ? d->m_rootPasswordEdit->text()
                      : d->m_passwordEdit->text());
}

void SystemInfoFormFrame::changeEvent(QEvent* event)
{
    Q_D(SystemInfoFormFrame);

    if (event->type() == QEvent::LanguageChange) {
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
    d->m_usernameEdit->setFocus();
    d->tooltip_->hide();

    d_private->updateCapsLockState(KeyboardMonitor::instance()->isCapslockOn());
}

void SystemInfoFormFramePrivate::initConnections()
{
    Q_Q(SystemInfoFormFrame);

    connect(m_usernameEdit, &DLineEdit::editingFinished, this,
            &SystemInfoFormFramePrivate::onUsernameEditingFinished);
    connect(m_hostnameEdit, &DLineEdit::editingFinished, this,
            &SystemInfoFormFramePrivate::onHostnameEditingFinished);
    connect(m_passwordEdit, &DLineEdit::editingFinished, this,
            &SystemInfoFormFramePrivate::onPasswordEditingFinished);
    connect(m_passwordCheckEdit, &DLineEdit::editingFinished, this,
            &SystemInfoFormFramePrivate::onPassword2EditingFinished);
    connect(m_rootPasswordEdit, &DLineEdit::editingFinished, this
            , &SystemInfoFormFramePrivate::onRootPasswordEditingFinished);
    connect(m_rootPasswordCheckEdit, &DLineEdit::editingFinished, this
            , &SystemInfoFormFramePrivate::onRootPasswordCheckEditingFinished);

    connect(m_usernameEdit, SIGNAL(returnPressed()), m_hostnameEdit,
            SLOT(setFocus()));
    connect(m_hostnameEdit, SIGNAL(returnPressed()), m_passwordEdit,
            SLOT(setFocus()));
    connect(m_passwordEdit, SIGNAL(returnPressed()), m_passwordCheckEdit,
            SLOT(setFocus()));
    connect(m_passwordCheckEdit, &DPasswordEdit::returnPressed, q, &SystemInfoFormFrame::systemInfoFormDone);
    connect(m_setRootPasswordCheck, &QCheckBox::clicked, this
            , &SystemInfoFormFramePrivate::onSetRootPasswordCheckChanged);
    connect(m_rootPasswordEdit, SIGNAL(returnPressed()), m_rootPasswordCheckEdit
            , SLOT(setFocus()));
    connect(m_rootPasswordCheckEdit, &DPasswordEdit::returnPressed, q, &SystemInfoFormFrame::systemInfoFormDone);


    QList<DLineEdit*> list {
        m_usernameEdit,
        m_hostnameEdit,
        m_passwordEdit,
        m_passwordCheckEdit,
        m_rootPasswordEdit,
        m_rootPasswordCheckEdit
    };

    for (DLineEdit* edit : list) {
        connect(edit, &DLineEdit::textEdited, this,
                &SystemInfoFormFramePrivate::onEditingLineEdit);
        // TODO(chenxiong)
        //        connect(edit, &DLineEdit::gotFocus, this, [=] {
        //            updateCapsLockState(KeyboardMonitor::instance()->isCapslockOn());
        //        });
    }

    connect(m_usernameEdit, &DLineEdit::textEdited, this,
            &SystemInfoFormFramePrivate::onUsernameEdited);
    connect(m_hostnameEdit, &DLineEdit::textEdited, this,
            &SystemInfoFormFramePrivate::onHostnameEdited);
    connect(m_passwordEdit, &DPasswordEdit::textEdited, this,
            &SystemInfoFormFramePrivate::onPasswordEdited);
    connect(m_passwordCheckEdit, &DPasswordEdit::textEdited, this,
            &SystemInfoFormFramePrivate::onPassword2Edited);
    connect(m_rootPasswordEdit, &DPasswordEdit::textEdited, this
            , &SystemInfoFormFramePrivate::onRootPasswordEdited);
    connect(m_rootPasswordCheckEdit, &DPasswordEdit::textEdited, this
            , &SystemInfoFormFramePrivate::onRootPasswordCheckEdited);

    connect(m_avatarButton_, &SystemInfoAvatarFrame::avatarUpdated, q, &SystemInfoFormFrame::updateAvatar);

    connect(KeyboardMonitor::instance(),
            &KeyboardMonitor::capslockStatusChanged, this,
            &SystemInfoFormFramePrivate::updateCapsLockState);
}

void SystemInfoFormFramePrivate::initUI()
{
    Q_Q(SystemInfoFormFrame);

    m_titleLabel_   = new TitleLabel("");
    m_commentLabel_ = new CommentLabel;
    m_commentLabel_->setAlignment(Qt::AlignCenter);

    m_avatarButton_ = new SystemInfoAvatarFrame;
    m_avatarButton_->setFixedWidth(kMainWindowWidth);

    m_currentAvatarButton = new AvatarButton(q);
    m_currentAvatarButton->hide();

    m_usernameLabel = new QLabel;
    m_usernameLabel->setAlignment(Qt::AlignLeft);
    m_usernameEdit = new DLineEdit;
    m_usernameEdit->lineEdit()->setReadOnly(GetSettingsBool(kSystemInfoLockUsername));
//    m_usernameEdit->setSpeechToTextEnabled(false);
//    m_usernameEdit->setTextToSpeechEnabled(false);
//    m_usernameEdit->setTextToTranslateEnabled(false);
    m_usernameEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QString str = GetSettingsString(kSystemInfoDefaultUsername);
    if (!str.isEmpty()) {
        m_usernameEdit->setText(str);
    }
    else {
        m_usernameEdit->lineEdit()->setPlaceholderText(::QObject::tr("Username"));
    }

    QHBoxLayout *usernameLayout = new QHBoxLayout;
    usernameLayout->setContentsMargins(0, 0, 0, 0);
    usernameLayout->setSpacing(0);
    usernameLayout->addWidget(m_usernameLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usernameLayout->addWidget(m_usernameEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    QFrame *usernameFrame = new QFrame;
    usernameFrame->setLayout(usernameLayout);
    m_usernameLabel->setFixedWidth(kHintLabelWidth);
    m_usernameEdit->setFixedWidth(kInputWidgetWidth);
    usernameFrame->setFixedWidth(kMainWindowWidth);

    m_hostnameLabel = new QLabel;
    m_hostnameLabel->setAlignment(Qt::AlignLeft);
    m_hostnameEdit = new DLineEdit;
    m_hostnameEdit->lineEdit()->setReadOnly(GetSettingsBool(kSystemInfoLockHostname));
//    m_hostnameEdit->setSpeechToTextEnabled(false);
//    m_hostnameEdit->setTextToSpeechEnabled(false);
//    m_hostnameEdit->setTextToTranslateEnabled(false);
    m_hostnameEdit->setContextMenuPolicy(Qt::NoContextMenu);

    str = GetSettingsString(kSystemInfoDefaultHostname);
    if (!str.isEmpty()) {
        m_hostnameEdit->setText(str);
    }
    else {
        m_hostnameEdit->lineEdit()->setPlaceholderText(::QObject::tr("Computer name"));
    }

    m_capsLock = new QAction();
    m_capsLock->setIcon(QIcon(":/images/capslock.svg"));

    QHBoxLayout *hostnameLayout = new QHBoxLayout;
    hostnameLayout->setContentsMargins(0, 0, 0, 0);
    hostnameLayout->setSpacing(0);
    hostnameLayout->addWidget(m_hostnameLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    hostnameLayout->addWidget(m_hostnameEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    QFrame *hostnameFrame = new QFrame;
    hostnameFrame->setLayout(hostnameLayout);
    m_hostnameLabel->setFixedWidth(kHintLabelWidth);
    m_hostnameEdit->setFixedWidth(kInputWidgetWidth);
    hostnameFrame->setFixedWidth(kMainWindowWidth);

    m_passwordLabel = new QLabel;
    m_passwordLabel->setAlignment(Qt::AlignLeft);
    m_passwordEdit = new DPasswordEdit;
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->lineEdit()->setReadOnly(GetSettingsBool(kSystemInfoLockPassword));
    m_passwordEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QHBoxLayout *passwordLayout = new QHBoxLayout;
    passwordLayout->setContentsMargins(0, 0, 0, 0);
    passwordLayout->setSpacing(0);
    passwordLayout->addWidget(m_passwordLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    passwordLayout->addWidget(m_passwordEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    QFrame *passwordFrame = new QFrame;
    passwordFrame->setLayout(passwordLayout);
    m_passwordLabel->setFixedWidth(kHintLabelWidth);
    m_passwordEdit->setFixedWidth(kInputWidgetWidth);
    passwordFrame->setFixedWidth(kMainWindowWidth);

    m_passwordCheckLabel = new QLabel;
    m_passwordCheckLabel->setAlignment(Qt::AlignLeft);
    m_passwordCheckEdit = new DPasswordEdit;;
    m_passwordCheckEdit->setEchoMode(QLineEdit::Password);
    m_passwordCheckEdit->lineEdit()->setReadOnly(m_passwordEdit->lineEdit()->isReadOnly());
    m_passwordCheckEdit->setContextMenuPolicy(Qt::NoContextMenu);

    str = GetSettingsString(kSystemInfoDefaultPassword);
    if (!str.isEmpty()) {
        m_passwordEdit->setText(str);
        m_passwordCheckEdit->setText(str);
    }
    else {
        m_passwordEdit->lineEdit()->setPlaceholderText(::QObject::tr("Password"));
        m_passwordCheckEdit->lineEdit()->setPlaceholderText(::QObject::tr("Repeat password"));
    }

    QHBoxLayout *passwordCheckLayout = new QHBoxLayout;
    passwordCheckLayout->setContentsMargins(0, 0, 0, 0);
    passwordCheckLayout->setSpacing(0);
    passwordCheckLayout->addWidget(m_passwordCheckLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    passwordCheckLayout->addWidget(m_passwordCheckEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    QFrame *passwordCheckFrame = new QFrame;
    passwordCheckFrame->setLayout(passwordCheckLayout);
    m_passwordCheckLabel->setFixedWidth(kHintLabelWidth);
    m_passwordCheckEdit->setFixedWidth(kInputWidgetWidth);
    passwordCheckFrame->setFixedWidth(kMainWindowWidth);

    m_setRootPasswordCheck = new QCheckBox;
    m_setRootPasswordCheck->setCheckable(true);
    m_setRootPasswordCheck->setChecked(false);
    m_setRootPasswordCheck->setObjectName("RootPasswordCheckBox");
    m_setRootPasswordCheck->setVisible(GetSettingsBool(kSetRootPasswordFromUser));

    m_rootPasswordLabel = new QLabel;
    m_rootPasswordLabel->setAlignment(Qt::AlignLeft);
    m_rootPasswordEdit = new DPasswordEdit;
    m_rootPasswordEdit->setEchoMode(QLineEdit::Password);
    m_rootPasswordEdit->lineEdit()->setReadOnly(GetSettingsBool(kSystemInfoLockPassword));
    m_rootPasswordEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QHBoxLayout *rootPasswordLayout = new QHBoxLayout;
    rootPasswordLayout->setContentsMargins(0, 0, 0, 0);
    rootPasswordLayout->setSpacing(0);
    rootPasswordLayout->addWidget(m_rootPasswordLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    rootPasswordLayout->addWidget(m_rootPasswordEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    m_rootPasswordFrame = new QFrame;
    m_rootPasswordFrame->setLayout(rootPasswordLayout);
    m_rootPasswordLabel->setFixedWidth(kHintLabelWidth);
    m_rootPasswordEdit->setFixedWidth(kInputWidgetWidth);
    m_rootPasswordFrame->setFixedWidth(kMainWindowWidth);
    QSizePolicy sp_retain = m_rootPasswordEdit->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    m_rootPasswordFrame->setSizePolicy(sp_retain);
    m_rootPasswordFrame->hide();

    m_rootPasswordCheckLabel = new QLabel;
    m_rootPasswordCheckLabel->setAlignment(Qt::AlignLeft);
    m_rootPasswordCheckEdit = new DPasswordEdit;
    m_rootPasswordCheckEdit->setEchoMode(QLineEdit::Password);
    m_rootPasswordCheckEdit->lineEdit()->setReadOnly(m_rootPasswordEdit->lineEdit()->isReadOnly());
    m_rootPasswordCheckEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QHBoxLayout *rootPasswordCheckLayout = new QHBoxLayout;
    rootPasswordCheckLayout->setContentsMargins(0, 0, 0, 0);
    rootPasswordCheckLayout->setSpacing(0);
    rootPasswordCheckLayout->addWidget(m_rootPasswordCheckLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    rootPasswordCheckLayout->addWidget(m_rootPasswordCheckEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    m_rootPasswordCheckFrame = new QFrame;
    m_rootPasswordCheckFrame->setLayout(rootPasswordCheckLayout);
    m_rootPasswordCheckLabel->setFixedWidth(kHintLabelWidth);
    m_rootPasswordCheckEdit->setFixedWidth(kInputWidgetWidth);
    m_rootPasswordCheckFrame->setFixedWidth(kMainWindowWidth);
    sp_retain = m_rootPasswordEdit->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    m_rootPasswordCheckFrame->setSizePolicy(sp_retain);
    m_rootPasswordCheckFrame->hide();

    m_editList.push_back(m_usernameEdit);
    m_editList.push_back(m_hostnameEdit);
    m_editList.push_back(m_passwordEdit);
    m_editList.push_back(m_passwordCheckEdit);
    m_editList.push_back(m_rootPasswordEdit);
    m_editList.push_back(m_rootPasswordCheckEdit);

    m_grubPasswordCheck_ = new QCheckBox;
    m_grubPasswordCheck_->setCheckable(true);
    m_grubPasswordCheck_->setChecked(false);
    m_grubPasswordCheck_->setObjectName("GrubPasswordCheckBox");
    m_grubPasswordCheck_->setVisible(GetSettingsBool(kSystemInfoEnableGrubEditPwd));

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kMainLayoutSpacing);
    layout->addWidget(usernameFrame, 0, Qt::AlignHCenter);
    layout->addSpacing(kSetRootPasswordCheckBoxHeight);
    layout->addWidget(hostnameFrame, 0, Qt::AlignHCenter);
    layout->addSpacing(kSetRootPasswordCheckBoxHeight);
    layout->addWidget(passwordFrame, 0, Qt::AlignHCenter);
    layout->addSpacing(kSetRootPasswordCheckBoxHeight);
    layout->addWidget(passwordCheckFrame, 0, Qt::AlignHCenter);
    layout->addSpacing(kSetRootPasswordCheckBoxHeight);
    m_setRootPasswordCheck->setFixedSize(kSetRootPasswordCheckBoxWidth, kSetRootPasswordCheckBoxHeight);
    layout->addWidget(m_setRootPasswordCheck, 0, Qt::AlignHCenter);
    layout->addSpacing(kSetRootPasswordCheckBoxHeight);
    layout->addWidget(m_rootPasswordFrame, 0, Qt::AlignHCenter);
    layout->addSpacing(kSetRootPasswordCheckBoxHeight);
    layout->addWidget(m_rootPasswordCheckFrame, 0, Qt::AlignHCenter);
    layout->addSpacing(kSetRootPasswordCheckBoxHeight);
    layout->addWidget(m_grubPasswordCheck_, 0, Qt::AlignHCenter);

    QFrame* content = new QFrame;
    content->setLayout(layout);

    QScrollArea* area = new QScrollArea(q);
    area->setWidget(content);
    area->setWidgetResizable(true);
    area->setFrameStyle(QScrollArea::NoFrame);
    area->setFixedWidth(kMainWindowWidth);
    area->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    area->setContentsMargins(0, 0, 0, 0);

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
    mainLayout->addWidget(area, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);

    q->setLayout(mainLayout);
    q->setContentsMargins(0, 0, 0, 0);

    updateTex();
}

void SystemInfoFormFramePrivate::updateTex()
{
    m_usernameEdit->lineEdit()->setPlaceholderText(::QObject::tr("Username"));
    m_hostnameEdit->lineEdit()->setPlaceholderText(::QObject::tr("Computer name"));
    m_passwordEdit->lineEdit()->setPlaceholderText(::QObject::tr("Password"));
    m_passwordCheckEdit->lineEdit()->setPlaceholderText(::QObject::tr("Repeat password"));
    m_rootPasswordEdit->lineEdit()->setPlaceholderText(::QObject::tr("Root password"));
    m_rootPasswordCheckEdit->lineEdit()->setPlaceholderText(::QObject::tr("Repeat root password"));

    m_usernameLabel->setText(::QObject::tr("Username").append(" :"));
    m_hostnameLabel->setText(::QObject::tr("Computer name").append(" :"));
    m_passwordLabel->setText(::QObject::tr("Password").append(" :"));
    m_passwordCheckLabel->setText(::QObject::tr("Repeat password").append(" :"));
    m_rootPasswordLabel->setText(::QObject::tr("Root password").append(" :"));
    m_rootPasswordCheckLabel->setText(::QObject::tr("Repeat root password").append(" :"));

    m_titleLabel_->setText(::QObject::tr("Create Accounts"));
    m_commentLabel_->setText(::QObject::tr("Fill in the username, computer name and your password"));
    m_grubPasswordCheck_->setText(::QObject::tr("Use that password to edit boot menu"));
    m_setRootPasswordCheck->setText(::QObject::tr("Set as root password"));
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
    const ValidateUsernameState state = ValidateUsername(m_usernameEdit->text()
                                                         , reserved_username_file, min_len, max_len);
    switch (state) {
    case ValidateUsernameState::ReservedError: {
        msg = ::QObject::tr("This username already exists");
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidateUsernameState::FirstCharError: {
        msg = ::QObject::tr("The first letter must be in lowercase");
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidateUsernameState::EmptyError:  // fall through
    case ValidateUsernameState::InvalidCharError: {
        msg = ::QObject::tr("Username must contain English letters (lowercase), "
                   "numbers or special symbols (_-)");
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidateUsernameState::TooLongError:  // fall through
    case ValidateUsernameState::TooShortError: {
        msg = ::QObject::tr("Please input username longer than %1 characters and "
                 "shorter than %2 characters")
                .arg(min_len)
                .arg(max_len);
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidateUsernameState::Ok: {
        // Pass
        break;
    }
    }
    emit q_ptr->requestNextButtonEnable(true);
    return true;
}

bool SystemInfoFormFramePrivate::validateHostname(QString& msg)
{
    const QStringList reserved =
            GetSettingsStringList(kSystemInfoHostnameReserved);
    const ValidateHostnameState state =
            ValidateHostname(m_hostnameEdit->text(), reserved);
    switch (state) {
    case ValidateHostnameState::EmptyError: {
        msg = ::QObject::tr("Please input computer name");
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidateHostnameState::InvalidChar: {
        msg = ::QObject::tr("Computer name is invalid");
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidateHostnameState::ReservedError: {
        msg = ::QObject::tr("Computer name already exists, please input another one");
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidateHostnameState::TooLongError:  // fall through
    case ValidateHostnameState::TooShortError: {
        msg = ::QObject::tr("Please input computer name longer than %1 characters and "
                 "shorter than %2 characters")
                .arg(kHostnameMinLen)
                .arg(kHostnameMaxLen);
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidateHostnameState::Ok: {
        // Pass
        break;
    }
    }
    emit q_ptr->requestNextButtonEnable(true);
    return true;
}

bool SystemInfoFormFramePrivate::validatePassword(DPasswordEdit *passwordEdit, QString& msg)
{
#ifndef QT_DEBUG
    const bool strong_pwd_check = GetSettingsBool(kSystemInfoPasswordStrongCheck);
#else
    const bool strong_pwd_check = true;
#endif // !QT_DEBUG

    int min_len = 1;
    int max_len = 16;

    if (strong_pwd_check) {
        if (passwordEdit->text().toLower() == m_usernameEdit->text().toLower()) {
            msg = ::QObject::tr("The password should be different from the username");
            emit q_ptr->requestNextButtonEnable(false);
            return false;
        }
        min_len = GetSettingsInt(kSystemInfoPasswordMinLen);
        max_len = GetSettingsInt(kSystemInfoPasswordMaxLen);
    }

    const QStringList validate = GetSettingsStringList(kSystemInfoPasswordValidate);
    const int required_num{ GetSettingsInt(kSystemInfoPasswordValidateRequired) };
    ValidatePasswordState state = ValidatePassword(
        passwordEdit->text(), min_len, max_len, strong_pwd_check, validate, required_num);

    switch (state) {
    case ValidatePasswordState::EmptyError: {
        msg = ::QObject::tr("The password cannot be empty​");
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidatePasswordState::StrongError: {  // fall through
        msg = ::QObject::tr("Password must contain letters, numbers and symbols");
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidatePasswordState::TooShortError:  // fall through
    case ValidatePasswordState::TooLongError: {
        msg = ::QObject::tr("Password must have at least 8 characters");
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    case ValidatePasswordState::Ok: {
        // Pass
        break;
    }
    default:
        break;
    }

    QString dict = PwqualityManager::instance()->dictChecked(passwordEdit->text());
    if (!dict.isEmpty()) {
        msg = ::QObject::tr("Password must not contain common words and combinations").arg(dict);
        return false;
    }

    QString palingrome = PwqualityManager::instance()->palindromeChecked(passwordEdit->text());
    if (!palingrome.isEmpty()) {
        msg = ::QObject::tr("Password must not contain more than 4 palindrome characters").arg(palingrome);
        return false;
    }

    return true;
}

void SystemInfoFormFramePrivate::updateCapsLockState(bool capsLock)
{
    for (DLineEdit* edit : m_editList) {
        QLineEdit *lineEdit = edit->lineEdit();
        if (capsLock) {
            lineEdit->addAction(m_capsLock, QLineEdit::TrailingPosition);
        } else {
            lineEdit->removeAction(m_capsLock);
        }
        lineEdit->update();
    }
}

bool SystemInfoFormFramePrivate::validatePassword2(DPasswordEdit *passwordEdit, DPasswordEdit *passwordCheckEdit, QString& msg)
{
    if (passwordEdit->text() != passwordCheckEdit->text()) {
        msg = ::QObject::tr("Passwords do not match");
        emit q_ptr->requestNextButtonEnable(false);
        return false;
    }
    else {
        emit q_ptr->requestNextButtonEnable(true);
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
        const QString password{ m_passwordEdit->text() };
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
        const QString username = m_usernameEdit->text();
        if (username.isEmpty()) {
            m_hostnameEdit->setText("");
        }
        else {
            // Add suffix to username
            m_hostnameEdit->setText(
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
            tooltip_->showBottom(m_usernameEdit);
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
            tooltip_->showBottom(m_hostnameEdit);
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
        if (!validatePassword(m_passwordEdit, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_passwordEdit);
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
        if (!validatePassword(m_passwordEdit, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_passwordEdit);
        }
        else if (!validatePassword2(m_passwordEdit, m_passwordCheckEdit, msg)) {
            tooltip_->setText(msg);
            tooltip_->showBottom(m_passwordCheckEdit);
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
        m_rootPasswordFrame->show();
        m_rootPasswordCheckFrame->show();
    }
    else {
        m_rootPasswordFrame->hide();
        m_rootPasswordCheckFrame->hide();

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
       tooltip_->setText(::QObject::tr("Add fingerprint password in Control Center > Accounts to unlock and authenticate"));
       tooltip_->showBottom(m_passwordEdit);      
    }
}

}  // namespace installer

#include "system_info_form_frame.moc"
