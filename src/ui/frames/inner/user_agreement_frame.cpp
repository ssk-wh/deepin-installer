#include "user_agreement_frame.h"
#include "base/file_util.h"
#include "service/settings_manager.h"
#include "ui/frames/consts.h"
#include "ui/widgets/pointer_button.h"
#include "service/language_manager.h"
#include "ui/delegates/license_delegate.h"

#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <DFrame>
#include <QShortcut>

DWIDGET_USE_NAMESPACE

using namespace installer;

UserAgreementFrame::UserAgreementFrame(QWidget *parent)
    : QFrame(parent)
    , m_language(QLocale::Language::English)
    , m_nextFileIndex(0)
{
    initUI();
    initConnect();
    updateText();
}

void UserAgreementFrame::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        QLocale locale;
        m_language = locale.language();
        updateText();
        setFocus();
    }

    QFrame::changeEvent(event);
}

void UserAgreementFrame::initUI()
{

    m_logoLbl = new QLabel();
    m_logoLbl->setPixmap(QPixmap(LicenseDelegate::logo()));

    m_sourceLbl = new QLabel(this);
    m_sourceLbl->setObjectName("user_agreement_sourceLbl");
    m_sourceLbl->setWordWrap(true);
    QPalette pl = m_sourceLbl->palette();
    pl.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0)));
    m_sourceLbl->setPalette(pl);

    QWidget *sourceWidget = new QWidget;
    sourceWidget->setObjectName("sourceWidget");
    sourceWidget->setContentsMargins(0, 0, 0, 0);
    QPalette pl2 = sourceWidget->palette();
    pl2.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0)));
    sourceWidget->setPalette(pl2);

    QHBoxLayout *sourceLayout = new QHBoxLayout;
    sourceLayout->setMargin(0);
    sourceLayout->setSpacing(0);
    sourceLayout->addWidget(m_sourceLbl, 0, Qt::AlignTop | Qt::AlignHCenter);
    sourceLayout->addStretch();
    sourceLayout->setContentsMargins(5, 5, 15, 0);
    sourceWidget->setLayout(sourceLayout);

    m_sourceScrollArea = new QScrollArea(this);
    m_sourceScrollArea->setWidget(sourceWidget);
    m_sourceScrollArea->setObjectName("scrollarea");
    m_sourceScrollArea->setWidgetResizable(true);
    m_sourceScrollArea->setFrameStyle(QFrame::NoFrame);
    m_sourceScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_sourceScrollArea->setContentsMargins(0, 0, 0, 0);
    m_sourceScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_sourceScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_sourceScrollArea->setContextMenuPolicy(Qt::NoContextMenu);
    m_sourceScrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_sourceScrollArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_sourceScrollArea->setStyleSheet("QScrollArea::focus{border:1px solid; border-color:rgb(1, 128, 255); border-radius:5px; padding:2px 4px;}");

    QScroller::grabGesture(m_sourceScrollArea, QScroller::TouchGesture);
    m_sourceScrollArea->setFixedWidth(540);
    QPalette pl3 = m_sourceScrollArea->palette();
    pl3.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0)));
    m_sourceScrollArea->setPalette(pl3);

    QHBoxLayout* descriptionLayout = new QHBoxLayout();
    descriptionLayout->setContentsMargins(5, 5, 0, 5);
    descriptionLayout->setSpacing(0);
    descriptionLayout->addWidget(m_sourceScrollArea);

    DFrame *m_bgGroup = new DFrame(this);
    m_bgGroup->setBackgroundRole(DPalette::ItemBackground);
    m_bgGroup->setLineWidth(0);
    m_bgGroup->setContentsMargins(0, 0, 0, 0);
    m_bgGroup->setLayout(descriptionLayout);

    m_back = new QPushButton;
    m_back->setFixedSize(310, 36);
    //m_back->setFocusPolicy(Qt::TabFocus);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(kMainLayoutSpacing);

    mainLayout->addSpacing(16);
    mainLayout->addWidget(m_logoLbl, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_bgGroup, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(19);
    mainLayout->addWidget(m_back, 0, Qt::AlignHCenter);

    setLayout(mainLayout);
}

void UserAgreementFrame::initConnect()
{
    connect(m_back, &QPushButton::clicked, this, &UserAgreementFrame::back);
}

void UserAgreementFrame::updateText()
{
    m_back->setText(::QObject::tr("Back", "button"));
}


void UserAgreementFrame::updateLicenseText()
{
    m_sourceLbl->setText(installer::ReadFile(m_fileNames[m_nextFileIndex]));

    if (m_nextFileIndex < m_fileNames.count() && m_fileNames.count() > 1) {
        m_nextFileIndex = (m_nextFileIndex + 1) % m_fileNames.count();
    }
}


bool UserAgreementFrame::doSelect()
{
    if (m_back->hasFocus()) {
        emit back();
    }
    return true;
}

bool UserAgreementFrame::directionKey(int keyvalue)
{
    switch (keyvalue) {
    case Qt::Key_Up: {
            if (m_sourceScrollArea->hasFocus()) {
                int testvalue = m_sourceScrollArea->verticalScrollBar()->value();
                m_sourceScrollArea->verticalScrollBar()->setValue(testvalue - 20);
            }
        }
        break;
    case Qt::Key_Down: {
            if (m_sourceScrollArea->hasFocus()) {
                int testvalue = m_sourceScrollArea->verticalScrollBar()->value();
                m_sourceScrollArea->verticalScrollBar()->setValue(testvalue + 20);
            }
        }
        break;
    }

    return true;
}

bool UserAgreementFrame::focusSwitch()
{
     if (m_sourceScrollArea->hasFocus()) {
        m_back->setFocus();
    } else if (m_back->hasFocus()) {
        m_sourceScrollArea->setFocus();
    } else {
        m_back->setFocus();
    }
    return true;

}

void UserAgreementFrame::setUserAgreement(const QString &primaryFileName, const QString &secondaryFileName)
{
    m_nextFileIndex = 0;
    m_fileNames.clear();
    m_fileNames.append(primaryFileName);
    updateLicenseText();
}
