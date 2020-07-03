#include "dynamic_disk_warning_frame.h"
#include "ui/utils/widget_util.h"
#include "base/file_util.h"
#include "ui/widgets/title_label.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

using namespace installer;

namespace {
    const int kButtonWidth = 200;
    const int kButtonHeight = 36;

    const int kWarningTipsWidth = 398;
}

DynamicDiskWarningFrame::DynamicDiskWarningFrame(QWidget* parent)
    : QWidget(parent)
    , m_warning(new TitleLabel(""))
    , m_warningTips(new QLabel)
    , m_cancelBtn(new QPushButton)
    , m_acceptBtn(new QPushButton)
    , m_diskListLayout(new QHBoxLayout)
{
    m_warning->setObjectName("WarningText");
    m_warningTips->setObjectName("WarningTips");
    m_warningTips->setWordWrap(true);
    m_warningTips->setFixedWidth(kWarningTipsWidth);

    m_diskListLayout->setContentsMargins(0, 0, 0, 0);
    m_diskListLayout->setSpacing(30);

    m_cancelBtn->setFixedSize(kButtonWidth, kButtonHeight);
    m_acceptBtn->setFixedSize(kButtonWidth, kButtonHeight);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(m_cancelBtn, 0, Qt::AlignHCenter | Qt::AlignLeft);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(m_acceptBtn, 0, Qt::AlignHCenter | Qt::AlignRight);
    QWidget *buttonWrapWidget = new QWidget;
    buttonWrapWidget->setContentsMargins(0, 0, 0, 0);
    buttonWrapWidget->setLayout(buttonLayout);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addSpacing(90);
    layout->addWidget(m_warning, 0, Qt::AlignHCenter);
    layout->addLayout(m_diskListLayout);
    layout->addSpacing(10);
    layout->addWidget(m_warningTips, 1, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(buttonWrapWidget, 0, Qt::AlignHCenter);

    setLayout(layout);

    connect(m_cancelBtn, &QPushButton::clicked, this, &DynamicDiskWarningFrame::requestCancel);
    connect(m_acceptBtn, &QPushButton::clicked, this, &DynamicDiskWarningFrame::requestNext);

    refreshTs();
}

void DynamicDiskWarningFrame::setDevice(const QList<Device::Ptr> list)
{
    QLayoutItem* child;
    while ((child = m_diskListLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->setParent(nullptr);
        }

        delete child;
    }

    for (const Device::Ptr &device : list) {
        QVBoxLayout* diskLayout = new QVBoxLayout;
        diskLayout->setMargin(0);
        diskLayout->setSpacing(0);

        QLabel* diskLbl = new QLabel;
        diskLbl->setPixmap(installer::renderPixmap(":/images/drive-harddisk-128px.svg"));

        QLabel* diskInfoLbl = new QLabel;
        diskInfoLbl->setObjectName("DiskInfoLabel");
        diskInfoLbl->setText(device->path);

        diskLayout->addWidget(diskLbl, 0, Qt::AlignHCenter);
        diskLayout->addSpacing(10);
        diskLayout->addWidget(diskInfoLbl, 0, Qt::AlignHCenter);
        diskLayout->addSpacing(30);

        m_diskListLayout->addLayout(diskLayout);
    }
}

void DynamicDiskWarningFrame::setWarningTip(const QString& tip)
{
    m_warningTips->setText(tip);
}

bool DynamicDiskWarningFrame::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        refreshTs();
    }

    return QWidget::event(event);
}

void DynamicDiskWarningFrame::refreshTs()
{
    m_warning->setText(::QObject::tr("Warning"));
    m_cancelBtn->setText(::QObject::tr("Cancel"));
    m_acceptBtn->setText(::QObject::tr("Next"));
}
