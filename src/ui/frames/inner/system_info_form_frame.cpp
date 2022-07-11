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
#include "service/password_manager.h"
#include "sysinfo/validate_hostname.h"
#include "sysinfo/validate_username.h"
#include "ui/frames/consts.h"
#include "ui/utils/keyboardmonitor.h"
#include "ui/widgets/avatar_button.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/line_edit.h"
#include "ui/widgets/system_info_tip.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/di_scrollarea.h"
#include "ui/widgets/caps_lock_line_edit.h"

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
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

namespace installer {

namespace {
const int kSetRootPasswordCheckBoxWidth = 520;
const int kSetRootPasswordCheckBoxHeight = 20;

const int kMainWindowWidth = 520;
const int kHintLabelWidth = 150;
const int kInputWidgetWidth = 300;

const int kLevelWidgetWidth = 60;
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
    void updatePasswdLevel();
    void updateTex();
    void initBoolvariable();

    // Validate line-edit. If failed, write tooltip to |msg| and returns false.
    bool validateUsername(QString& msg);
    bool validateHostname(QString& msg);
    bool validatePassword(const QString &user, const QString &passwd, QString& msg);
    bool validatePassword2(DPasswordEdit* passwordEdit, DPasswordEdit* passwordCheckEdit, QString& msg);

    void updateCapsLockState();
//    void updateCapsLockState(bool capslock);
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
    QFrame *m_rootUserFrame = nullptr;
    QLabel *m_rootUserLabel = nullptr;
    QLabel *m_stretchLabel = nullptr;

    // Display tooltip error message.
    SystemInfoTip*         tooltip_     = nullptr;

    std::vector<DLineEdit*> m_editList;

    // 密码等级显示控件
    QLabel* m_passwdLevelLabel = nullptr;
    QLabel* m_rootPasswdLevelLabel = nullptr;
};

SystemInfoFormFrame::SystemInfoFormFrame(QWidget* parent) : QFrame(parent)
  , d_private(new SystemInfoFormFramePrivate(this))
{
    this->setObjectName("system_info_form_frame");
    d_private->initBoolvariable();
    d_private->initUI();
    d_private->updateTex();
    d_private->initConnections();

    Q_D(SystemInfoFormFrame);
    this->setFocusProxy(d->m_avatarButton_);

    KeyboardMonitor::instance()->start();
}

SystemInfoFormFrame::~SystemInfoFormFrame()
{
}

bool SystemInfoFormFrame::validateUserInfo()
{
    Q_D(SystemInfoFormFrame);
    d->tooltip_->hide();

    bool reset = false;
    QString msg;
    if (!d->validateUsername(msg)) {
        d->tooltip_->setText(msg);
        d->tooltip_->showBottom(d->m_usernameEdit);
    }
    else if (!d->validateHostname(msg)) {
        d->tooltip_->setText(msg);
        d->tooltip_->showBottom(d->m_hostnameEdit);
    }
    else if (!d->validatePassword(d->m_usernameEdit->text(), d->m_passwordEdit->text(), msg)) {
        d->tooltip_->setText(msg);
        d->tooltip_->showBottom(d->m_passwordEdit);
    }
    else if (!d->validatePassword2(d->m_passwordEdit, d->m_passwordCheckEdit, msg)) {
        d->tooltip_->setText(msg);
        d->tooltip_->showBottom(d->m_passwordCheckEdit);
    }
    else if (d->m_setRootPasswordCheck->isChecked()) {
        if (!d->validatePassword("root", d->m_rootPasswordEdit->text(), msg)) {
            d->tooltip_->setText(msg);
            d->tooltip_->showBottom(d->m_rootPasswordEdit);
        }
        else if (!d->validatePassword2(d->m_rootPasswordEdit, d->m_rootPasswordCheckEdit, msg)) {
            d->tooltip_->setText(msg);
            d->tooltip_->showBottom(d->m_rootPasswordCheckEdit);
        }
        else{
            reset = true;
        }
    }
    else {
        reset = true;
    }

//    if (reset) {
//        // 处理密码强度为低时的确认处理逻辑，目前创建用户界面不涉及回退动作，故只确认一次
//        static bool repeatFlag = true;
//        if (repeatFlag && PwqualityManager::instance()->passwdLevel(d->m_passwordEdit->text())
//                == PasswdLevel::LowerLevel) {
//            d->tooltip_->setText(tr("Password strength is low, please press the next button again to confirm."));
//            d->tooltip_->showBottom(d->m_passwordEdit);
//            repeatFlag = false;
//            return false;
//        }

//        return reset;
//    }

    return reset;
}

void SystemInfoFormFrame::checkNextButtonEnable()
{
    Q_D(SystemInfoFormFrame);

    if (d->m_usernameEdit->text().isEmpty()
        || d->m_hostnameEdit->text().isEmpty()
        || d->m_passwordEdit->text().isEmpty()
        || d->m_passwordCheckEdit->text().isEmpty()) {
        emit requestNextButtonEnable(false);
        return;
    }

    if (d->m_setRootPasswordCheck->isChecked()) {
        if (d->m_rootPasswordEdit->text().isEmpty()
            || d->m_rootPasswordCheckEdit->text().isEmpty()) {
            emit requestNextButtonEnable(false);
            return;
        }
    }

    emit requestNextButtonEnable(true);
}

bool SystemInfoFormFrame::focusSwitch()
{
    Q_D(SystemInfoFormFrame);
    if(d->m_avatarButton_->hasFocus()) {
        d->m_usernameEdit->lineEdit()->setFocus();
    } else if (d->m_usernameEdit->lineEdit()->hasFocus()) {
        d->m_hostnameEdit->lineEdit()->setFocus();
    } else if (d->m_hostnameEdit->lineEdit()->hasFocus()) {
        d->m_passwordEdit->lineEdit()->setFocus();
    } else if (d->m_passwordEdit->lineEdit()->hasFocus()) {
        d->m_passwordCheckEdit->lineEdit()->setFocus();
    } else if (d->m_passwordCheckEdit->lineEdit()->hasFocus()) {
        if (d->m_grubPasswordCheck_->isVisible()) {
            d->m_grubPasswordCheck_->setFocus();
        } else if (d->m_setRootPasswordCheck->isVisible()) {
            d->m_setRootPasswordCheck->setFocus();
        } else {
            d->m_passwordCheckEdit->lineEdit()->clearFocus();
            return false;
        }
    } else if (d->m_grubPasswordCheck_->hasFocus()) {
            d->m_grubPasswordCheck_->clearFocus();
            return false;
    } else if (d->m_setRootPasswordCheck->hasFocus()) {
        if (d->m_rootPasswordEdit->lineEdit()->isVisible()) {
            d->m_rootPasswordEdit->lineEdit()->setFocus();
        } else {
            d->m_setRootPasswordCheck->clearFocus();
            return false;
        }
    } else if (d->m_rootPasswordEdit->lineEdit()->hasFocus()) {
        d->m_rootPasswordCheckEdit->lineEdit()->setFocus();
    } else if (d->m_rootPasswordCheckEdit->hasFocus()) {
        d->m_rootPasswordCheckEdit->clearFocus();
        return false;
    } else {
        d->m_avatarButton_->setFocus();
    }

    return true;
}

bool SystemInfoFormFrame::doSpace()
{
    Q_D(SystemInfoFormFrame);
    if (d->m_grubPasswordCheck_->hasFocus()) {
        d->m_grubPasswordCheck_->setChecked(!d->m_grubPasswordCheck_->isChecked());
    } else if (d->m_setRootPasswordCheck->hasFocus()) {
        d->m_setRootPasswordCheck->setChecked(!d->m_setRootPasswordCheck->isChecked());
        d->onSetRootPasswordCheckChanged(d->m_setRootPasswordCheck->isChecked());
    }
    return true;
}

bool SystemInfoFormFrame::doSelect()
{
    return true;
}

bool SystemInfoFormFrame::directionKey(int keyvalue)
{
    Q_D(SystemInfoFormFrame);
    if(d->m_avatarButton_->hasFocus()) {
        d->m_avatarButton_->directionKey(keyvalue);
    }
    return true;
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
    WritePasswdLevel(PasswordManager::instance()->passwdLevel(d->m_usernameEdit->text(), d->m_passwordEdit->text()));

    if (d->m_setRootPasswordCheck->isChecked()) {
        WriteRootPassword(d->m_rootPasswordEdit->text());
    } else {
        WriteRootPassword("");
    }
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
    d->m_avatarButton_->setFocus();
    d->tooltip_->hide();
    d->updateDevice();

    d_private->updateCapsLockState();
}

bool SystemInfoFormFrame::eventFilter(QObject *watched, QEvent *event) {

    Q_D(SystemInfoFormFrame);
    if (event->type()== QEvent::FocusIn || event->type() == QEvent::FocusOut){
        d->updateCapsLockState();
    }

    return QFrame::eventFilter(watched, event);
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
        edit->lineEdit()->installEventFilter(q);
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
    usernameLayout->addStretch();
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
    hostnameLayout->addStretch();
    QFrame *hostnameFrame = new QFrame;
    hostnameFrame->setLayout(hostnameLayout);
    m_hostnameLabel->setFixedWidth(kHintLabelWidth);
    m_hostnameEdit->setFixedWidth(kInputWidgetWidth);
    hostnameFrame->setFixedWidth(kMainWindowWidth);

    // 密码等级空间初始化
    m_passwdLevelLabel = new QLabel;
    m_passwdLevelLabel->setFixedWidth(kLevelWidgetWidth);

    m_rootPasswdLevelLabel = new QLabel;
    m_rootPasswdLevelLabel->setFixedWidth(kLevelWidgetWidth);

    m_passwordLabel = new QLabel;
    m_passwordLabel->setAlignment(Qt::AlignLeft);
    m_passwordEdit = new DPasswordEdit;
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->lineEdit()->setReadOnly(GetSettingsBool(kSystemInfoLockPassword));
    m_passwordEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordEdit->layout()->setSpacing(7);

    QHBoxLayout *passwordLayout = new QHBoxLayout;
    passwordLayout->setContentsMargins(0, 0, 0, 0);
    passwordLayout->setSpacing(0);
    passwordLayout->addWidget(m_passwordLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    passwordLayout->addWidget(m_passwordEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    passwordLayout->addWidget(m_passwdLevelLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
    QFrame *passwordFrame = new QFrame;
    passwordFrame->setLayout(passwordLayout);
    m_passwordLabel->setFixedWidth(kHintLabelWidth);
    m_passwordEdit->setFixedWidth(kInputWidgetWidth);
    passwordFrame->setFixedWidth(kMainWindowWidth);

    m_passwordCheckLabel = new QLabel;
    m_passwordCheckLabel->setAlignment(Qt::AlignLeft);
    m_passwordCheckEdit = new DPasswordEdit;
    m_passwordCheckEdit->setEchoMode(QLineEdit::Password);
    m_passwordCheckEdit->lineEdit()->setReadOnly(m_passwordEdit->lineEdit()->isReadOnly());
    m_passwordCheckEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordCheckEdit->layout()->setSpacing(7);

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
    passwordCheckLayout->addStretch();
    QFrame *passwordCheckFrame = new QFrame;
    passwordCheckFrame->setLayout(passwordCheckLayout);
    m_passwordCheckLabel->setFixedWidth(kHintLabelWidth);
    m_passwordCheckEdit->setFixedWidth(kInputWidgetWidth);
    passwordCheckFrame->setFixedWidth(kMainWindowWidth);

    m_setRootPasswordCheck = new QCheckBox;
    m_setRootPasswordCheck->setObjectName("RootPasswordCheckBox");
    m_setRootPasswordCheck->setVisible(GetSettingsBool(kSetRootPasswordFromUser));
    //m_setRootPasswordCheck->setFocusPolicy(Qt::TabFocus);

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
    rootPasswordLayout->addWidget(m_rootPasswdLevelLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
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
    rootPasswordCheckLayout->addStretch();
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
    //m_grubPasswordCheck_->setFocusPolicy(Qt::TabFocus);

    m_rootUserLabel = new QLabel;
    m_rootUserLabel->setFixedWidth(kHintLabelWidth);

    DLineEdit* rootUser = new DLineEdit;
    rootUser->setEnabled(false);
    rootUser->setFixedWidth(kInputWidgetWidth);
    rootUser->setContextMenuPolicy(Qt::NoContextMenu);
    rootUser->lineEdit()->setPlaceholderText("root");

    QHBoxLayout *rootUserFrameLayout = new QHBoxLayout;
    rootUserFrameLayout->setContentsMargins(0, 0, 0, 0);
    rootUserFrameLayout->setSpacing(0);
    rootUserFrameLayout->addWidget(m_rootUserLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    rootUserFrameLayout->addWidget(rootUser, 0, Qt::AlignLeft | Qt::AlignVCenter);
    rootUserFrameLayout->addStretch();

    m_rootUserFrame= new QFrame;
    m_rootUserFrame->setLayout(rootUserFrameLayout);
    m_rootUserFrame->setFixedWidth(kMainWindowWidth);
    m_rootUserFrame->hide();

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

    if (m_setRootPasswordCheck->isVisible()) {
        layout->addWidget(m_setRootPasswordCheck, 0, Qt::AlignHCenter);
        layout->addSpacing(kSetRootPasswordCheckBoxHeight);
        layout->addWidget(m_rootUserFrame, 0, Qt::AlignHCenter);
        layout->addSpacing(kSetRootPasswordCheckBoxHeight);
        layout->addWidget(m_rootPasswordFrame, 0, Qt::AlignHCenter);
        layout->addSpacing(kSetRootPasswordCheckBoxHeight);
        layout->addWidget(m_rootPasswordCheckFrame, 0, Qt::AlignHCenter);
        layout->addSpacing(kSetRootPasswordCheckBoxHeight);
        layout->addWidget(m_grubPasswordCheck_, 0, Qt::AlignHCenter);
        layout->addSpacing(kSetRootPasswordCheckBoxHeight);
    } else {
        layout->addSpacing(kSetRootPasswordCheckBoxHeight * 6);
    }

    QFrame* content = new QFrame;
    content->setLayout(layout);

    QScrollArea* area = new QScrollArea(q);
    area->setWidget(content);
    area->setWidgetResizable(true);
    area->setFrameStyle(QScrollArea::NoFrame);
    area->setFixedWidth(kMainWindowWidth + 10);
    area->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    area->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    area->setContentsMargins(0, 0, 0, 0);

    tooltip_ = new SystemInfoTip(content);
    tooltip_->hide();

    m_stretchLabel = new CommentLabel;
    m_stretchLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(kMainLayoutSpacing);
    mainLayout->addStretch();
    mainLayout->addWidget(m_titleLabel_, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_commentLabel_, 0, Qt::AlignHCenter | Qt::AlignTop);
    mainLayout->addWidget(m_stretchLabel, 0, Qt::AlignHCenter | Qt::AlignTop);
    m_stretchLabel->hide();
    mainLayout->addWidget(m_avatarButton_, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(area, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);

    q->setLayout(mainLayout);
    q->setContentsMargins(0, 0, 0, 0);

    updateTex();
}

void SystemInfoFormFramePrivate::updatePasswdLevel()
{
    QMap<PasswdLevel, QString> mapPasswdLevel = {
        {LowerLevel, QObject::tr("Weak")},
        {MediumLevel, QObject::tr("Medium")},
        {HigherLevel, QObject::tr("Strong")}};

    QMap<PasswdLevel, QColor> mapPasswdLevelColor = {
        {LowerLevel, QColor("#ff5736")},        // 红色
        {MediumLevel, QColor("#f7a11b")},       // 橙色
        {HigherLevel, QColor("#3ec500")}};      // 绿色

    if (!m_passwordEdit->text().isEmpty()) {
        QPalette palette;
        palette.setColor(QPalette::Text, mapPasswdLevelColor[PasswordManager::instance()->passwdLevel(m_usernameEdit->text(), m_passwordEdit->text())]);
        m_passwdLevelLabel->setPalette(palette);
        m_passwdLevelLabel->setForegroundRole(QPalette::Text);
        m_passwdLevelLabel->setText(mapPasswdLevel[PasswordManager::instance()->passwdLevel(m_usernameEdit->text(), m_passwordEdit->text())]);
    } else {
        m_passwdLevelLabel->setText("");
    }

    if (!m_rootPasswordEdit->text().isEmpty()) {
        QPalette palette;
        palette.setColor(QPalette::Text, mapPasswdLevelColor[PasswordManager::instance()->passwdLevel("root", m_rootPasswordEdit->text())]);
        m_rootPasswdLevelLabel->setPalette(palette);
        m_rootPasswdLevelLabel->setForegroundRole(QPalette::Text);
        m_rootPasswdLevelLabel->setText(mapPasswdLevel[PasswordManager::instance()->passwdLevel("root", m_rootPasswordEdit->text())]);

    } else {
        m_rootPasswdLevelLabel->setText("");
    }
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

    m_rootUserLabel->setText(::QObject::tr("Username").append(" :"));

    m_titleLabel_->setText(::QObject::tr("Create Accounts"));
    m_commentLabel_->setText(::QObject::tr("Fill in the username, computer name and your password"));
    m_stretchLabel->setText(::QObject::tr("Add fingerprint password in Control Center > Accounts to unlock and authenticate"));
    m_grubPasswordCheck_->setText(::QObject::tr("Use that password to edit boot menu"));
    m_setRootPasswordCheck->setText(::QObject::tr("Enable root user"));
    tooltip_->setText("");

    updatePasswdLevel();
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
    const int     min_len = GetSettingsInt(kSystemInfoUsernameMinLen);
    const int     max_len = GetSettingsInt(kSystemInfoUsernameMaxLen);
    const ValidateUsernameState state = ValidateUsername(m_usernameEdit->text(), min_len, max_len);
    switch (state) {
    case ValidateUsernameState::ReservedError: {
        msg = ::QObject::tr("This username already exists");
        return false;
    }
    case ValidateUsernameState::FirstCharError: {
        msg = ::QObject::tr("The first letter must be in lowercase");
        return false;
    }
    case ValidateUsernameState::EmptyError:  // fall through
    case ValidateUsernameState::Digital:
    case ValidateUsernameState::InvalidCharError: {
        msg = ::QObject::tr("Username must start with letters or numbers, only contain letters, numbers, dashes (-) and underscores (_), and cannot use numbers alone");
        return false;
    }
    case ValidateUsernameState::TooLongError:  // fall through
    case ValidateUsernameState::TooShortError: {
        msg = ::QObject::tr("Username must be between %1 and %2 characters")
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
            ValidateHostname(m_hostnameEdit->text(), reserved);
    switch (state) {
    case ValidateHostnameState::EmptyError: {
        msg = ::QObject::tr("Please input computer name");
        return false;
    }
    case ValidateHostnameState::InvalidChar: {
        msg = ::QObject::tr("Computer name can only contain letters, numbers, and dashes (-), and cannot start or end with a dash (-)");
        return false;
    }
    case ValidateHostnameState::ReservedError: {
        msg = ::QObject::tr("Computer name already exists, please input another one");
        return false;
    }
    case ValidateHostnameState::TooLongError:  // fall through
    case ValidateHostnameState::TooShortError: {
        msg = ::QObject::tr("Please input a computer name longer than %1 characters and "
                 "shorter than %2 characters")
                .arg(kHostnameMinLen)
                .arg(kHostnameMaxLen);
        return false;
    }
    case ValidateHostnameState::LabelTooLongError: {
        msg = ::QObject::tr("Please input a computer name longer than %1 characters and "
                 "shorter than %2 characters")
                .arg(kHostnameMinLen)
                .arg(LabelMaxLen);
        return false;
    }
    case ValidateHostnameState::Ok: {
        // Pass
        break;
    }
    }

    return true;
}

bool SystemInfoFormFramePrivate::validatePassword(const QString &user,
                                                  const QString &passwd,
                                                  QString& msg)
{
    return PasswordManager::instance()->checked(user, passwd, msg);
}

void SystemInfoFormFramePrivate::updateCapsLockState()
{
    Q_Q(SystemInfoFormFrame);

    for (DLineEdit* edit : m_editList) {
        QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(q->focusWidget());
        if (lineEdit == edit->lineEdit()) {
            if (KeyboardMonitor::instance()->isCapslockOn()) {
                edit->lineEdit()->addAction(m_capsLock, QLineEdit::TrailingPosition);
            } else {
                edit->lineEdit()->removeAction(m_capsLock);
            }
        } else {
            edit->lineEdit()->removeAction(m_capsLock);
        }
    }

}

//void SystemInfoFormFramePrivate::updateCapsLockState(bool capsLock)
//{
//    for (DLineEdit* edit : m_editList) {
//        QLineEdit *lineEdit = edit->lineEdit();
//        if (capsLock) {
//            lineEdit->addAction(m_capsLock, QLineEdit::TrailingPosition);
//        } else {
//            lineEdit->removeAction(m_capsLock);
//        }
//        lineEdit->update();
//    }
//}

bool SystemInfoFormFramePrivate::validatePassword2(DPasswordEdit *passwordEdit, DPasswordEdit *passwordCheckEdit, QString& msg)
{
    if (passwordEdit->text() != passwordCheckEdit->text()) {
        msg = ::QObject::tr("Passwords do not match");
        return false;
    }
    else {
        return true;
    }
}

void SystemInfoFormFramePrivate::systemInfoFrameFinish()
{
//    Q_Q(SystemInfoFormFrame);

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
    qDebug() << "onEditingLineEdit";
    if (tooltip_->isVisible()) {
        tooltip_->hide();
    }

    Q_Q(SystemInfoFormFrame);
    q->checkNextButtonEnable();
}

void SystemInfoFormFramePrivate::onUsernameEdited()
{
    m_isUsernameEdited_ = true;
    updatePasswdLevel();
    if (!m_isHostnameEditedManually_ &&
            !GetSettingsBool(kSystemInfoLockHostname)) {
        // Update hostname based on username.
        QString username = m_usernameEdit->text();
        if (username.isEmpty()) {
            m_hostnameEdit->setText("");
        }
        else {
            // Add suffix to username
            m_hostnameEdit->setText(
                        username.replace("_","") + GetSettingsString(kSystemInfoHostnameAutoSuffix));
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
    updatePasswdLevel();
}

void SystemInfoFormFramePrivate::onPasswordEditingFinished()
{
    if (m_isPasswordEdited_) {
        m_isPasswordEdited_ = false;
        QString msg;
        if (!validatePassword(m_usernameEdit->text(), m_passwordEdit->text(), msg)) {
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
        if (!validatePassword(m_usernameEdit->text(), m_passwordEdit->text(), msg)) {
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
    updatePasswdLevel();
}

void SystemInfoFormFramePrivate::onRootPasswordEditingFinished()
{
    if (m_isRootPasswordEdited) {
        m_isRootPasswordEdited = false;
        QString msg;
        if (!validatePassword("root", m_rootPasswordEdit->text(), msg)) {
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
        if (!validatePassword("root", m_rootPasswordEdit->text(), msg)) {
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
        m_rootUserFrame->show();
    }
    else {
        m_rootPasswordFrame->hide();
        m_rootPasswordCheckFrame->hide();
        m_rootUserFrame->hide();

        if (tooltip_->isVisible()) {
            tooltip_->hide();
        }
    }

    Q_Q(SystemInfoFormFrame);
    q->checkNextButtonEnable();
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

#ifdef QT_DEBUG
    const bool test = true;
#else
    const bool test = false;
#endif // QT_DEBUG
    if (test || searchDevice()) {
        m_stretchLabel->show();
        m_commentLabel_->hide();
    }
    else {
        m_stretchLabel->hide();
        m_commentLabel_->show();
    }
}

}  // namespace installer

#include "system_info_form_frame.moc"
