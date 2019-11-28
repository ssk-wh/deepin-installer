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
#include <QtConcurrent/QtConcurrent>
#include <QTimer>

namespace installer {

SelectInstallComponentFrame::SelectInstallComponentFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(FrameType::Frame, frameProxyInterface, parent)
    , m_currentComponentWidget(nullptr)
{
    setObjectName("install_component_frame");

    initUI();
    initConnections();
}

void SelectInstallComponentFrame::init()
{
    const QString& defaultInstallType = GetSelectedInstallType();

    if (defaultInstallType.isEmpty()){
        return;
    }

    for (auto it = m_componentStructMap.cbegin(); it != m_componentStructMap.cend(); ++it) {
        if (it.value()->id() == defaultInstallType){
            it.key()->setSelected(true);
            emit it.key()->clicked();
            break;
        }
    }
}

void SelectInstallComponentFrame::finished()
{
    WriteComponentPackages("");
    WriteComponentUninstallPackages("");

    // Write about language
    const QStringList packages =
        ComponentInstallManager::Instance()->loadStructForLanguage(
            installer::ReadLocale());

    if (!packages.isEmpty()) {
        WriteComponentLanguage(packages.join(" "));
    }

    if (!m_currentComponentWidget) {
        return;
    }

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

bool SelectInstallComponentFrame::shouldDisplay() const
{
#ifdef QT_DEBUG
    return true;
#endif // QT_DEBUG
    return !GetSettingsBool(kSkipSelectComponentPage);
}

bool SelectInstallComponentFrame::event(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        // update all widget ts
        m_selectPageLabel->setText(tr("Select Software"));
        m_serverTypeLabel->setText(tr("Basic Environment"));
        m_componentLabel->setText(tr("Add-Ons for Selected Environment"));
        m_nextButton->setText(tr("Next"));
        m_selectAllCheckBox->setText(tr("Select All"));

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
    }

    return QWidget::event(event);
}

bool SelectInstallComponentFrame::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_selectAllFrame) {
        if (event->type() == QMouseEvent::MouseButtonPress) {
            m_selectAllCheckBox->setChecked(!m_selectAllCheckBox->isChecked());
            checkAllComponent(m_selectAllCheckBox->isChecked());
        }
    }

    return QWidget::eventFilter(watched, event);
}

void SelectInstallComponentFrame::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(0, this, [=] {
        m_componentLabel->setFixedWidth(m_serverTypeLabel->width());
    });

    QWidget::resizeEvent(event);
}

void SelectInstallComponentFrame::initUI()
{
    m_selectPageLabel = new TitleLabel(tr("Select Software"));
    m_selectPageLabel->setObjectName("selectPageLabel");

    m_serverTypeLabel = new QLabel(tr("Basic Environment"));
    m_serverTypeLabel->setObjectName("serverTypeLabel");
    m_serverTypeLabel->setWordWrap(false);
    m_serverTypeLabel->setAlignment(Qt::AlignHCenter);

    m_componentLabel = new QLabel(tr("Add-Ons for Selected Environment"));
    m_componentLabel->setObjectName("componentLabel");
    m_componentLabel->setWordWrap(false);
    m_componentLabel->setAlignment(Qt::AlignHCenter);

    QVBoxLayout* serverLayout = new QVBoxLayout;
    serverLayout->setSpacing(1);
    serverLayout->setMargin(0);

    ComponentInstallManager* manager = ComponentInstallManager::Instance();
    QList<QSharedPointer<ComponentStruct>> serverList = manager->list();
    QList<ComponentWidget*> serverWidgetList;
    for (auto it = serverList.cbegin(); it != serverList.cend(); ++it) {
        ComponentWidget* compWdg = new ComponentWidget(true);

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
    serverTypeLayout->addWidget(m_serverTypeLabel);
    serverTypeLayout->addSpacing(20);
    serverTypeLayout->addWidget(m_serverScrollArea);

    QVBoxLayout* componentLayout = new QVBoxLayout;
    componentLayout->setMargin(0);
    componentLayout->setSpacing(0);
    componentLayout->setContentsMargins(0, 0, 0, 0);
    componentLayout->addWidget(m_componentLabel);
    componentLayout->addSpacing(20);
    componentLayout->addWidget(m_compScrollArea);

    componentLayout->addSpacing(1);

    QHBoxLayout *allCheckLayout = new QHBoxLayout;
    allCheckLayout->setContentsMargins(0, 0, 0, 0);

    m_selectAllCheckBox = new QCheckBox;
    m_selectAllCheckBox->setObjectName("selectAllCheckBox");
    m_selectAllCheckBox->setCheckable(true);
    m_selectAllCheckBox->setChecked(false);
    m_selectAllCheckBox->setText(tr("Select All"));
    m_selectAllCheckBox->setFocusPolicy(Qt::NoFocus);
    m_selectAllCheckBox->installEventFilter(this);

    allCheckLayout->addSpacing(8);
    allCheckLayout->addWidget(m_selectAllCheckBox, 0, Qt::AlignLeft | Qt::AlignVCenter);
    allCheckLayout->addStretch();
    m_selectAllFrame = new QFrame;
    m_selectAllFrame->setObjectName("selectAllFrame");
    m_selectAllFrame->setFixedHeight(30);
    m_selectAllFrame->setLayout(allCheckLayout);
    m_selectAllFrame->installEventFilter(this);
    m_selectAllFrame->hide();

    QWidget* serverWidget = new QWidget;
    serverWidget->setLayout(serverTypeLayout);

    componentLayout->addWidget(m_selectAllFrame);
    QWidget* componentWidget = new QWidget;
    componentWidget->setLayout(componentLayout);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    hLayout->addWidget(serverWidget, 0, Qt::AlignRight);
    hLayout->addSpacing(1);
    hLayout->addWidget(componentWidget, 0, Qt::AlignLeft);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_selectPageLabel, 0, Qt::AlignCenter);
    mainLayout->addSpacing(50);
    mainLayout->addLayout(hLayout);
    mainLayout->addSpacing(60);
    mainLayout->addWidget(m_nextButton, 0, Qt::AlignCenter);

    setLayout(mainLayout);
    setStyleSheet(ReadFile(":/styles/install_component_frame.css"));
}

void SelectInstallComponentFrame::initConnections()
{
    connect(m_nextButton, &QPushButton::clicked, this, [=] {
        m_proxy->nextFrame();
    });
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
    }

    m_currentComponentWidget = componentWidget;
    m_selectAllCheckBox->setChecked(false);

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
        compWdg->setSelected(it->get()->Selected);

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

    if (componentWidgetList.count() > 0) {
        m_selectAllFrame->show();
    }
    else {
        m_selectAllFrame->hide();
    }

    updateSelectAllCheckBoxState();

    // Write selected install type
    WriteSelectedInstallType(m_componentStructMap[m_currentComponentWidget]->id());
}

void SelectInstallComponentFrame::onComponentClicked()
{
    ComponentWidget* widget = qobject_cast<ComponentWidget*>(sender());
    QSharedPointer<ComponentInfo> compInfo = m_componentInfoMap[widget];
    compInfo->Selected = widget->isSelected();

    updateSelectAllCheckBoxState();

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

void SelectInstallComponentFrame::checkAllComponent(bool checked)
{
    for (auto it = m_componentInfoMap.cbegin(); it != m_componentInfoMap.end(); ++it) {
        it.key()->setSelected(checked);
        it.value()->Selected = checked;
    }
}

void SelectInstallComponentFrame::updateSelectAllCheckBoxState()
{
    for (auto it = m_componentInfoMap.cbegin(); it != m_componentInfoMap.end(); ++it) {
        if (!it.key()->isSelected()) {
            m_selectAllCheckBox->setChecked(false);
            return;
        }
    }

    m_selectAllCheckBox->setChecked(true);
}

}
