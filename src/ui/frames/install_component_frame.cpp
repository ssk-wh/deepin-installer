#include "install_component_frame.h"
#include "base/file_util.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/component_widget.h"
#include "ui/delegates/componentinstallmanager.h"
#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>

namespace installer {
SelectInstallComponentFrame::SelectInstallComponentFrame(QWidget *parent)
    : QWidget(parent)
    , m_currentComponentWidget(nullptr)
{
    this->setObjectName("install_component_frame");

    initUI();
    initConnections();
}

void SelectInstallComponentFrame::readConf()
{
}

void SelectInstallComponentFrame::writeConf()
{
}

void SelectInstallComponentFrame::initUI()
{
    TitleLabel* selectPageLabel = new TitleLabel(tr(""));
    selectPageLabel->setObjectName("selectPageLabel");

    QLabel* serverTypeLabel = new QLabel(tr(""));
    serverTypeLabel->setObjectName("serverTypeLabel");
    serverTypeLabel->setWordWrap(false);
    serverTypeLabel->setAlignment(Qt::AlignHCenter);

    QLabel* componentLabel = new QLabel(tr(""));
    componentLabel->setObjectName("componentLabel");
    componentLabel->setWordWrap(false);
    componentLabel->setAlignment(Qt::AlignHCenter);

    QVBoxLayout* serverLayout = new QVBoxLayout;

    ComponentInstallManager* manager = ComponentInstallManager::Instance();
    QList<QSharedPointer<ComponentStruct>> serverList = manager->list();
    for (auto it = serverList.cbegin(); it != serverList.cend(); ++it) {
        QString id = it->get()->id();
        ComponentWidget* compWdg = new ComponentWidget(true);
        compWdg->setTitle(id);
        compWdg->setDesc(id.append("desc"));

        serverLayout->addWidget(compWdg, 0, Qt::AlignCenter);

        connect(compWdg, &ComponentWidget::clicked, this
                , &SelectInstallComponentFrame::onServerTypeClicked);

        m_componentStructMap[compWdg] = *it;
    }

    serverLayout->addStretch();

    QWidget* serverWdg = new QWidget;
    serverWdg->setLayout(serverLayout);

    QScrollArea* serverScrollArea = new QScrollArea;
    serverScrollArea->setWidget(serverWdg);
    serverScrollArea->setObjectName("serverScrollArea");
    serverScrollArea->setWidgetResizable(true);
    serverScrollArea->setFocusPolicy(Qt::NoFocus);
    serverScrollArea->setFrameStyle(QFrame::NoFrame);
    serverScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    serverScrollArea->setContentsMargins(0, 0, 0, 0);
    serverScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    serverScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    serverScrollArea->setContextMenuPolicy(Qt::NoContextMenu);
    serverScrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    serverScrollArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    serverScrollArea->setStyleSheet("background: transparent;");
    QScroller::grabGesture(serverScrollArea, QScroller::TouchGesture);
    serverScrollArea->setFixedWidth(468);

    m_compLayout = new QVBoxLayout;
    QWidget *compWdg = new QWidget;
    compWdg->setLayout(m_compLayout);

    QScrollArea* compScrollArea = new QScrollArea;
    compScrollArea->setWidget(compWdg);
    compScrollArea->setObjectName("serverScrollArea");
    compScrollArea->setWidgetResizable(true);
    compScrollArea->setFocusPolicy(Qt::NoFocus);
    compScrollArea->setFrameStyle(QFrame::NoFrame);
    compScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    compScrollArea->setContentsMargins(0, 0, 0, 0);
    compScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    compScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    compScrollArea->setContextMenuPolicy(Qt::NoContextMenu);
    compScrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    compScrollArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    compScrollArea->setStyleSheet("background: transparent;");
    QScroller::grabGesture(compScrollArea, QScroller::TouchGesture);
    compScrollArea->setFixedWidth(468);

    m_backButton = new NavButton(tr("Back"));
    m_backButton->setEnabled(true);

    m_nextButton = new NavButton(tr("Next"));
    m_nextButton->setEnabled(true);

    QVBoxLayout* serverTypeLayout = new QVBoxLayout;
    serverTypeLayout->setContentsMargins(0, 0, 0, 0);
    serverTypeLayout->addWidget(serverTypeLabel);
    serverTypeLayout->addSpacing(20);
    serverTypeLayout->addWidget(serverScrollArea);

    QVBoxLayout* componentLayout = new QVBoxLayout;
    componentLayout->setContentsMargins(0, 0, 0, 0);
    componentLayout->addWidget(componentLabel);
    componentLayout->addSpacing(20);
    componentLayout->addWidget(compScrollArea);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addStretch();
    hLayout->addLayout(serverTypeLayout);
    hLayout->addSpacing(2);
    hLayout->addLayout(componentLayout);
    hLayout->addStretch();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(selectPageLabel, 0, Qt::AlignCenter);
    mainLayout->addSpacing(50);
    mainLayout->addLayout(hLayout);
    mainLayout->addSpacing(40);
    mainLayout->addWidget(m_backButton, 0, Qt::AlignCenter);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_nextButton, 0, Qt::AlignCenter);

    setLayout(mainLayout);
    this->setStyleSheet(ReadFile(":/styles/install_component_frame.css"));
}

void SelectInstallComponentFrame::initConnections()
{
    connect(m_nextButton, &QPushButton::clicked,
            this, &SelectInstallComponentFrame::finished);
}

void SelectInstallComponentFrame::onServerTypeClicked()
{
    ComponentWidget* componentWidget = qobject_cast<ComponentWidget*>(sender());

    if(componentWidget == m_currentComponentWidget){
        return;
    }

    if(m_currentComponentWidget){
        m_currentComponentWidget->setSelected(false);
    }

    m_currentComponentWidget = componentWidget;

    QSharedPointer<ComponentStruct> compStruct = m_componentStructMap[componentWidget];
    QList<QSharedPointer<ComponentInfo>> extra = compStruct->extra();

    QLayoutItem *item;
    while((item = m_compLayout->takeAt(0)) != nullptr){
        m_compLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    m_componentInfoMap.clear();
    for (auto it = extra.cbegin(); it != extra.cend(); ++it) {
        QString id = it->get()->Id;
        ComponentWidget* compWdg = new ComponentWidget(false);
        compWdg->setTitle(id);
        compWdg->setDesc(id.append("desc"));
        m_compLayout->addWidget(compWdg, 0, Qt::AlignCenter);
        m_componentInfoMap[compWdg] = *it;

        connect(compWdg, &ComponentWidget::clicked, this
                , &SelectInstallComponentFrame::onComponentClicked);
    }

    m_compLayout->addStretch();
}

void SelectInstallComponentFrame::onComponentClicked()
{
    QSharedPointer<ComponentInfo> compInfo = m_componentInfoMap[qobject_cast<ComponentWidget*>(sender())];
    ComponentInstallManager* manager = ComponentInstallManager::Instance();
    manager->setComponentSelected(compInfo, !compInfo->isSelected());
}

}
