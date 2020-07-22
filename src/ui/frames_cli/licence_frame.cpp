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


namespace installer {

#ifdef PROFESSIONAL
const QString zh_CN_license { ":/license/server-deepin-end-user-license-agreement_zh_CN.txt" };
const QString en_US_license{ ":/license/server-deepin-end-user-license-agreement_en_US.txt" };
#else
const QString zh_CN_license { ":/license/deepin-end-user-license-agreement_community_zh_CN.txt" };
const QString en_US_license { ":/license/deepin-end-user-license-agreement_community_en_US.txt" };
#endif  // PROFESSIONAL


LicenceFramePrivate::LicenceFramePrivate(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
    m_ncursesTextBrower(nullptr),
    m_NcursesCheckBox(nullptr)
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

    m_ncursesTextBrower = new NcursesTextBrower(this, height() - 11, width() - 2, begy() + 2, begx() + 1);

    QString checkboxtext = "I have read and agree to the UOS Software End User License Agreement";
    int textlength = checkboxtext.length();
    if (installer::ReadLocale() == "zh_CN") {
        if((textlength * 2) > (width() - 2)){
            textlength = width() - 2;
        }
    } else {
        textlength += 5;
    }
    m_NcursesCheckBox = new NcursesCheckBox(this, 1, textlength, begy() + height() - 7, begx() + (width() - checkboxtext.length()) / 2);
    m_NcursesCheckBox->setIsUseTitle(false);

    QString errorinfo = ::QObject::tr("Please agree to the license");
    m_errorInfoLabel = new NcursesLabel(this, errorinfo, 1, errorinfo.length(), begy() + height() - 5, begx() + (width() - errorinfo.length()) / 2);
    m_errorInfoLabel->setFocusEnabled(false);
    m_errorInfoLabel->setBackground(NcursesUtil::getInstance()->error_attr());
    m_errorInfoLabel->hide();

    m_pNextButton->setFocus(false);
    m_pNextButton->setFocusEnabled(false);
    m_ncursesTextBrower->setFocus(true);
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
        NCursesWindowBase::show();
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
    qDebug()<< keyCode;
    NCursesWindowBase::onKeyPress(keyCode);
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
    box(ACS_VLINE,ACS_HLINE);
    printTitle(::QObject::tr("UOS Software End User License Agreement"), width());
    QString teststr = ::QObject::tr("I have read and agree to the UOS Software End User License Agreement");
    if (installer::ReadLocale() == "zh_CN") {
        QString testlicenceinfo = installer::ReadFile(zh_CN_license);
        m_NcursesCheckBox->setText("", teststr, true);
        m_ncursesTextBrower->setText(testlicenceinfo, true);
    } else {
        QString testlicenceinfo = installer::ReadFile(en_US_license);
        m_NcursesCheckBox->setText("", teststr, false);
        m_ncursesTextBrower->setText(testlicenceinfo, false);
    }

    QString errorinfo = ::QObject::tr("Please agree to the license");
    m_errorInfoLabel->setText(errorinfo);

    FrameInterfacePrivate::updateTs();

    layout();
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
        textlength += 5;
    }
    m_NcursesCheckBox->resizew(m_NcursesCheckBox->height(), textlength);
    m_NcursesCheckBox->moveWindowTo(begy() + height() - 7, begx() + (width() - checkboxtext.length()) / 2);
}


LicenceFrame::LicenceFrame(FrameInterface* parent)
    :FrameInterface (parent)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new LicenceFramePrivate (parent->getPrivate(), h, w, beginY, beginX);
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
