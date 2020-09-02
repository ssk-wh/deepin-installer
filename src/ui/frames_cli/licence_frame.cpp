#include "licence_frame.h"
#include "cursesp.h"
#include "cursesm.h"
#include <QScopedPointer>
#include <QTextCodec>
#include "ui/ncurses_widgets/ncurses_checkbox.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_text_brower.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "base/file_util.h"
#include "service/settings_manager.h"
#include "ui/delegates/license_delegate.h"


namespace installer {

LicenceFramePrivate::LicenceFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
    m_ncursesTextBrower(nullptr),
    m_NcursesCheckBox(nullptr),
    m_localeString(""),
    m_show_license(QString(":/license/end-user-license-agreement-%1")\
        .arg(installer::LicenseDelegate::OSType())),
    m_isshow(false)
{
    initUI();
    initConnection();
}

LicenceFramePrivate::~LicenceFramePrivate()
{
}

void LicenceFramePrivate::initUI()
{
    setBackground(NcursesUtil::getInstance()->dialog_attr());
    FrameInterfacePrivate::initUI();

    m_user_license_lab = new NcursesLabel(this, 1, 6, begy() + 1, begx());
    m_user_license_lab->box(true);
    m_privacy_license = new NcursesLabel(this, 1, 6, begy() + 1, begx());
    m_privacy_license->box(true);
    m_privacy_license->setFocusEnabled(false);

    m_ncursesTextBrower = new NcursesTextBrower(this, height() - 12, width() - 2, begy() + 4, begx() + 1);
    m_ncursesTextBrower->setFocusEnabled(false);
    //m_ncursesTextBrower->hide();

    QString checkboxtext = "I have read and agree to the UOS Software End User License Agreement";
    int textlength = checkboxtext.length();
    if (installer::ReadLocale() == "zh_CN") {
        if((textlength * 2) > (width() - 2)){
            textlength = width() - 2;
        }
    } else {
        textlength += 5;
    }
    m_NcursesCheckBox = new NcursesCheckBox(this, 1, textlength, begy(), begx());
    m_NcursesCheckBox->setIsUseTitle(false);
    //m_NcursesCheckBox->hide();

    QString errorinfo = ::QObject::tr("Please agree to the license");
    m_errorInfoLabel = new NcursesLabel(this, errorinfo, 1, errorinfo.length(), begy() + height() - 5, begx() + (width() - errorinfo.length()) / 2);
    m_errorInfoLabel->setFocusEnabled(false);
    m_errorInfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    //m_errorInfoLabel->hide();
}

bool LicenceFramePrivate::validate()
{
    if(m_NcursesCheckBox->isSelect()) {
        m_errorInfoLabel->hide();
        return true;
    } else {
        m_errorInfoLabel->show();
        return false;
    }
}

void LicenceFramePrivate::show()
{
    if (!m_isshow) {
        FrameInterfacePrivate::show();
        m_pNextButton->setFocusEnabled(m_NcursesCheckBox->isSelect());
        m_errorInfoLabel->hide();
        m_isshow = true;
    }
}

void LicenceFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void LicenceFramePrivate::initConnection()
{
    connect(m_NcursesCheckBox, SIGNAL(signal_SelectChange(bool)), this, SLOT(checkboxSelectChange(bool)));
}


void LicenceFramePrivate::onKeyPress(int keyCode)
{
    switch (keyCode) {
    case KEY_TAB:
        if (m_privacy_license->isOnFoucs()) {
            m_privacy_license->setFocus(false);
            m_user_license_lab->setFocus(true);
        }
        switchChildWindowsFoucs();
        if (m_user_license_lab->isOnFoucs()) {
            setLicense(QString(":/license/end-user-license-agreement-%1")\
                       .arg(installer::LicenseDelegate::OSType()));
        }
        break;
    case kKeyLeft:
    case kKeyRight: switchLicense(); break;
    case kKeyUp:
    case kKeyDown: browseLicense(keyCode); break;
    }

    qDebug()<< keyCode;
    //NCursesWindowBase::onKeyPress(keyCode);
}

void LicenceFramePrivate::setLicense(const QString text)
{

    QString str = ::QObject::tr("I have read and agree to the");
    QString user_lice_str = ::QObject::tr("End User License Agreement");
    QString and_str = ::QObject::tr("and");
    QString privacy_str = ::QObject::tr("Privacy Policy");
    QString teststr = ::QObject::tr("%1 %2 %3 %4")\
            .arg(str)\
            .arg(user_lice_str)\
            .arg(and_str)
            .arg(privacy_str);

    QString zh_cn_li = QString("%1_zh_CN.txt")\
            .arg(text);
    QString en_us_li = QString("%1_en_US.txt")\
            .arg(text);

    qDebug() << "zh_cn_li = " << zh_cn_li;
    qDebug() << "en_us_li = " << en_us_li;
    if (installer::ReadLocale() == "zh_CN") {
        QString testlicenceinfo = installer::ReadFile(zh_cn_li);
        m_NcursesCheckBox->setText("", teststr, true);
        m_ncursesTextBrower->clearText();
        m_ncursesTextBrower->setText(testlicenceinfo, true);

    } else {
        QString testlicenceinfo = installer::ReadFile(en_us_li);
        m_NcursesCheckBox->setText("", teststr, false);
        m_ncursesTextBrower->clearText();
        m_ncursesTextBrower->setText(testlicenceinfo, false);
    }

    m_ncursesTextBrower->show();
}

void LicenceFramePrivate::switchLicense()
{
    if (m_user_license_lab->isOnFoucs()) {
        m_user_license_lab->setFocus(false);
        m_privacy_license->setFocus(true);
        m_show_license = QString(":/license/privacy-policy-%1")\
                .arg(installer::LicenseDelegate::OSType());
    } else if (m_privacy_license->isOnFoucs()) {
        m_user_license_lab->setFocus(true);
        m_privacy_license->setFocus(false);
        m_show_license = QString(":/license/end-user-license-agreement-%1")\
                .arg(installer::LicenseDelegate::OSType());
    }

    setLicense(m_show_license);
}

void LicenceFramePrivate::browseLicense(int key)
{
    if (m_user_license_lab->isOnFoucs() || m_privacy_license->isOnFoucs()) {
        m_ncursesTextBrower->onKeyPress(key);
    }
}

void LicenceFramePrivate::checkboxSelectChange(bool select)
{
    if (select) {
        m_errorInfoLabel->hide();   
        m_pNextButton->setFocusEnabled(true);
    } else {
        m_pNextButton->setFocusEnabled(false);
    }
}

void LicenceFramePrivate::updateTs()
{
    if (!m_localeString.compare(installer::ReadLocale())) {
        return;
    }
    m_localeString = installer::ReadLocale();

    box(ACS_VLINE,ACS_HLINE);
    printTitle(::QObject::tr("UOS Software End User License Agreement"), width());

    switchLicense();

    QString errorinfo = ::QObject::tr("Please agree to the license");
    m_errorInfoLabel->setText(errorinfo);

    m_user_license_lab->setText(::QObject::tr("End User License Agreement"));
    m_user_license_lab->adjustSizeByContext();
    m_user_license_lab->mvwin(begy() + 2, begx() + width() / 4);
    m_privacy_license->setText(::QObject::tr("Privacy Policy"));
    m_privacy_license->adjustSizeByContext();
    m_privacy_license->mvwin(begy() + 2, begx() + width() - width() / 4 - m_privacy_license->width());

    FrameInterfacePrivate::updateTs();

    layout();

    m_pBackButton->setFocus(true);
}

void LicenceFramePrivate::layout()
{
    QString checkboxtext = "I have read and agree to the UOS Software End User License Agreement";
    int textlength = checkboxtext.length();
    if (installer::ReadLocale() == "zh_CN") {
        if((textlength * 2) > (width() - 2)){
            textlength = (width() + checkboxtext.length()) / 2 - 10;
        }
    } else {
        textlength += 10;
    }
    m_NcursesCheckBox->resizew(m_NcursesCheckBox->height(), textlength);
    m_NcursesCheckBox->moveWindowTo(begy() + height() - 7, begx() + (width() - checkboxtext.length()) / 2);
}


LicenceFrame::LicenceFrame(FrameInterface* parent)
    :FrameInterface (parent)
{
    int h = MAINWINDOW_HEIGHT;//LINES / 2;
    int w = MAINWINDOW_WIDTH;//COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new LicenceFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
    //m_private->hide();
}

LicenceFrame::~LicenceFrame()
{

}

bool LicenceFrame::init()
{
    if (m_currState == FRAME_STATE_NOT_START) {
        m_private->layout();
        m_currState = FRAME_STATE_RUNNING;
    }
    return true;
}

QString LicenceFrame::getFrameName()
{
    return "LicenceFrame";
}


bool LicenceFrame::handle()
{
    return true;
}

}
