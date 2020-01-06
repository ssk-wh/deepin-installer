#include "user_agreement_frame.h"
#include "base/file_util.h"
#include "service/settings_manager.h"
#include "ui/frames/consts.h"
#include "ui/widgets/pointer_button.h"
#include "service/language_manager.h"

#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>

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

    m_buttonBox = new DButtonBox;
    m_chineseButton = new DButtonBoxButton("中文");
    m_chineseButton->setObjectName("chineseButton");
    m_chineseButton->setCheckable(true);
    m_chineseButton->setFixedHeight(36);
    m_englishButton = new DButtonBoxButton("English");
    m_englishButton->setObjectName("englishButton");
    m_englishButton->setCheckable(true);
    m_englishButton->setFixedHeight(36);

    m_btnlist.append(m_chineseButton);
    m_btnlist.append(m_englishButton);

    m_buttonBox->setButtonList(m_btnlist, true);
    m_buttonBox->setId(m_chineseButton, 0);
    m_buttonBox->setId(m_englishButton, 1);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_buttonBox, 0, Qt::AlignHCenter);

    QWidget* buttonBoxWidget = new QWidget;
    buttonBoxWidget->setLayout(buttonLayout);

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

    m_back = new QPushButton;
    m_back->setFixedSize(310, 36);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(kMainLayoutSpacing);

    mainLayout->addSpacing(16);
    mainLayout->addWidget(m_logoLbl, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_subTitle, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(buttonBoxWidget, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_sourceScrollArea, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(19);
    mainLayout->addWidget(m_back, 0, Qt::AlignHCenter);

    setLayout(mainLayout);
}

void UserAgreementFrame::initConnect()
{
    connect(m_back, &QPushButton::clicked, this, &UserAgreementFrame::back);
    connect(m_buttonBox, &DButtonBox::buttonClicked, this, &UserAgreementFrame::toggleLicense);
}

void UserAgreementFrame::updateText()
{
    m_subTitle->setText(tr("End User License Agreement"));
    LanguageManager::translator(m_back, &QPushButton::setText, TranslatorType::BackButton);
}

void UserAgreementFrame::updateLicenseText()
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
    m_fileNames.clear();
    m_fileNames.append(primaryFileName);

    if (!secondaryFileName.isEmpty()) {
        m_fileNames.append(secondaryFileName);
        //switch to primary user agreement,and update the text of m_sourceLbl.
        toggleLicense();
    } else {
        m_sourceLbl->setText(installer::ReadFile(primaryFileName));
    }

    updateLicenseText();
}

void UserAgreementFrame::setCheckedButton(int buttonId)
{
    m_buttonGroup->button(buttonId)->setChecked(true);
    m_currentButton = m_buttonGroup->button(buttonId);
}

void UserAgreementFrame::setCheckedButton(int buttonId)
{
    m_buttonBox->button(buttonId)->setChecked(true);
}
