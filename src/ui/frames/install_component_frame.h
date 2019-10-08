#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMap>
#include <QLabel>

namespace installer {

class NavButton;
class ComponentStruct;
struct ComponentInfo;
class ComponentWidget;
class DIScrollArea;
class TitleLabel;

class SelectInstallComponentFrame : public QWidget
{
    Q_OBJECT

public:
    SelectInstallComponentFrame(QWidget *parent = nullptr);

    // Read default install components.
    void readConf();
    // Write install components to settings file.
    void writeConf();
signals:
    void finished();

protected:
    bool event(QEvent* event) override;

private:
    void initUI();
    void initConnections();
    void onServerTypeClicked();
    void onComponentClicked();
    void clearComponentLayout();

private:
    TitleLabel* m_selectPageLabel = nullptr;
    QLabel* m_serverTypeLabel = nullptr;
    QLabel* m_componentLabel = nullptr;

    QMap<ComponentWidget*, QSharedPointer<ComponentStruct>> m_componentStructMap;
    QMap<ComponentWidget*, QSharedPointer<ComponentInfo>> m_componentInfoMap;

    QList<QSharedPointer<ComponentInfo>> m_extraComponent;

    QVBoxLayout* m_componentLayout = nullptr;
    ComponentWidget* m_currentComponentWidget = nullptr;

    DIScrollArea* m_serverScrollArea = nullptr;
    DIScrollArea* m_compScrollArea = nullptr;

    NavButton* m_nextButton = nullptr;
};

}
