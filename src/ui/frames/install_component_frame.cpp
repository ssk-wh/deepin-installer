#include "install_component_frame.h"
#include "base/file_util.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/component_widget.h"
#include "ui/widgets/di_scrollarea.h"
#include "ui/delegates/componentinstallmanager.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"

#include "ui/interfaces/frameinterfaceprivate.h"

#include <QLineEdit>
#include <QEvent>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QCheckBox>
#include <DFrame>
#include <DVerticalLine>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

namespace  {
    const int kBottomSpacing = 60;
    const int kItemSpacing = 10;

    const int kListViewWidth = 270;

    const char kMinimalGraphicsSystem[] = "Server-Environment-with-GUI";
    const char kMinimalCharacterSystem[] = "Custom-Installation";
}

DWIDGET_USE_NAMESPACE

namespace installer {
class SelectInstallComponentFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT

    friend SelectInstallComponentFrame;

public:
    explicit SelectInstallComponentFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate (parent)
        , m_currentComponentWidget(nullptr)
        , q_ptr(qobject_cast<SelectInstallComponentFrame* >(parent))
    {}

    void initUI();
    void onServerTypeClicked();
    void onComponentClicked();
    void clearComponentLayout();
    void checkAllComponent(bool checked);
    void updateSelectAllCheckBoxState();
    void setWidgetLayout(QWidget* subWidget, FrameInterface* obj, QWidget* parentWiget);
    void setSubTitleLayout();
    bool isMinimalGraphicInstall();

    TitleLabel* m_selectPageLabel = nullptr;
    QLabel* m_selectPromptLabel = nullptr;
    QLabel* m_serverTypeLabel = nullptr;
    QLabel* m_componentLabel = nullptr;

    QMap<ComponentWidget*, QSharedPointer<ComponentStruct>> m_componentStructMap;
    QMap<ComponentWidget*, QSharedPointer<ComponentInfo>> m_componentInfoMap;

    QVBoxLayout* m_componentLayout = nullptr;
    ComponentWidget* m_currentComponentWidget = nullptr;

    DIScrollArea* m_serverScrollArea = nullptr;
    DIScrollArea* m_compScrollArea = nullptr;
    QWidget* m_baseComponentListWidget = nullptr;
    QWidget* m_extraComponentListWidget = nullptr;

    QCheckBox* m_selectAllCheckBox = nullptr;
    QFrame *m_selectAllFrame = nullptr;

    SelectInstallComponentFrame* q_ptr = nullptr;
};

SelectInstallComponentFrame::SelectInstallComponentFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new SelectInstallComponentFramePrivate(this))
{
    setObjectName("install_component_frame");

    m_private->initUI();
}

SelectInstallComponentFrame::~SelectInstallComponentFrame()
{

}

void SelectInstallComponentFrame::init()
{
    const QString& defaultInstallType = GetSelectedInstallType();

    if (defaultInstallType.isEmpty()){
        return;
    }

    for (auto it = m_private->m_componentStructMap.cbegin(); it != m_private->m_componentStructMap.cend(); ++it) {
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

    if (!m_private->m_currentComponentWidget) {
        return;
    }

    QSharedPointer<ComponentStruct> current =
        m_private->m_componentStructMap[m_private->m_currentComponentWidget];

    const QStringList installPackages =
        ComponentInstallManager::Instance()->packageListByComponentStruct(current);

    if (!installPackages.isEmpty()) {
        WriteComponentPackages(installPackages.join(" "));
    }

    // Write if minimal graphic install is true, add choice-uninstall to uninstall packages.
    const QStringList uninstallPackages =
        ComponentInstallManager::Instance()->uninstallPackageListByComponentStruct(
            current, m_private->isMinimalGraphicInstall());
    if (!uninstallPackages.isEmpty()) {
        WriteComponentUninstallPackages(uninstallPackages.join(" "));
    }

    const QString selectedInstallType = m_private->m_componentStructMap[m_private->m_currentComponentWidget]->id();
    if (m_private->isMinimalGraphicInstall()) {
        if (selectedInstallType == kMinimalGraphicsSystem) {
            WriteIsMinimalGraphicsSystem(true);
        }
        else if (selectedInstallType == kMinimalCharacterSystem) {
            WriteIsMinimalCharacterSystem(true);
        }
        else {
            qInfo() << "Select component install type: " << selectedInstallType;
        }
    }
}

bool SelectInstallComponentFrame::shouldDisplay() const
{
#ifdef QT_DEBUG
    return true;
#else
    return !GetSettingsBool(kSkipSelectComponentPage);
#endif // QT_DEBUG
}

QString SelectInstallComponentFrame::returnFrameName() const
{
    return "Select Software";
}

bool SelectInstallComponentFrame::event(QEvent* event) {

    if (event->type() == QEvent::LanguageChange) {
        // update all widget ts
        m_private->m_selectPageLabel->setText(tr("Select Software"));
        m_private->m_serverTypeLabel->setText(tr("Basic Environment"));
        m_private->m_selectPromptLabel->setText(tr("Select the components according to your needs"));
        m_private->m_componentLabel->setText(tr("Add-Ons"));
        m_private->nextButton->setText(tr("Next"));
        m_private->m_selectAllCheckBox->setText(tr("Select All"));

        for (auto it = m_private->m_componentStructMap.cbegin(); it != m_private->m_componentStructMap.cend(); ++it) {
            QPair<QString, QString> ts = ComponentInstallManager::Instance()->updateTs(it.value());
            it.key()->setTitle(ts.first);
            it.key()->setDesc(ts.second);
        }

        for (auto it = m_private->m_componentInfoMap.cbegin(); it != m_private->m_componentInfoMap.cend(); ++it) {
            QPair<QString, QString> ts = ComponentInstallManager::Instance()->updateTs(it.value());
            it.key()->setTitle(ts.first);
            it.key()->setDesc(ts.second);
        }
    }

    return QWidget::event(event);
}

bool SelectInstallComponentFrame::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_private->m_selectAllFrame) {
        if (event->type() == QMouseEvent::MouseButtonPress) {
            m_private->m_selectAllCheckBox->setChecked(!m_private->m_selectAllCheckBox->isChecked());
            m_private->checkAllComponent(m_private->m_selectAllCheckBox->isChecked());
        }
    }
    if (event->type() == QEvent::Resize) {
        m_private->setSubTitleLayout();
    }

    return QWidget::eventFilter(watched, event);
}

void SelectInstallComponentFrame::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(0, this, [=] {
        m_private->setSubTitleLayout();
    });

    QWidget::resizeEvent(event);
}

void SelectInstallComponentFramePrivate::initUI()
{
    m_selectPageLabel = new TitleLabel(tr("Select Software"));
    m_selectPageLabel->setObjectName("selectPageLabel");

    m_selectPromptLabel = new QLabel(tr("Select the component according to your needs"));
    m_selectPromptLabel->setObjectName("m_selectPromptLabel");

    m_serverTypeLabel = new QLabel(tr("Basic Environment"), q_ptr);
    m_serverTypeLabel->setObjectName("serverTypeLabel");
    m_serverTypeLabel->setWordWrap(false);
    m_serverTypeLabel->setAlignment(Qt::AlignHCenter);

    m_componentLabel = new QLabel(tr("Add-Ons"), q_ptr);
    m_componentLabel->setObjectName("componentLabel");
    m_componentLabel->setWordWrap(false);
    m_componentLabel->setAlignment(Qt::AlignHCenter);

    QVBoxLayout* serverLayout = new QVBoxLayout;
    serverLayout->setSpacing(kItemSpacing);
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
                , &SelectInstallComponentFramePrivate::onServerTypeClicked);

        m_componentStructMap[compWdg] = *it;
        serverWidgetList << compWdg;
    }

    if (!serverWidgetList.isEmpty()) {
        serverWidgetList.first()->setIsHead(true);
        serverWidgetList.last()->setIsTail(true);
    }

    m_baseComponentListWidget = new QWidget;
    m_baseComponentListWidget->setLayout(serverLayout);
    m_baseComponentListWidget->setObjectName("serverWdg");

    m_serverScrollArea = new DIScrollArea;
    m_serverScrollArea->setFrameShape(QFrame::Shape::NoFrame);
    m_serverScrollArea->setWidget(m_baseComponentListWidget);

    m_componentLayout = new QVBoxLayout;
    m_componentLayout->setSpacing(kItemSpacing);
    m_componentLayout->setMargin(0);

    m_extraComponentListWidget = new QWidget;
    m_extraComponentListWidget->setLayout(m_componentLayout);
    m_extraComponentListWidget->setObjectName("compWdg");

    m_compScrollArea = new DIScrollArea;
    m_compScrollArea->setFrameShape(QFrame::Shape::NoFrame);
    m_compScrollArea->setWidget(m_extraComponentListWidget);

    QVBoxLayout* serverTypeLayout = new QVBoxLayout;
    serverTypeLayout->setContentsMargins(kItemSpacing, kItemSpacing, kItemSpacing, kItemSpacing);
    serverTypeLayout->setSpacing(0);
    serverTypeLayout->addWidget(m_serverScrollArea);

    QVBoxLayout* componentLayout = new QVBoxLayout;
    componentLayout->setContentsMargins(kItemSpacing, kItemSpacing, kItemSpacing, 0);
    componentLayout->setSpacing(0);
    componentLayout->addWidget(m_compScrollArea);

    QHBoxLayout *allCheckLayout = new QHBoxLayout;
    allCheckLayout->setContentsMargins(0, 0, 0, 0);

    m_selectAllCheckBox = new QCheckBox;
    m_selectAllCheckBox->setObjectName("selectAllCheckBox");
    m_selectAllCheckBox->setCheckable(true);
    m_selectAllCheckBox->setChecked(false);
    m_selectAllCheckBox->setText(tr("Select All"));
    m_selectAllCheckBox->setFocusPolicy(Qt::NoFocus);

    connect(m_selectAllCheckBox, &QCheckBox::clicked, this, [&] {
        checkAllComponent(m_selectAllCheckBox->isChecked());
    });
    m_selectAllCheckBox->installEventFilter(this);

    allCheckLayout->addSpacing(kItemSpacing);
    allCheckLayout->addWidget(m_selectAllCheckBox, 0, Qt::AlignLeft | Qt::AlignVCenter);
    allCheckLayout->addStretch();
    m_selectAllFrame = new QFrame;
    m_selectAllFrame->setObjectName("selectAllFrame");
    m_selectAllFrame->setFixedHeight(30);
    m_selectAllFrame->setLayout(allCheckLayout);
    m_selectAllFrame->installEventFilter(this);
    m_selectAllFrame->hide();

    QFrame* serverWidget = new QFrame;
    serverWidget->setFrameShape(QFrame::Shape::NoFrame);
    serverWidget->setContentsMargins(0, 0, 0, 0);
    serverWidget->setLayout(serverTypeLayout);
    serverWidget->setFixedWidth(kListViewWidth);

    componentLayout->addSpacing(kItemSpacing);
    componentLayout->addWidget(m_selectAllFrame);
    QFrame* componentWidget = new QFrame;
    componentWidget->setFrameShape(QFrame::Shape::NoFrame);
    componentWidget->setContentsMargins(0, 0, 0, 0);
    componentWidget->setLayout(componentLayout);
    componentWidget->setFixedWidth(kListViewWidth);

    DVerticalLine* dVerticalLine = new DVerticalLine;

    QHBoxLayout* hLineBoxLayout = new QHBoxLayout;
    hLineBoxLayout->addWidget(dVerticalLine);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->addWidget(serverWidget, 0, Qt::AlignLeft);
    hLayout->addLayout(hLineBoxLayout);
    hLayout->addWidget(componentWidget, 0, Qt::AlignRight);

    DFrame* frame = new DFrame;
    frame->setFrameRounded(true);
    frame->setContentsMargins(1, 1, 1, 1);
    frame->setLayout(hLayout);
    frame->installEventFilter(q_ptr);

    centerLayout->setSpacing(0);
    centerLayout->setMargin(0);
    centerLayout->addWidget(m_selectPageLabel, 0, Qt::AlignCenter);
    centerLayout->addWidget(m_selectPromptLabel, 0, Qt::AlignCenter);
    centerLayout->addSpacing(55);
    centerLayout->addWidget(frame, 0, Qt::AlignCenter);
    centerLayout->addSpacing(20);
}

void SelectInstallComponentFramePrivate::onServerTypeClicked()
{
    ComponentWidget* componentWidget = qobject_cast<ComponentWidget*>(sender());
    nextButton->setEnabled(componentWidget->isSelected());

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
                , &SelectInstallComponentFramePrivate::onComponentClicked);

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

void SelectInstallComponentFramePrivate::setWidgetLayout(QWidget* subWidget,
                                                             FrameInterface* obj, QWidget* parentWiget)
{
    subWidget->setFixedWidth(parentWiget->width());
    subWidget->move(parentWiget->mapTo(obj, QPoint()).x() +
                    (parentWiget->width() - subWidget->width()) / 2,
                    parentWiget->mapTo(obj, QPoint()).y() - kBottomSpacing);
}

void SelectInstallComponentFramePrivate::setSubTitleLayout()
{
    setWidgetLayout(m_serverTypeLabel, q_ptr, m_baseComponentListWidget);
    setWidgetLayout(m_componentLabel, q_ptr, m_extraComponentListWidget);
}

void SelectInstallComponentFramePrivate::onComponentClicked()
{
    ComponentWidget* widget = qobject_cast<ComponentWidget*>(sender());
    QSharedPointer<ComponentInfo> compInfo = m_componentInfoMap[widget];
    compInfo->Selected = widget->isSelected();

    updateSelectAllCheckBoxState();

    qDebug() << compInfo->Id;
}

void SelectInstallComponentFramePrivate::clearComponentLayout()
{
    QLayoutItem *item;
    while((item = m_componentLayout->takeAt(0)) != nullptr){
        m_componentLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
}

void SelectInstallComponentFramePrivate::checkAllComponent(bool checked)
{
    for (auto it = m_componentInfoMap.cbegin(); it != m_componentInfoMap.end(); ++it) {
        it.key()->setSelected(checked);
        it.value()->Selected = checked;
    }
}

void SelectInstallComponentFramePrivate::updateSelectAllCheckBoxState()
{
    for (auto it = m_componentInfoMap.cbegin(); it != m_componentInfoMap.end(); ++it) {
        if (!it.key()->isSelected()) {
            m_selectAllCheckBox->setChecked(false);
            return;
        }
    }

    m_selectAllCheckBox->setChecked(true);
}

bool SelectInstallComponentFramePrivate::isMinimalGraphicInstall()
{
    if (!m_currentComponentWidget) {
        return false;
    }

    for (auto it = m_componentInfoMap.cbegin(); it != m_componentInfoMap.end(); ++it) {
        if (it.key()->isSelected()) {
            return false;
        }
    }

    return true;
}

}// namespace installer

#include "install_component_frame.moc"
