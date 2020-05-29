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
    m_buttonBox->setFocusPolicy(Qt::NoFocus);
    m_chineseButton = new DButtonBoxButton("中文");
    m_chineseButton->setObjectName("chineseButton");
    m_chineseButton->setCheckable(true);
    m_chineseButton->setFixedWidth(75);
    m_chineseButton->setFixedHeight(36);
    m_englishButton = new DButtonBoxButton("English");
    m_englishButton->setObjectName("englishButton");
    m_englishButton->setCheckable(true);
    m_englishButton->setFixedWidth(75);
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
    m_sourceScrollArea->setFixedWidth(540);
    QPalette pl3 = m_sourceScrollArea->palette();
    pl3.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0)));
    m_sourceScrollArea->setPalette(pl3);

    QHBoxLayout* descriptionLayout = new QHBoxLayout();
    descriptionLayout->setContentsMargins(5, 5, 5, 5);
    descriptionLayout->setSpacing(0);
    descriptionLayout->addWidget(m_sourceScrollArea);

    DFrame *m_bgGroup = new DFrame(this);
    m_bgGroup->setBackgroundRole(DPalette::ItemBackground);
    m_bgGroup->setLineWidth(0);
    m_bgGroup->setContentsMargins(0, 0, 0, 0);
    m_bgGroup->setLayout(descriptionLayout);

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
    mainLayout->addWidget(m_bgGroup, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(19);
    mainLayout->addWidget(m_back, 0, Qt::AlignHCenter);

    setLayout(mainLayout);
}

void UserAgreementFrame::initConnect()
{
    QShortcut *key = new QShortcut(QKeySequence(Qt::Key_Return), m_back);
    key->setAutoRepeat(false);
    connect(key, &QShortcut::activated, this, [=]{
        emit m_back->click();
    });
    connect(m_back, &QPushButton::clicked, this, &UserAgreementFrame::back);
    connect(m_buttonBox, &DButtonBox::buttonClicked, this, &UserAgreementFrame::toggleLicense);
}

void UserAgreementFrame::updateText()
{
    LanguageManager::translator(m_back, &QPushButton::setText, TranslatorType::BackButton);
}

void UserAgreementFrame::toggleLicense(QAbstractButton* button)
{
    if (button == m_currentButton) {
        return;
    }

    m_currentButton = button;

    updateLicenseText();
}

void UserAgreementFrame::updateLicenseText()
{
    m_sourceLbl->setText(installer::ReadFile(m_fileNames[m_nextFileIndex]));

    if (m_nextFileIndex < m_fileNames.count() && m_fileNames.count() > 1) {
        m_nextFileIndex = (m_nextFileIndex + 1) % m_fileNames.count();
    }
}

void UserAgreementFrame::setTitle(const QString &text)
{
    m_subTitle->setText(text);
}

void UserAgreementFrame::setUserAgreement(const QString &primaryFileName, const QString &secondaryFileName)
{
    m_nextFileIndex = 0;
    m_fileNames.clear();
    m_fileNames.append(primaryFileName);

    if (!secondaryFileName.isEmpty()) {
        m_fileNames.append(secondaryFileName);
    }

    updateLicenseText();
}

void UserAgreementFrame::setCheckedButton(int buttonId)
{
    m_buttonBox->button(buttonId)->setChecked(true);
    m_currentButton = m_buttonBox->button(buttonId);
}
