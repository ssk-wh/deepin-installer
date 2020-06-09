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

#include "create_root_user_frame.h"

#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_line_edit.h"
#include "sysinfo/validate_password.h"
#include "service/settings_name.h"
#include "service/settings_manager.h"

installer::CreateRootUserFramePrivate::CreateRootUserFramePrivate(CreateRootUserFrame *parent, int lines, int cols, int beginY, int beginX):
    FrameInterfacePrivate (nullptr, lines, cols, beginY, beginX),
    q_ptr(qobject_cast<CreateRootUserFrame*>(parent))
{
    initUI();
    updateTs();
    layout();
    initConnection();
}

bool installer::CreateRootUserFramePrivate::validate()
{
    QString msg;

    do {
        if (!validatePassword(m_rootPasswordLineEdit, msg)) {
            break;
        }
        if (!validatePassword2(m_rootPasswordLineEdit, m_passwordConfirmLineEdit, msg)) {
            break;
        }

        writeConf();

        return true;

    } while (false);

    showError(msg);

    return false;
}

void installer::CreateRootUserFramePrivate::show()
{
    m_errorInfo->hide();
    return FrameInterfacePrivate::show();
}

bool installer::CreateRootUserFramePrivate::validatePassword(NCursesLineEdit *passwordEdit, QString &msg)
{
    Q_Q(CreateRootUserFrame);

#ifndef QT_DEBUG
    const bool strong_pwd_check = GetSettingsBool(kSystemInfoPasswordStrongCheck);
#else
    const bool strong_pwd_check = true;
#endif // !QT_DEBUG

    int min_len = 1;
    int max_len = 16;

    if (strong_pwd_check) {
        if (passwordEdit->text().toLower() == q->m_userName.toLower()) {
            msg = ::QObject::tr("The password should be different from the username");
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
        msg = ::QObject::tr("Please input password longer than %1 characters and "
                 "shorter than %2 characters")
                .arg(min_len)
                .arg(max_len);
        return false;
    }
    case ValidatePasswordState::StrongError: {  // fall through
        msg = ::QObject::tr("The password must contain English letters (case-sensitive), numbers or special symbols (~!@#$%^&*()[]{}\\|/?,.<>)");
        return false;
    }
    case ValidatePasswordState::TooShortError:  // fall through
    case ValidatePasswordState::TooLongError: {
        msg = ::QObject::tr("Please input password longer than %1 characters and "
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


bool installer::CreateRootUserFramePrivate::validatePassword2(NCursesLineEdit *passwordEdit, NCursesLineEdit *passwordCheckEdit, QString &msg)
{
    if (passwordEdit->text() != passwordCheckEdit->text()) {
        msg = ::QObject::tr("Passwords do not match");
        return false;
     }
     else {
        return true;
    }
}

void installer::CreateRootUserFramePrivate::showError(const QString &text)
{
    m_errorInfo->setText(text);
    m_errorInfo->adjustSizeByContext();
    m_errorInfo->mvwin(m_errorInfo->begy(), begx() + (width() - m_errorInfo->width()) / 2);
    m_errorInfo->show();
}


void installer::CreateRootUserFramePrivate::initUI()
{
    try {
        FrameInterfacePrivate::initUI();

        m_labelTitle = new NcursesLabel(this, 1, 1, begy(), begx());
        m_labelTitle->setFocusEnabled(false);

        m_rootPasswordLabel = new NcursesLabel(this, 1, width() - 4, begy(), begx());
        m_rootPasswordLabel->setFocusEnabled(false);

        m_rootPasswordLineEdit = new NCursesLineEdit(this, 1, width() - 4, begy(), begx());
        m_rootPasswordLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
        m_rootPasswordLineEdit->setFocus(true);
        m_rootPasswordLineEdit->setEchoMode(true);

        m_passwordConfirmLabel = new NcursesLabel(this, 1, width() - 4, begy(), begx());
        m_passwordConfirmLabel->setFocusEnabled(false);

        m_passwordConfirmLineEdit = new NCursesLineEdit(this, 1, width() - 4, begy(), begx());
        m_passwordConfirmLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
        m_passwordConfirmLineEdit->setEchoMode(true);

        m_errorInfo = new NcursesLabel(this, 1, 1, begy(), begx());
        m_errorInfo->setBackground(NcursesUtil::getInstance()->error_attr());
        m_errorInfo->setFocusEnabled(false);

    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }
}

void installer::CreateRootUserFramePrivate::layout()
{
    try {
        int beginY = begy();
        int beginX = begx() + 2;
        m_labelTitle->mvwin(beginY, beginX + (width() - m_labelTitle->width()) / 2);
        beginY += m_labelTitle->height() + 1;

        m_rootPasswordLabel->mvwin(beginY, beginX);
        beginY += m_rootPasswordLabel->height();

        m_rootPasswordLineEdit->mvwin(beginY, beginX);
        beginY += m_rootPasswordLineEdit->height() + 1;

        m_passwordConfirmLabel->mvwin(beginY, beginX);
        beginY += m_passwordConfirmLabel->height();

        m_passwordConfirmLineEdit->mvwin(beginY, beginX);
        beginY += m_passwordConfirmLineEdit->height() + 1;

        m_errorInfo->mvwin(beginY, beginX);

    } catch (NCursesException& e) {
         qCritical() << QString(e.message);
    }
}

void installer::CreateRootUserFramePrivate::updateTs()
{
    m_labelTitle->setText(::QObject::tr("Create Root Account"));
    m_labelTitle->adjustSizeByContext();

    m_rootPasswordLabel->setText(::QObject::tr("Password:"));
    m_rootPasswordLabel->adjustSizeByContext();

    m_passwordConfirmLabel->setText(::QObject::tr("Repeat password") + ":");
    m_passwordConfirmLabel->adjustSizeByContext();

    return FrameInterfacePrivate::updateTs();
}

void installer::CreateRootUserFramePrivate::initConnection()
{
    auto creatErr = [=]{showError(QString());};
    connect(m_rootPasswordLineEdit, &NCursesLineEdit::textChanged, this, creatErr, Qt::QueuedConnection);
    connect(m_passwordConfirmLineEdit, &NCursesLineEdit::textChanged, this, creatErr, Qt::QueuedConnection);
}

void installer::CreateRootUserFramePrivate::readConf()
{
}

void installer::CreateRootUserFramePrivate::writeConf()
{
    Q_Q(CreateRootUserFrame);
    WriteRootPassword(GetSettingsBool(kSetRootPasswordFromUser) ?
                          m_passwordConfirmLineEdit->text():
                          q->m_userPassword);
}

installer::CreateRootUserFrame::CreateRootUserFrame(installer::FrameInterface *parent):
    FrameInterface(parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;

    m_private = new CreateRootUserFramePrivate (this, h, w, beginY, beginX);
}

bool installer::CreateRootUserFrame::shouldDisplay() const
{
    return m_display;
}

void installer::CreateRootUserFrame::setShoulDispaly(bool dispaly)
{
    m_display = dispaly;
}

void installer::CreateRootUserFrame::setUserName(const QString &name)
{
    m_userName = name;
}

void installer::CreateRootUserFrame::setUserPassword(const QString &password)
{
    m_userPassword = password;
}

QString installer::CreateRootUserFrame::getFrameName()
{
    return "Create Root";
}

bool installer::CreateRootUserFrame::handle()
{
    return true;
}
