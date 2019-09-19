#include "install_component_frame.h"
#include "base/file_util.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/component_widget.h"
#include "ui/widgets/di_scrollarea.h"
#include "ui/delegates/componentinstallmanager.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"

#include <QLineEdit>
#include <QEvent>
#include <QDebug>

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
    if (!m_currentComponentWidget) {
        return;
    }

    WriteComponentPackages("");
    WriteComponentUninstallPackages("");

    QSharedPointer<ComponentStruct> current =
        m_componentStructMap[m_currentComponentWidget];

    const QStringList installPackages =
        ComponentInstallManager::Instance()->packageListByComponentStruct(current);

    if (!installPackages.isEmpty()) {
        WriteComponentPackages(installPackages.join(" "));
    }

    const QStringList uninstallPackages =
        ComponentInstallManager::Instance()->uninstallPackageListByComponentStruct(
            current);
    if (!uninstallPackages.isEmpty()) {
        WriteComponentUninstallPackages(uninstallPackages.join(" "));
    }
}

bool SelectInstallComponentFrame::event(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        // update all widget ts

        for (auto it = m_componentStructMap.cbegin(); it != m_componentStructMap.cend(); ++it) {
            QPair<QString, QString> ts = ComponentInstallManager::Instance()->updateTs(it.value());
            it.key()->setTitle(ts.first);
            it.key()->setDesc(ts.second);
        }

        for (auto it = m_componentInfoMap.cbegin(); it != m_componentInfoMap.cend(); ++it) {
            QPair<QString, QString> ts = ComponentInstallManager::Instance()->updateTs(it.value());
            it.key()->setTitle(ts.first);
            it.key()->setDesc(ts.second);
        }

        // Write about language
        const QStringList packages =
            ComponentInstallManager::Instance()->loadStructForLanguage(
                installer::ReadLocale());

        if (packages.isEmpty()) {
            return QWidget::event(event);
        }

        WriteComponentLanguage(packages.join(" "));
    }

    return QWidget::event(event);
}

void SelectInstallComponentFrame::initUI()
{
    TitleLabel* selectPageLabel = new TitleLabel(tr("Select component"));
    selectPageLabel->setObjectName("selectPageLabel");

    QLabel* serverTypeLabel = new QLabel(tr("Server Type"));
    serverTypeLabel->setObjectName("serverTypeLabel");
    serverTypeLabel->setWordWrap(false);
    serverTypeLabel->setAlignment(Qt::AlignHCenter);

    QLabel* componentLabel = new QLabel(tr("Component list"));
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

        QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(*it);
        compWdg->setTitle(tsPair.first);
        compWdg->setDesc(tsPair.second);

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

    m_serverScrollArea = new DIScrollArea;
    m_serverScrollArea->setWidget(serverWdg);

    m_componentLayout = new QVBoxLayout;
    m_componentLayout->setSpacing(1);
    m_componentLayout->setMargin(0);

    QWidget *compWdg = new QWidget;
    compWdg->setLayout(m_componentLayout);
    compWdg->setObjectName("compWdg");

    m_compScrollArea = new DIScrollArea;
    m_compScrollArea->setWidget(compWdg);

    m_nextButton = new NavButton(tr("Next"));
    m_nextButton->setEnabled(false);

    QVBoxLayout* serverTypeLayout = new QVBoxLayout;
    serverTypeLayout->setMargin(0);
    serverTypeLayout->setSpacing(0);
    serverTypeLayout->addWidget(serverTypeLabel);
    serverTypeLayout->addSpacing(20);
    serverTypeLayout->addWidget(m_serverScrollArea);

    QVBoxLayout* componentLayout = new QVBoxLayout;
    componentLayout->setMargin(0);
    componentLayout->setSpacing(0);
    componentLayout->setContentsMargins(0, 0, 0, 0);
    componentLayout->addWidget(componentLabel);
    componentLayout->addSpacing(20);
    componentLayout->addWidget(m_compScrollArea);

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

        QPair<QString, QString> tsPair = ComponentInstallManager::Instance()->updateTs(*it);
        compWdg->setTitle(tsPair.first);
        compWdg->setDesc(tsPair.second);

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

    qDebug() << compInfo->Id;
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
