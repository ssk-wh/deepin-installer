#include "install_component_frame.h"
#include "base/file_util.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/component_widget.h"
#include "ui/delegates/componentinstallmanager.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"

#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>

namespace installer {

namespace {
    const int kScrollAreaWidth = 468;
}

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
    QSharedPointer<ComponentStruct> current =
        m_componentStructMap[m_currentComponentWidget];

    const QStringList installPackages =
        ComponentInstallManager::Instance()->packageListByComponentStruct(current);

    if (!installPackages.isEmpty()) {
        WriteComponentPackages(QString("\"%1\"").arg(installPackages.join(" ")));
    }

    const QStringList uninstallPackages =
        ComponentInstallManager::Instance()->uninstallPackageListByComponentStruct(
            current);
    if (!uninstallPackages.isEmpty()) {
        WriteComponentUninstallPackages(QString("'%1'").arg(uninstallPackages.join(" ")));
    }
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
    serverLayout->setSpacing(1);
    serverLayout->setMargin(0);

    ComponentInstallManager* manager = ComponentInstallManager::Instance();
    QList<QSharedPointer<ComponentStruct>> serverList = manager->list();
    QList<ComponentWidget*> serverWidgetList;
    for (auto it = serverList.cbegin(); it != serverList.cend(); ++it) {
        QString id = it->get()->id();
        ComponentWidget* compWdg = new ComponentWidget(true);
        compWdg->setTitle(id);
        compWdg->setDesc(id.append("desc"));

        serverLayout->addWidget(compWdg);

        connect(compWdg, &ComponentWidget::clicked, this
                , &SelectInstallComponentFrame::onServerTypeClicked);

        m_componentStructMap[compWdg] = *it;
        serverWidgetList << compWdg;
    }

    if (!serverWidgetList.isEmpty()) {
        serverWidgetList.first()->setIsHead(true);
        serverWidgetList.last()->setIsTail(true);
    }

    QWidget* serverWdg = new QWidget;
    serverWdg->setLayout(serverLayout);
    serverWdg->setObjectName("serverWdg");

    QScrollArea* serverScrollArea = new QScrollArea;
    serverScrollArea->setWidget(serverWdg);
    serverScrollArea->setObjectName("serverScrollArea");
    serverScrollArea->setWidgetResizable(true);
    serverScrollArea->setFocusPolicy(Qt::NoFocus);
    serverScrollArea->setFrameStyle(QFrame::NoFrame);
    serverScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    serverScrollArea->setContentsMargins(0, 0, 0, 0);
    serverScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    serverScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    serverScrollArea->setContextMenuPolicy(Qt::NoContextMenu);
    serverScrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    serverScrollArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    serverScrollArea->setStyleSheet("background: transparent;");
    QScroller::grabGesture(serverScrollArea, QScroller::TouchGesture);
    serverScrollArea->setFixedWidth(kScrollAreaWidth);

    m_componentLayout = new QVBoxLayout;
    m_componentLayout->setSpacing(1);
    m_componentLayout->setMargin(0);

    QWidget *compWdg = new QWidget;
    compWdg->setLayout(m_componentLayout);

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
    compScrollArea->setFixedWidth(kScrollAreaWidth);

    m_nextButton = new NavButton(tr("Next"));
    m_nextButton->setEnabled(false);

    QVBoxLayout* serverTypeLayout = new QVBoxLayout;
    serverTypeLayout->setMargin(0);
    serverTypeLayout->setSpacing(0);
    serverTypeLayout->addWidget(serverTypeLabel);
    serverTypeLayout->addSpacing(20);
    serverTypeLayout->addWidget(serverScrollArea);

    QVBoxLayout* componentLayout = new QVBoxLayout;
    componentLayout->setMargin(0);
    componentLayout->setSpacing(0);
    componentLayout->setContentsMargins(0, 0, 0, 0);
    componentLayout->addWidget(componentLabel);
    componentLayout->addSpacing(20);
    componentLayout->addWidget(compScrollArea);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    hLayout->addStretch();
    hLayout->addLayout(serverTypeLayout);
    hLayout->addSpacing(1);
    hLayout->addLayout(componentLayout);
    hLayout->addStretch();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(selectPageLabel, 0, Qt::AlignCenter);
    mainLayout->addSpacing(50);
    mainLayout->addLayout(hLayout);
    mainLayout->addSpacing(60);
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
    m_nextButton->setEnabled(componentWidget->isSelected());

    if(componentWidget == m_currentComponentWidget){
        return;
    }

    if(m_currentComponentWidget){
        m_currentComponentWidget->setSelected(false);

        QList<QSharedPointer<ComponentInfo>> defaultValue = m_componentStructMap[m_currentComponentWidget]->defaultValue();
        QList<QSharedPointer<ComponentInfo>> extraList = m_componentStructMap[m_currentComponentWidget]->extra();

        for (QSharedPointer<ComponentInfo> info : defaultValue) {
            info->Selected = false;
        }

        for (QSharedPointer<ComponentInfo> info : extraList) {
            info->Selected = false;
        }
    }

    m_currentComponentWidget = componentWidget;

    QSharedPointer<ComponentStruct> compStruct = m_componentStructMap[componentWidget];
    QList<QSharedPointer<ComponentInfo>> defaultValue = compStruct->defaultValue();
    QList<QSharedPointer<ComponentInfo>> extra = compStruct->extra();

    for (QSharedPointer<ComponentInfo> info : defaultValue) {
        info->Selected = true;
    }

    clearComponentLayout();
    m_componentInfoMap.clear();

    if (extra.isEmpty()) {
        return;
    }

    QList<ComponentWidget*> componentWidgetList;
    for (auto it = extra.cbegin(); it != extra.cend(); ++it) {
        QString id = it->get()->Id;
        ComponentWidget* compWdg = new ComponentWidget(false);
        compWdg->setTitle(id);
        compWdg->setDesc(id.append("desc"));

        m_componentLayout->addWidget(compWdg);

        m_componentInfoMap[compWdg] = *it;

        connect(compWdg, &ComponentWidget::clicked, this
                , &SelectInstallComponentFrame::onComponentClicked);

        componentWidgetList << compWdg;
    }

    m_componentLayout->addStretch();

    ComponentWidget* first = componentWidgetList.first();
    ComponentWidget* last = componentWidgetList.last();

    first->setIsHead(true);
    last->setIsTail(true);
}

void SelectInstallComponentFrame::onComponentClicked()
{
    ComponentWidget* widget = qobject_cast<ComponentWidget*>(sender());
    QSharedPointer<ComponentInfo> compInfo = m_componentInfoMap[widget];
    compInfo->Selected = widget->isSelected();
}

void SelectInstallComponentFrame::clearComponentLayout()
{
    QLayoutItem *item;
    while((item = m_componentLayout->takeAt(0)) != nullptr){
        m_componentLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
}

}
