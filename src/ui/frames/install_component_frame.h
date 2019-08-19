#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMap>

namespace installer {

class NavButton;
class ComponentStruct;
class ComponentInfo;
class ComponentWidget;

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

private:
    void initUI();
    void initConnections();
    void onServerTypeClicked();
    void onComponentClicked();
    void clearComponentLayout();

private:
    QMap<ComponentWidget*, QSharedPointer<ComponentStruct>> m_componentStructMap;
    QMap<ComponentWidget*, QSharedPointer<ComponentInfo>> m_componentInfoMap;

    QList<QSharedPointer<ComponentInfo>> m_extraComponent;

    QVBoxLayout* m_componentLayout = nullptr;
    ComponentWidget* m_currentComponentWidget = nullptr;

    NavButton* m_nextButton = nullptr;
};

}
