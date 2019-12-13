#include "user_agreement_frame.h"
#include "base/file_util.h"
#include "service/settings_manager.h"
#include "ui/frames/consts.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/pointer_button.h"

#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QButtonGroup>

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
    }

    QFrame::changeEvent(event);
}

void UserAgreementFrame::initUI()
{
    m_logoLbl = new QLabel();
    m_logoLbl->setPixmap(QPixmap(installer::GetVendorLogo()));

    m_subTitle = new QLabel(this);
    m_subTitle->setObjectName("user_agreement_subtitle");

    m_buttonGroup = new QButtonGroup;
    m_chineseButton = new PointerButton("中文");
    m_chineseButton->setObjectName("chineseButton");
    m_chineseButton->setCheckable(true);
    m_chineseButton->setFlat(true);
    m_chineseButton->setMinimumWidth(60);
    m_chineseButton->setMaximumHeight(38);
    m_englishButton = new PointerButton("English");
    m_englishButton->setObjectName("englishButton");
    m_englishButton->setCheckable(true);
    m_englishButton->setFlat(true);
    m_englishButton->setMinimumWidth(60);
    m_englishButton->setMaximumHeight(38);

    m_buttonGroup->addButton(m_chineseButton, kChineseToggleButtonId);
    m_buttonGroup->addButton(m_englishButton, kEnglishToggleButtonId);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_chineseButton, 0, Qt::AlignCenter);
    buttonLayout->addWidget(m_englishButton, 0, Qt::AlignCenter);

    QWidget* buttonGroupWidget = new QWidget;
    buttonGroupWidget->setFixedWidth(120);
    buttonGroupWidget->setLayout(buttonLayout);

    m_sourceLbl = new QLabel(this);
    m_sourceLbl->setObjectName("user_agreement_sourceLbl");
    m_sourceLbl->setWordWrap(true);

    QWidget *sourceWidget = new QWidget;
    sourceWidget->setObjectName("sourceWidget");
    QHBoxLayout *sourceLayout = new QHBoxLayout;
    sourceLayout->setMargin(0);
    sourceLayout->setSpacing(0);
    sourceLayout->addWidget(m_sourceLbl, 0, Qt::AlignTop | Qt::AlignHCenter);
    sourceLayout->addStretch();
    sourceLayout->setContentsMargins(10, 10, 5, 0);
    sourceWidget->setLayout(sourceLayout);

    m_sourceScrollArea = new QScrollArea(this);
    m_sourceScrollArea->setWidget(sourceWidget);
    m_sourceScrollArea->setObjectName("scrollarea");
    m_sourceScrollArea->setWidgetResizable(true);
    m_sourceScrollArea->setFocusPolicy(Qt::NoFocus);
    m_sourceScrollArea->setFrameStyle(QFrame::NoFrame);
    m_sourceScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_sourceScrollArea->setContentsMargins(0, 0, 0, 0);
    m_sourceScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_sourceScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_sourceScrollArea->setContextMenuPolicy(Qt::NoContextMenu);
    m_sourceScrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_sourceScrollArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    QScroller::grabGesture(m_sourceScrollArea, QScroller::TouchGesture);

    m_sourceScrollArea->setFixedWidth(468);

    m_back = new NavButton(this);

    QHBoxLayout* subTitleWrapLayout = new QHBoxLayout;
    subTitleWrapLayout->setMargin(0);
    subTitleWrapLayout->setSpacing(0);
    subTitleWrapLayout->addSpacerItem(new QSpacerItem(120, 38));
    subTitleWrapLayout->addStretch();
    subTitleWrapLayout->addWidget(m_subTitle, 0, Qt::AlignHCenter | Qt::AlignTop);
    subTitleWrapLayout->addStretch();
    subTitleWrapLayout->addWidget(buttonGroupWidget, 0, Qt::AlignRight | Qt::AlignBottom);

    QWidget* subTitleWrapWidget = new QWidget;
    subTitleWrapWidget->setFixedSize(468, 50);
    subTitleWrapWidget->setLayout(subTitleWrapLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(kMainLayoutSpacing);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_logoLbl, 0, Qt::AlignHCenter);
    mainLayout->addWidget(subTitleWrapWidget, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_sourceScrollArea, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_back, 0, Qt::AlignHCenter);

    setLayout(mainLayout);
    setStyleSheet(installer::ReadFile(":/styles/user_agreement_frame.css"));
}

void UserAgreementFrame::initConnect()
{
    connect(m_back, &NavButton::clicked, this, &UserAgreementFrame::back);

    connect(m_buttonGroup, static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked)
            , this, &UserAgreementFrame::toggleLicense);
}

void UserAgreementFrame::updateText()
{
    m_subTitle->setText(tr("End User License Agreement"));
    m_back->setText(tr("Back"));
}

void UserAgreementFrame::toggleLicense()
{
    if (m_nextFileIndex < m_fileNames.count() && m_fileNames.count() > 1) {        
        m_currentFileName = m_fileNames[m_nextFileIndex];
        updateText();
        m_sourceLbl->setText(installer::ReadFile(m_currentFileName));
        m_nextFileIndex = (m_nextFileIndex + 1) % m_fileNames.count();
    }
}

void UserAgreementFrame::setUserAgreement(const QString &primaryFileName, const QString &secondaryFileName)
{
    m_nextFileIndex = 0;
    m_currentFileName = primaryFileName;
    m_fileNames.clear();
    m_fileNames.append(primaryFileName);
    if (!secondaryFileName.isEmpty()) {
        m_fileNames.append(secondaryFileName);
        //switch to primary user agreement,and update the text of m_sourceLbl.
        toggleLicense();
    } else {
        m_sourceLbl->setText(installer::ReadFile(primaryFileName));
    }
}

void UserAgreementFrame::setCheckedButton(int buttonId)
{
    m_buttonGroup->button(buttonId)->setChecked(true);
}
