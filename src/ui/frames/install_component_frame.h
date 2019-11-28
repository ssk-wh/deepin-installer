#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMap>
#include <QLabel>
#include <QCheckBox>

#include "ui/interfaces/frameinterface.h"

namespace installer {

class NavButton;
class ComponentStruct;
struct ComponentInfo;
class ComponentWidget;
class DIScrollArea;
class TitleLabel;

class SelectInstallComponentFrame : public FrameInterface
{
    Q_OBJECT

public:
    SelectInstallComponentFrame(FrameProxyInterface* frameProxyInterface, QWidget *parent = nullptr);

    // Read default install components.
    void init() override;

    // Write install components to settings file.
    void finished() override;

    // Read the configuration file to confirm that the current page is available
    bool shouldDisplay() const override;

protected:
    bool event(QEvent* event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void initUI();
    void initConnections();
    void onServerTypeClicked();
    void onComponentClicked();
    void clearComponentLayout();
    void checkAllComponent(bool checked);
    void updateSelectAllCheckBoxState();

private:
    TitleLabel* m_selectPageLabel = nullptr;
    QLabel* m_serverTypeLabel = nullptr;
    QLabel* m_componentLabel = nullptr;

    QMap<ComponentWidget*, QSharedPointer<ComponentStruct>> m_componentStructMap;
    QMap<ComponentWidget*, QSharedPointer<ComponentInfo>> m_componentInfoMap;

    QVBoxLayout* m_componentLayout = nullptr;
    ComponentWidget* m_currentComponentWidget = nullptr;

    DIScrollArea* m_serverScrollArea = nullptr;
    DIScrollArea* m_compScrollArea = nullptr;

    QCheckBox* m_selectAllCheckBox = nullptr;
    QFrame *m_selectAllFrame = nullptr;
    NavButton* m_nextButton = nullptr;
};

}
