#pragma once

#include <QWidget>

namespace installer {

class FramelessListView;
class InstallComponentListModel;
class InstallComponentDelegate;
class NavButton;

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
    void onInstallServerTypeListSelected(const QModelIndex current);
    void onInstallServerTypeListCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    FramelessListView *m_installServerTypeListView = nullptr;
    InstallComponentListModel *m_installServerTypeListModel = nullptr;
    InstallComponentDelegate *m_installServerTypeListDelegate = nullptr;

    FramelessListView *m_installComponentListView = nullptr;
    InstallComponentListModel *m_installComponentListModel = nullptr;
    InstallComponentDelegate *m_installComponentListDelegate = nullptr;

    NavButton* m_nextButton = nullptr;
};

}
