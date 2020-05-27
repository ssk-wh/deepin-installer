#include "full_disk_encrypt_frame.h"

#include "base/file_util.h"
#include "../../../partman/device.h"
#include "../../widgets/rounded_progress_bar.h"
#include "../../widgets/nav_button.h"
#include "../../widgets/line_edit.h"
#include "../../utils/widget_util.h"
#include "../../widgets/title_label.h"
#include "../../../service/settings_manager.h"
#include "../../widgets/system_info_tip.h"
#include "ui/delegates/partition_util.h"
#include "ui/utils/keyboardmonitor.h"
#include "../../widgets/full_disk_partition_colorbar.h"
#include "ui/delegates/full_disk_delegate.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QEvent>
#include <QPushButton>

#define NEXTBTN_WIDTH 310
#define NEXTBTN_HEIGHT 36

using namespace installer;

Full_Disk_Encrypt_frame::Full_Disk_Encrypt_frame(FullDiskDelegate * delegate, QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout(this))
    , m_frameLbl(new TitleLabel(""))
    , m_frameSubLbl(new QLabel)
    , m_encryptLbl(new QLabel)
    , m_encryptCheckLbl(new QLabel)
    , m_encryptEdit(new DPasswordEdit)
    , m_encryptRepeatEdit(new DPasswordEdit)
    , m_cancelBtn(new QPushButton)
    , m_nextBtn(new QPushButton)
    , m_errTip(new SystemInfoTip(this))
    , m_diskPartitionWidget(new FullDiskPartitionWidget)
    , m_diskPartitionDelegate(delegate)
{
    m_layout->setMargin(0);
    m_layout->setSpacing(10);
    m_errTip->hide();
    setObjectName("FullDiskEncryptFrame");
    m_layout->addStretch();

    // add encrypt label
    m_layout->addWidget(m_frameLbl, 0, Qt::AlignHCenter);
    m_layout->addWidget(m_frameSubLbl, 0, Qt::AlignHCenter);

    QHBoxLayout * hboxlayout = new QHBoxLayout();
    hboxlayout->addStretch();
    for(int i = 0; i < FULL_DISK_DISK_MAX_COUNT; i++) {
        QLabel *diskLbl = new QLabel;
        diskLbl->setPixmap(installer::renderPixmap(":/images/driver_128.svg"));
        m_diskinfo[i].m_diskLbl = diskLbl;
        m_diskinfo[i].m_devicePathLbl = new QLabel();
        m_diskinfo[i].m_deviceSizeLbl = new QLabel();
        m_diskinfo[i].m_deviceModelLbl = new QLabel();
        QVBoxLayout *diskInfoLayout = new QVBoxLayout;
        diskInfoLayout->setMargin(0);
        diskInfoLayout->setSpacing(0);
        diskInfoLayout->addWidget(m_diskinfo[i].m_diskLbl, 0, Qt::AlignHCenter);
        diskInfoLayout->addSpacing(6);
        diskInfoLayout->addWidget(m_diskinfo[i].m_devicePathLbl, 0, Qt::AlignHCenter);
        diskInfoLayout->addSpacing(6);
        diskInfoLayout->addWidget(m_diskinfo[i].m_deviceModelLbl, 0, Qt::AlignHCenter);
        diskInfoLayout->addSpacing(6);
        diskInfoLayout->addWidget(m_diskinfo[i].m_deviceSizeLbl, 0, Qt::AlignHCenter);
        hboxlayout->addLayout(diskInfoLayout);
    }
    hboxlayout->addStretch();
    m_layout->addLayout(hboxlayout);
    m_layout->addWidget(m_diskPartitionWidget, 0, Qt::AlignHCenter);

    // add round progress bar
    RoundedProgressBar* spacingBar = new RoundedProgressBar;
    spacingBar->setFixedHeight(2);
    spacingBar->setFixedWidth(600);

    m_layout->addSpacing(10);
    m_layout->addWidget(spacingBar, 0, Qt::AlignHCenter);
    m_layout->addSpacing(10);

    // add encrypt input
    m_encryptLbl->setAlignment(Qt::AlignLeft);
    m_encryptCheckLbl->setAlignment(Qt::AlignLeft);

    QHBoxLayout *encryptLayout = new QHBoxLayout;
    encryptLayout->setContentsMargins(0, 0, 0, 0);
    encryptLayout->setSpacing(0);
    encryptLayout->addWidget(m_encryptLbl, 0, Qt::AlignLeft | Qt::AlignVCenter);
    encryptLayout->addWidget(m_encryptEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    QFrame *encryptFrame = new QFrame;
    encryptFrame->setLayout(encryptLayout);
    m_encryptLbl->setFixedWidth(150);
    m_encryptEdit->setFixedWidth(350);
    encryptFrame->setFixedWidth(600);

    QHBoxLayout *encryptCheckLayout = new QHBoxLayout;
    encryptCheckLayout->setContentsMargins(0, 0, 0, 0);
    encryptCheckLayout->setSpacing(0);
    encryptCheckLayout->addWidget(m_encryptCheckLbl, 0, Qt::AlignLeft | Qt::AlignVCenter);
    encryptCheckLayout->addWidget(m_encryptRepeatEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    QFrame *encryptCheckFrame = new QFrame;
    encryptCheckFrame->setLayout(encryptCheckLayout);
    m_encryptCheckLbl->setFixedWidth(150);
    m_encryptRepeatEdit->setFixedWidth(350);
    encryptCheckFrame->setFixedWidth(600);

    m_layout->addWidget(encryptFrame, 0, Qt::AlignHCenter);
    m_layout->addSpacing(10);
    m_layout->addWidget(encryptCheckFrame, 0, Qt::AlignHCenter);

    m_layout->addStretch();

    // add buttons
    m_cancelBtn->setFixedSize(NEXTBTN_WIDTH, NEXTBTN_HEIGHT);
    m_nextBtn->setFixedSize(NEXTBTN_WIDTH, NEXTBTN_HEIGHT);
    m_layout->addWidget(m_cancelBtn, 0, Qt::AlignHCenter);
    m_layout->addWidget(m_nextBtn, 0, Qt::AlignHCenter);

    setLayout(m_layout);

    m_editList << m_encryptEdit << m_encryptRepeatEdit;

    m_encryptEdit->setEchoMode(QLineEdit::Password);
    m_encryptRepeatEdit->setEchoMode(QLineEdit::Password);

    updateText();

    initConnections();
    onEncryptUpdated(true);
}

void Full_Disk_Encrypt_frame::onShowDeviceInfomation()
{
    updateDiskInfo();
    m_diskPartitionWidget->setDevices(m_diskPartitionDelegate->selectedDevices());
}

void Full_Disk_Encrypt_frame::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        updateText();
    }

    return QWidget::changeEvent(event);
}

void Full_Disk_Encrypt_frame::initConnections()
{
    connect(m_cancelBtn, &QPushButton::clicked, this, &Full_Disk_Encrypt_frame::cancel);
    connect(m_nextBtn, &QPushButton::clicked, this, &Full_Disk_Encrypt_frame::onNextBtnClicked);
    connect(m_encryptEdit, &DLineEdit::textChanged, m_errTip, &SystemInfoTip::hide);
    connect(m_encryptRepeatEdit, &DLineEdit::textChanged, m_errTip, &SystemInfoTip::hide);
    connect(KeyboardMonitor::instance(), &KeyboardMonitor::capslockStatusChanged, this, &Full_Disk_Encrypt_frame::updateEditCapsLockState);
}

void Full_Disk_Encrypt_frame::onNextBtnClicked()
{
    if (m_encryptEdit->text().isEmpty()) {
        m_errTip->setText(tr("Please input password"));
        m_errTip->showBottom(m_encryptEdit);
        return;
    }

    if (m_encryptEdit->text() != m_encryptRepeatEdit->text()) {
        m_errTip->setText(tr("Passwords do not match"));
        m_errTip->showBottom(m_encryptRepeatEdit);
        return;
    }

    WriteFullDiskEncryptPassword(m_encryptEdit->text());

    FinalFullDiskResolution resolution;
    m_diskPartitionDelegate->getFinalDiskResolution(resolution);
    FinalFullDiskOptionList& option_list = resolution.option_list;
    for (FinalFullDiskOption& option : option_list) {
        option.encrypt = true;
    }
    WriteFullDiskResolution(resolution);

    emit finished();
}

void Full_Disk_Encrypt_frame::onEncryptUpdated(bool checked)
{
    m_encryptCheckLbl->setEnabled(checked);
    m_encryptEdit->setEnabled(checked);
    m_encryptLbl->setEnabled(checked);
    m_encryptRepeatEdit->setEnabled(checked);

    if (!checked){
        m_errTip->hide();
    }
}

void Full_Disk_Encrypt_frame::updateText()
{
    m_frameLbl->setText(tr("Encrypt This Disk"));
    m_frameSubLbl->setText(tr("Make sure you have backed up important data, then select the disk to install"));
    m_encryptLbl->setText(tr("Password").append(" :"));
    m_encryptCheckLbl->setText(tr("Repeat Password").append(" :"));
<<<<<<< HEAD
    m_tilabel->setText(tr("Please take good care of your security secret key,"
                         " once the secret key is lost, all your data will be lost!!"));
=======
>>>>>>> parent of 77136906... refactor:modify fulldiskencryptframe UI style
    m_cancelBtn->setText(tr("Previous"));
    m_nextBtn->setText(tr("Start Installation"));
}

void Full_Disk_Encrypt_frame::updateEditCapsLockState(bool on) {
    for (DLineEdit *edit : m_editList) {
//        edit->setCapsLockVisible(on && edit->hasFocus());
    }
}

void Full_Disk_Encrypt_frame::updateDiskInfo(int index)
{
    Device::Ptr device(m_diskinfo[index].m_device);
    m_diskinfo[index].m_devicePathLbl->setText(device->path);
    m_diskinfo[index].m_deviceModelLbl->setText(device->model);
    m_diskinfo[index].m_deviceSizeLbl->setText(QString("%1 GB").arg(ToGigByte(device->getByteLength())));
    m_diskinfo[index].m_diskLbl->show();
    m_diskinfo[index].m_devicePathLbl->show();
    m_diskinfo[index].m_deviceModelLbl->show();
    m_diskinfo[index].m_deviceSizeLbl->show();
}

void Full_Disk_Encrypt_frame::updateDiskInfo()
{
    int i = 0;
    const QStringList& disks = m_diskPartitionDelegate->selectedDisks();
    for (const QString& disk: disks) {
          int index = DeviceIndex(m_diskPartitionDelegate->virtualDevices(), disk);
          if (index < 0) {
              continue;
          }
          const Device::Ptr device = m_diskPartitionDelegate->virtualDevices().at(index);
          m_diskinfo[i].m_device = device;
          updateDiskInfo(i);
          i++;
    }
    while (i < FULL_DISK_DISK_MAX_COUNT) {
        m_diskinfo[i].m_diskLbl->hide();
        m_diskinfo[i].m_devicePathLbl->hide();
        m_diskinfo[i].m_deviceModelLbl->hide();
        m_diskinfo[i].m_deviceSizeLbl->hide();
        i++;
    }
}
