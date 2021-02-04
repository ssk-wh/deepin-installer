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
#include "service/password_manager.h"

installer::CreateRootUserFramePrivate::CreateRootUserFramePrivate(CreateRootUserFrame *parent, int lines, int cols, int beginY, int beginX):
    FrameInterfacePrivate (nullptr, lines, cols, beginY, beginX),
    q_ptr(qobject_cast<CreateRootUserFrame*>(parent))
{
    initUI();
//    updateTs();
//    layout();
    initConnection();
}

bool installer::CreateRootUserFramePrivate::validate()
{
    QString msg;

    do {
        if (!validatePassword("root", m_rootPasswordLineEdit->text(), msg)) {
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
    if(!m_isshow) {
        updateTs();
        layout();
        m_errorInfo->hide();
        m_rootPasswordLineEdit->setFocus(true);
        return FrameInterfacePrivate::show();
    }
}

void installer::CreateRootUserFramePrivate::hide()
{
    FrameInterfacePrivate::hide();
    m_isshow = false;
}

bool installer::CreateRootUserFramePrivate::validatePassword(const QString &user, const QString &passwd, QString &msg)
{
    return PasswordManager::instance()->checked(user, passwd, msg);
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
        //m_labelTitle->hide();

        m_rootPasswordLabel = new NcursesLabel(this, 1, width() - 4, begy(), begx());
        m_rootPasswordLabel->setFocusEnabled(false);
        //m_rootPasswordLabel->hide();

        m_rootPasswordLineEdit = new NCursesLineEdit(this, 1, width() - 4, begy(), begx());
        m_rootPasswordLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
        ////m_rootPasswordLineEdit->setFocus(true);
        m_rootPasswordLineEdit->setEchoMode(true);
        //m_rootPasswordLineEdit->hide();

        m_passwordConfirmLabel = new NcursesLabel(this, 1, width() - 4, begy(), begx());
        m_passwordConfirmLabel->setFocusEnabled(false);
        //m_passwordConfirmLabel->hide();

        m_passwordConfirmLineEdit = new NCursesLineEdit(this, 1, width() - 4, begy(), begx());
        m_passwordConfirmLineEdit->setBackground(NcursesUtil::getInstance()->edit_attr());
        m_passwordConfirmLineEdit->setEchoMode(true);
        //m_passwordConfirmLineEdit->setFocusEnabled(false);
        //m_passwordConfirmLineEdit->hide();

        m_errorInfo = new NcursesLabel(this, 1, 1, begy(), begx());
        m_errorInfo->setBackground(NcursesUtil::getInstance()->error_attr());
        m_errorInfo->setFocusEnabled(false);
        //m_errorInfo->hide();

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
    //auto creatErr = [=]{showError(QString());};
    //connect(m_rootPasswordLineEdit, &NCursesLineEdit::textChanged, this, creatErr, Qt::QueuedConnection);
    //connect(m_passwordConfirmLineEdit, &NCursesLineEdit::textChanged, this, creatErr, Qt::QueuedConnection);
}

void installer::CreateRootUserFramePrivate::readConf()
{
}

void installer::CreateRootUserFramePrivate::writeConf()
{
    WriteRootPassword(m_rootPasswordLineEdit->text());
}

void installer::CreateRootUserFramePrivate::onKeyPress(int keyCode)
{
    switch (keyCode) {
    case KEY_TAB:
            switchChildWindowsFoucs();
        break;
    }

    qDebug()<< keyCode;
}

void installer::CreateRootUserFramePrivate::downHandle()
{
    if (m_rootPasswordLineEdit->isOnFoucs()) {
        m_rootPasswordLineEdit->setFocus(false);
        m_passwordConfirmLineEdit->setFocus(true);
    }
}

void installer::CreateRootUserFramePrivate::upHandle()
{
    if (m_passwordConfirmLineEdit->isOnFoucs()) {
        m_passwordConfirmLineEdit->setFocus(false);
        m_rootPasswordLineEdit->setFocus(true);
    }
}

installer::CreateRootUserFrame::CreateRootUserFrame(installer::FrameInterface *parent):
    FrameInterface(parent)
{
    int h = MAINWINDOW_HEIGHT;//LINES / 2;
    int w = MAINWINDOW_WIDTH;//COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;

    m_private = new CreateRootUserFramePrivate (this, h, w, beginY, beginX);
    //m_private->hide();
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
