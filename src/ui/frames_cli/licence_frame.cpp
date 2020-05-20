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
const QString zh_CN_license { ":/license/deepin-end-user-license-agreement_zh_CN.txt" };
const QString en_US_license{ ":/license/deepin-end-user-license-agreement_en_US.txt" };
#else
const QString zh_CN_license { ":/license/deepin-end-user-license-agreement_community_zh_CN.txt" };
const QString en_US_license{ ":/license/deepin-end-user-license-agreement_community_en_US.txt" };
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

    m_NcursesCheckBox = new NcursesCheckBox(this, 1, (width() - 5) / 2, begy(), begx());
    m_NcursesCheckBox->setIsUseTitle(false);

    m_ncursesTextBrower = new NcursesTextBrower(this, height() - 11, width() - 5, begy() + 2, begx() + 1);
}

bool LicenceFramePrivate::validate()
{
    return true;
}

void LicenceFramePrivate::initConnection()
{

}


void LicenceFramePrivate::onKeyPress(int keyCode)
{
    qDebug()<< keyCode;
    NCursesWindowBase::onKeyPress(keyCode);
}

void LicenceFramePrivate::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    printTitle(QObject::tr("licence"), width());
    QString teststr = QObject::tr("I have read the licence and agree");
    if (installer::ReadLocale() == "zh_CN") {
        QString testlicenceinfo = installer::ReadFile(zh_CN_license);
        m_NcursesCheckBox->setText("", teststr, true);
        m_ncursesTextBrower->setText(testlicenceinfo, true);
    } else {
        QString testlicenceinfo = installer::ReadFile(en_US_license);
        m_NcursesCheckBox->setText("", teststr, false);
        m_ncursesTextBrower->setText(testlicenceinfo, false);
    }

    FrameInterfacePrivate::updateTs();

    layout();
}

void LicenceFramePrivate::layout()
{
    m_NcursesCheckBox->moveWindowTo(begy() + height() - 7, begx() + (width() - 5) / 2 - m_NcursesCheckBox->text().length() / 2);
    m_ncursesTextBrower->setFocus(true);
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
    }
    return true;
}

QString LicenceFrame::getFrameName()
{
    return QObject::tr("LicenceFrame");
}


bool LicenceFrame::handle()
{
    return true;
}

}
