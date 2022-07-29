#include "resize_root_frame.h"

#include <dlineedit.h>
#include <dspinbox.h>
#include <dsuggestbutton.h>
#include <qboxlayout.h>
#include <qnamespace.h>
#include <qvalidator.h>

#include <DHorizontalLine>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSlider>
#include <QTextEdit>
#include <QVBoxLayout>
#include <memory>

#include "partman/partition.h"
#include "partman/structs.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/full_disk_delegate.h"
#include "ui/delegates/partition_util.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/partition_color_label.h"
#include "ui/widgets/select_button.h"
#include "ui/widgets/title_label.h"

const int kMainFrameWidth  = 560;
const int kMainFrameHeight = 500;

// sync to colorbar
enum class MountPoint { Null, Root, RootB, Data };

struct MountPointInfo {
    QString Color;
    QString Path;
    quint64 Size;
    QString FileSystem;
};

const QMap<MountPoint, MountPointInfo> PART_NAME_COLOR_NAME_MAP{
    { MountPoint::Root, { "#FB7A1F", "/", 15 } },
    { MountPoint::RootB, { "#0587F0", "/rootb", 15 } },
    { MountPoint::Data, { "#7F23FF", "/data", 70 } }
};

class ResizeRootFramePrivate {
    ResizeRootFrame*             q_ptr;
    installer::TitleLabel*       titleLabel         = new installer::TitleLabel;
    installer::FullDiskDelegate* full_disk_delegate = nullptr;
    QLabel*                      inputLabel         = nullptr;
    installer::SelectButton*     cancelBtn          = nullptr;
    DSuggestButton*              acceptBtn          = nullptr;
    QSpinBox*                    box                = nullptr;

public:
    ResizeRootFramePrivate(ResizeRootFrame*             parent,
                           installer::FullDiskDelegate* delegate)
        : q_ptr(parent), full_disk_delegate(delegate)
    {
        q_ptr->setFixedSize(kMainFrameWidth, kMainFrameHeight);
        q_ptr->setContentsMargins(0, 0, 0, 0);
        initUI();
        updateTs();
    }

    void initUI()
    {
        using namespace Dtk::Widget;
        DHorizontalLine* dHerticalLine = new DHorizontalLine;
        QHBoxLayout*     lineLayout    = new QHBoxLayout;
        box                            = new QSpinBox;
        lineLayout->addSpacing(30);
        lineLayout->addWidget(dHerticalLine);
        lineLayout->addSpacing(30);

        cancelBtn                 = new installer::SelectButton;
        acceptBtn                 = new DSuggestButton;
        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->setSpacing(0);
        buttonLayout->addStretch();
        buttonLayout->addWidget(cancelBtn, 0, Qt::AlignHCenter | Qt::AlignLeft);
        buttonLayout->addSpacing(10);
        buttonLayout->addWidget(acceptBtn, 0,
                                Qt::AlignHCenter | Qt::AlignRight);
        buttonLayout->addStretch();

        QLabel* disk_label = new QLabel;
        disk_label->setObjectName("DiskLabel");
        disk_label->setPixmap(QPixmap(installer::GetPartitionIcon128()));

        // 计算出目前 root 最大可用空间
        // efi partition_default_efi_space
        // boot 1.5G
        // swap: DI_SWAP_SIZE
        // 为 data 保留的大小，按最低要求在 64G 下可用空间大小
        // backup kRecoveryDefaultSize
        const int efi_boot  = 2;
        const int swap_size = installer::GetSettingsInt("DI_SWAP_SIZE");
        const int recovery_size =
            installer::GetSettingsInt(installer::kRecoveryDefaultSize);
        const int root_size =
            installer::GetSettingsInt(installer::kPartitionRootMiniSpace);
        const int root_all_size = root_size * 2;
        const int protect_data_size =
            64 - efi_boot - swap_size - recovery_size - root_all_size;
        const int disk_size = installer::ToGigByte(
            full_disk_delegate->selectedDevices().first()->getByteLength());
        const int max = (disk_size - efi_boot - swap_size - recovery_size -
                         protect_data_size) /
                        2;

        // input
        QHBoxLayout* inputLayout = new QHBoxLayout;
        inputLabel               = new QLabel;
        inputLayout->setContentsMargins(0, 0, 0, 0);
        inputLayout->setSpacing(0);
        inputLayout->addStretch();
        inputLayout->addWidget(inputLabel);
        inputLayout->addSpacing(30);
        inputLayout->addWidget(box);
        inputLayout->addStretch();

        box->setFixedWidth(150);
        box->setMinimum(root_size);
        box->setMaximum(max);
        box->setValue(box->minimum());

        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);
        mainLayout->addSpacing(30);
        mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
        mainLayout->addWidget(disk_label, 0, Qt::AlignHCenter);
        mainLayout->addLayout(lineLayout);
        mainLayout->addSpacing(30);
        mainLayout->addLayout(inputLayout);
        mainLayout->addStretch();
        mainLayout->addLayout(buttonLayout);
        mainLayout->addSpacing(30);

        q_ptr->setLayout(mainLayout);

        q_ptr->connect(acceptBtn, &DSuggestButton::clicked, q_ptr, [=] {
            writeSetting(box->value());
            emit q_ptr->finished();
        });
        q_ptr->connect(cancelBtn, &installer::SelectButton::clicked, q_ptr,
                       &ResizeRootFrame::canceled);
    }

    void writeSetting(int size)
    {
        installer::WriteRootPartitionMiniSize(size);
    }

    void updateTs()
    {
        cancelBtn->setText(::QObject::tr("Cancel"));
        acceptBtn->setText(::QObject::tr("Accept"));
        titleLabel->setText(::QObject::tr("Resize Root Partition"));
        inputLabel->setText(QObject::tr("Enter a size (%1 GB - %2 GB)")
                                .arg(box->minimum())
                                .arg(box->maximum()));
    }
};

ResizeRootFrame::ResizeRootFrame(installer::FrameProxyInterface* inter,
                                 installer::FullDiskDelegate*    delegate,
                                 QWidget*                        parent)
    : installer::ChildFrameInterface(inter, parent)
    , d_ptr(new ResizeRootFramePrivate(this, delegate))
{
}

ResizeRootFrame::~ResizeRootFrame() {}

void ResizeRootFrame::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 25, 25);
    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);

    return QWidget::paintEvent(event);
}

void ResizeRootFrame::changeEvent(QEvent* event)
{
    d_ptr->updateTs();
    return installer::ChildFrameInterface::changeEvent(event);
}

#include "resize_root_frame.moc"
