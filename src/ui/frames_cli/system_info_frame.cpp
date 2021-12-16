#include "system_info_frame.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "sysinfo/validate_hostname.h"
#include "sysinfo/validate_username.h"
#include "sysinfo/validate_password.h"
#include "service/password_manager.h"

namespace installer {



bool SystemInfoFramePrivate::validate()
{
    try {
        QString msg;
        do {
            if (!validateUsername(msg)) {
                break;
            }

            if (!validateHostname(msg)) {
                break;
            }

            if (!validatePassword(m_le_username->text(), m_le_password->text(), msg)) {
                break;
            }
            else if (!validatePassword2(m_le_password, m_le_password_confirm, msg)) {
                break;
            }
            showError(QString());

            writeConf();

            return true;
        } while (false);

        showError(msg);
        return false;


    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }

    return true;
}

void SystemInfoFramePrivate::show()
{
    if(!m_isshow) {
        updateTs();
        m_le_username->setFocus(true);
        FrameInterfacePrivate::show();
        m_isshow = true;
    }

    if (!GetSettingsBool(kSetRootPasswordFromUser)) {
        m_NcursesCheckBox->hide();
        m_NcursesCheckBox->setFocusEnabled(false);
    }
}

void SystemInfoFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void SystemInfoFramePrivate::onKeyPress(int keyCode)
{
    switch (keyCode) {
    case KEY_TAB:
            switchChildWindowsFoucs();
        break;
    }
    if (!m_isHostEdited && m_le_username->isOnFoucs()) {
        QString s_str = m_le_username->text().isEmpty() ? "" : QString("%1-PC").arg(m_le_username->text());
        m_le_hostname->setText(s_str);
    }
}

void SystemInfoFramePrivate::readConf()
{
    m_le_username->setText(GetSettingsString(kSystemInfoDefaultUsername));
    m_le_hostname->setText(GetSettingsString(kSystemInfoDefaultHostname));
    m_le_password->setText(GetSettingsString(kSystemInfoDefaultPassword));
    m_le_password_confirm->setText(GetSettingsString(kSystemInfoDefaultPassword));
}

void SystemInfoFramePrivate::writeConf()
{
    WriteUsername(m_le_username->text());
    WriteHostname(m_le_hostname->text());
    WritePassword(m_le_password->text());
    CustomSetSettingsBool(kSystemInfoSetupAfterReboot, true);

//    WriteRootPassword(GetSettingsBool(kSetRootPasswordFromUser)
//                     ? d->m_rootPasswordEdit->text()
    //                     : d->m_passwordEdit->text());
    if (!m_NcursesCheckBox->isSelect()) {
        WriteRootPassword("");
    }
}

void SystemInfoFramePrivate::downHandle()
{
    if (m_le_username->isOnFoucs()) {
        m_le_username->setFocus(false);
        m_le_hostname->setFocus(true);
    } else if (m_le_hostname->isOnFoucs()) {
        m_le_hostname->setFocus(false);
        m_le_password->setFocus(true);
    } else if (m_le_password->isOnFoucs()) {
        m_le_password->setFocus(false);
        m_le_password_confirm->setFocus(true);
    }
}

void SystemInfoFramePrivate::upHandle()
{
    if (m_le_password_confirm->isOnFoucs()) {
        m_le_password_confirm->setFocus(false);
        m_le_password->setFocus(true);
    } else if (m_le_password->isOnFoucs()) {
        m_le_password->setFocus(false);
        m_le_hostname->setFocus(true);
    } else if (m_le_hostname->isOnFoucs()) {
        m_le_hostname->setFocus(false);
        m_le_username->setFocus(true);
    }
}

void SystemInfoFramePrivate::switchChildWindowsFoucs()
{
    FrameInterfacePrivate::switchChildWindowsFoucs();
//    if (this->isOnFoucs()) {
//        m_le_password_confirm->setFocus(false);
//        m_le_password->setFocus(false);
//        m_le_hostname->setFocus(false);
//        m_le_username->setFocus(true);
//    }
}

bool SystemInfoFramePrivate::validateHostname(QString &msg)
{
    const QStringList reserved =
                GetSettingsStringList(kSystemInfoHostnameReserved);
        const ValidateHostnameState state =
                ValidateHostname(m_le_hostname->text(), reserved);
        switch (state) {
        case ValidateHostnameState::EmptyError: {
            msg = ::QObject::tr("Please input computer name");
            return false;
        }
        case ValidateHostnameState::InvalidChar: {
            msg = ::QObject::tr("Computer name is invalid");
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
        case ValidateHostnameState::Ok: {
            // Pass
            break;
        }
        }
        return true;
}

bool SystemInfoFramePrivate::validateUsername(QString &msg)
{
        const int min_len = GetSettingsInt(kSystemInfoUsernameMinLen);
        const int max_len = GetSettingsInt(kSystemInfoUsernameMaxLen);

        const ValidateUsernameState state = ValidateUsername(m_le_username->text(), min_len, max_len);
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

bool SystemInfoFramePrivate::validatePassword(const QString &user, const QString &passwd, QString &msg)
{
    return PasswordManager::instance()->checked(user, passwd, msg);
}


bool SystemInfoFramePrivate::validatePassword2(NCursesLineEdit *passwordEdit, NCursesLineEdit *passwordCheckEdit, QString &msg)
{
    if (passwordEdit->text() != passwordCheckEdit->text()) {
        msg = ::QObject::tr("Passwords do not match");
        return false;
     }
     else {
        return true;
    }
}

void SystemInfoFramePrivate::showError(const QString &msg)
{
    qDebug() << "showError = " << msg;
    if (msg.isEmpty()) {
        m_label_error_info->setText(msg);
    }
    if (m_label_error_info->text().isEmpty()) {
        m_label_error_info->setText(msg);
        if (msg.length() > width() - 2) {
            m_label_error_info->resize(msg.length() / (width() -2) + 1, width() -2);
            m_label_error_info->mvwin(m_label_error_info->begy(), begx() + 1);
        } else {
            m_label_error_info->adjustSizeByContext();
            m_label_error_info->mvwin(m_label_error_info->begy(), begx() + (width() - m_label_error_info->width()) / 2);
        }
    } else {
        m_label_error_info->setText(m_label_error_info->text());
    }

    m_label_error_info->show();
}

SystemInfoFramePrivate::SystemInfoFramePrivate(SystemInfoFrame *parent, int lines, int cols, int beginY, int beginX):
    FrameInterfacePrivate(nullptr, lines, cols, beginY, beginX),
    q_ptr(qobject_cast<SystemInfoFrame*>(parent))
{
    initUI();
    initConnection();
    //updateTs();
}

void SystemInfoFramePrivate::initUI()
{
    try {
        FrameInterfacePrivate::initUI();

        m_label_title = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_title->setFocusEnabled(false);
        //m_label_title->hide();

        m_label_instructions = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_instructions->setFocusEnabled(false);
        m_label_instructions->attron(NcursesUtil::getInstance()->comment_attr());
        //m_label_instructions->hide();

        m_label_username = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_username->setFocusEnabled(false);
        //m_label_username->hide();

        m_le_username = new NCursesLineEdit(this, 1, width() - 4, begy(), begx());
        m_le_username->setBackground(NcursesUtil::getInstance()->edit_attr());
        ////m_le_username->setFocus(true);
        ////m_le_username->setFocusEnabled(false);
        //m_le_username->hide();

        m_label_hostname = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_hostname->setFocusEnabled(false);
        //m_label_hostname->hide();

        m_le_hostname = new NCursesLineEdit(this, 1, width() - 4, begy(), begx());
        m_le_hostname->setBackground(NcursesUtil::getInstance()->edit_attr());
        ////m_le_hostname->setFocusEnabled(false);
        //m_le_hostname->hide();

        m_label_password = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_password->setFocusEnabled(false);
        //m_label_password->hide();

        m_le_password = new NCursesLineEdit(this, 1, width() - 4, begy(), begx());
        m_le_password->setBackground(NcursesUtil::getInstance()->edit_attr());
        m_le_password->setEchoMode(true);
        ////m_le_password->setFocusEnabled(false);
        //m_le_password->hide();

        m_label_password_confirm = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_password_confirm->setFocusEnabled(false);
        //m_label_password_confirm->hide();

        m_le_password_confirm = new NCursesLineEdit(this, 1, width() - 4, begy(), begx());
        m_le_password_confirm->setBackground(NcursesUtil::getInstance()->edit_attr());
        m_le_password_confirm->setEchoMode(true);
        ////m_le_password_confirm->setFocusEnabled(false);
        //m_le_password_confirm->hide();

        ////m_foucsWindows.append(this);

        m_NcursesCheckBox = new NcursesCheckBox(this, 1, (width() - 5) / 2, begy(), begx());
        m_NcursesCheckBox->setIsUseTitle(false);
        //m_NcursesCheckBox->hide();

        m_label_error_info = new NcursesLabel(this, 1, width() - 4, begy(), begx());
        m_label_error_info->setFocusEnabled(false);
        m_label_error_info->setBackground(NcursesUtil::getInstance()->error_attr());
        //m_label_error_info->hide();

        m_pNextButton->setBackground(NcursesUtil::getInstance()->button());
        //this->setFocus(true);

////        connect(m_le_username, &NCursesLineEdit::textChanged, this, [=](){
////            m_le_hostname->setText(m_le_username->text().append("-PC"));
////        });
    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }

}

void SystemInfoFramePrivate::layout()
{
    try {
        int beginY = begy();
        int beginX = begx() + 2;
        m_label_title->adjustSizeByContext();
        m_label_title->mvwin(beginY, begx() + (width() - m_label_title->width()) / 2);
        beginY += m_label_title->height();

        m_label_instructions->adjustSizeByContext();
        m_label_instructions->mvwin(beginY, begx() + (width() - m_label_instructions->width()) / 2);
        beginY += m_label_instructions->height();

        m_label_username->adjustSizeByContext();
        m_label_username->mvwin(beginY, beginX);
        beginY += m_label_username->height();

        m_le_username->mvwin(beginY, beginX);
        beginY += m_le_username->height() + 1;

        m_label_hostname->adjustSizeByContext();
        m_label_hostname->mvwin(beginY, beginX);
        beginY += m_label_hostname->height();

        m_le_hostname->mvwin(beginY, beginX);
        beginY += m_le_hostname->height() + 1;

        m_label_password->adjustSizeByContext();
        m_label_password->mvwin(beginY, beginX);
        beginY += m_label_password->height();

        m_le_password->mvwin(beginY, beginX);
        beginY += m_le_password->height() + 1;

        m_label_password_confirm->adjustSizeByContext();
        m_label_password_confirm->mvwin(beginY, beginX);
        beginY += m_label_password_confirm->height();

        m_le_password_confirm->mvwin(beginY, beginX);
        beginY += m_le_password_confirm->height() + 1;

        m_NcursesCheckBox->mvwin(beginY, beginX);
        beginY += m_NcursesCheckBox->height() + 1;

        m_label_error_info->mvwin(beginY, beginX);

    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }
}

void SystemInfoFramePrivate::initConnection()
{
    Q_Q(SystemInfoFrame);

    connect(m_NcursesCheckBox, &NcursesCheckBox::signal_SelectChange, q, &SystemInfoFrame::createRoot);
    connect(m_le_username, &NCursesLineEdit::textChanged, q, &SystemInfoFrame::userName);
    connect(m_le_password, &NCursesLineEdit::textChanged, q, &SystemInfoFrame::userPassword);
    connect(m_le_hostname, &NCursesLineEdit::editChanged, this, [=]{m_isHostEdited = true;});

    connect(m_le_username, &NCursesLineEdit::outFoucs, [this]{
        QString msg;
        if (!validateUsername(msg)){
            showError(msg);
        } else {
            showError(QString());
        }
    });

    connect(m_le_hostname, &NCursesLineEdit::outFoucs, [this]{
        QString msg;
        if (!validateHostname(msg)){
            showError(msg);
        } else {
            showError(QString());
        }
    });

    connect(m_le_password, &NCursesLineEdit::outFoucs, [this]{
        QString msg;
        if (!validatePassword(m_le_username->text(), m_le_password->text(), msg)){
            showError(msg);
        } else {
            showError(QString());
        }
    });

    connect(m_le_password_confirm, &NCursesLineEdit::outFoucs, [this]{
        QString msg;
        if (!validatePassword2(m_le_password, m_le_password_confirm, msg)){
            showError(msg);
        } else {
            showError(QString());
        }
    });

#ifdef QT_DEBUG
    connect(m_le_username, &NCursesWindowBase::outFoucs, [=]{qDebug() << "m_le_username outFoucs test";});
    connect(m_le_hostname, &NCursesWindowBase::outFoucs, [=]{qDebug() << "m_le_hostname outFoucs test";});
    connect(m_le_password, &NCursesWindowBase::outFoucs, [=]{qDebug() << "m_le_password outFoucs test";});
    connect(m_le_password_confirm, &NCursesWindowBase::outFoucs, [=]{qDebug() << "m_le_password_confirm outFoucs test";});

    connect(m_NcursesCheckBox, &NcursesCheckBox::signal_SelectChange, this, [=]{qDebug() << "select change";});
    connect(q, &SystemInfoFrame::createRoot, this, [=]{
        if (GetSettingsBool(kSystemIsZF)) qDebug() << "create user:";}
    );
    connect(m_le_username, &NCursesLineEdit::textChanged, this, [=](const QString &name){
        if (GetSettingsBool(kSystemIsZF)) qDebug() << "user name: " << name;
    });
#endif // QT_DEBUG
}

void SystemInfoFramePrivate::updateTs()
{
    m_label_title->erase();
    m_label_instructions->erase();
    m_label_hostname->erase();
    m_label_username->erase();
    m_label_password->erase();
    m_label_password_confirm->erase();

    m_label_title->setText(::QObject::tr("Create Accounts"));
    //printTitle(::QObject::tr("Create Accounts"), width());
    m_label_instructions->setText(::QObject::tr("Fill in the username, computer name and your password"));
    m_label_hostname->setText(::QObject::tr("Computer name").append(" :"));
    m_label_username->setText(::QObject::tr("Username").append(" :"));
    m_label_password->setText(::QObject::tr("Password").append(" :"));
    m_label_password_confirm->setText(::QObject::tr("Repeat password").append(" :"));
    m_NcursesCheckBox->setText("", ::QObject::tr("Set Root Password"));

    ::QObject::tr("If you check \"Set as root password\", the root password is the same as the password, otherwise, you need to set it separately.");
    ::QObject::tr("Your root password should be different from the user password, please take care of them.");

    FrameInterfacePrivate::updateTs();

    layout();

}


SystemInfoFrame::SystemInfoFrame(FrameInterface* parent) :
    FrameInterface (parent)
{
    int h = MAINWINDOW_HEIGHT;//LINES / 2;
    int w = MAINWINDOW_WIDTH;//COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new SystemInfoFramePrivate (this, h, w, beginY, beginX);
    //m_private->hide();
}

SystemInfoFrame::~SystemInfoFrame()
{

}

bool SystemInfoFrame::init()
{
    if (m_currState == FRAME_STATE_NOT_START) {
        //readConf();
        m_currState = FRAME_STATE_RUNNING;
    }
    //m_private->show();
    return true;
}

QString SystemInfoFrame::getFrameName()
{
    return "System Info Frame";
}

bool SystemInfoFrame::handle()
{
    {
        //do something
    }
    m_private->keyHandle();

    return true;
}

}
