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

#ifndef CREATE_ROOT_USER_FRAME_H
#define CREATE_ROOT_USER_FRAME_H

#include <QScopedPointer>

#include "ui/interfaces_cli/frameinterfaceprivate.h"
#include "ui/interfaces_cli/frameinterface.h"

namespace installer {

class CreateRootUserFrame;
class NcursesLabel;
class NCursesLineEdit;

class CreateRootUserFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
    CreateRootUserFramePrivate(CreateRootUserFrame *parent, int lines, int cols, int beginY, int beginX);

public:
    bool validate() override;
    void show() override;

protected:
    void initUI() override;
    void layout() override;
    void updateTs() override;
    void initConnection();

    void readConf();
    void writeConf();

    void onKeyPress(int keyCode) override;
    void downHandle() override;
    void upHandle() override;

private:
    bool validatePassword(NCursesLineEdit *passwordEdit, QString& msg);
    bool validatePassword2(NCursesLineEdit* passwordEdit, NCursesLineEdit* passwordCheckEdit, QString& msg);
    void showError(const QString &text);

private:
    NcursesLabel* m_labelTitle = nullptr;

    NcursesLabel*    m_rootPasswordLabel = nullptr;
    NCursesLineEdit* m_rootPasswordLineEdit = nullptr;

    NcursesLabel* m_passwordConfirmLabel = nullptr;
    NCursesLineEdit* m_passwordConfirmLineEdit;

    NcursesLabel* m_errorInfo = nullptr;

    CreateRootUserFrame *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(CreateRootUserFrame)
};

class CreateRootUserFrame  : public FrameInterface
{
    Q_OBJECT
    friend class CreateRootUserFramePrivate;

public:
    CreateRootUserFrame(FrameInterface* parent);

    bool shouldDisplay() const override;
    void setShoulDispaly(bool dispaly);
    void setUserName(const QString &name);
    void setUserPassword(const QString &password);

    QString getFrameName() override;

protected:
    bool handle() override;

    bool m_display = false;
    QString m_userName;
    QString m_userPassword;
};
}

#endif // CREATE_ROOT_USER_FRAME_H
