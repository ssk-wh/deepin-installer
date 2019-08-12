#include "install_component_frame.h"
#include "base/file_util.h"
#include "ui/widgets/nav_button.h"
#include "ui/models/install_component_model.h"
#include "ui/delegates/install_component_delegate.h"
#include "ui/views/frameless_list_view.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace installer {
SelectInstallComponentFrame::SelectInstallComponentFrame(QWidget *parent)
    : QWidget(parent)
{
    m_installServerTypeListView = new FramelessListView;
    m_installServerTypeListModel = new InstallComponentListModel(m_installServerTypeListView);
    m_installServerTypeListView->setModel(m_installServerTypeListModel);
    m_installServerTypeListDelegate = new InstallComponentDelegate;
    m_installServerTypeListDelegate->setObjectName("install_serverType_list_delegate");


    m_nextButton = new NavButton(tr("Next"));
    m_nextButton->setEnabled(true);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(m_installServerTypeListView);
    hLayout->addStretch();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hLayout);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_nextButton, 0, Qt::AlignCenter);

    initConnections();

    setLayout(mainLayout);
}

void SelectInstallComponentFrame::readConf()
{
}

void SelectInstallComponentFrame::writeConf()
{
}

void SelectInstallComponentFrame::initConnections()
{
    connect(m_installServerTypeListView, &QListView::clicked, this
            , &SelectInstallComponentFrame::onInstallServerTypeListSelected);
    connect(m_installServerTypeListView->selectionModel(), &QItemSelectionModel::currentChanged, this
            , &SelectInstallComponentFrame::onInstallServerTypeListCurrentChanged);
    connect(m_nextButton, &QPushButton::clicked,
            this, &SelectInstallComponentFrame::finished);
}

void SelectInstallComponentFrame::onInstallServerTypeListSelected(const QModelIndex current)
{
    if(current.isValid()){
        m_installServerTypeListView->setCurrentIndex(current);
    }
}

void SelectInstallComponentFrame::onInstallServerTypeListCurrentChanged(const QModelIndex &current
                                                                        , const QModelIndex &previous)
{
    Q_UNUSED(previous);
    // Skip first signal
    if (current == m_installServerTypeListView->model()->index(0, 0) && !previous.isValid()) {
        return;
    }
    emit m_installServerTypeListView->clicked(current);
}

}
