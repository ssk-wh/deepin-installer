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

using namespace installer;

const int kWindowWidth = 960;

Full_Disk_Encrypt_frame::Full_Disk_Encrypt_frame(FullDiskDelegate * delegate, QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout(this))
    , m_frameLbl(new TitleLabel(""))
    , m_frameSubLbl(new QLabel)
    , m_encryptCheck(new QCheckBox)
    , m_encryptLbl(new QLabel)
    , m_encryptCheckLbl(new QLabel)
    , m_encryptEdit(new LineEdit(""))
    , m_encryptRepeatEdit(new LineEdit(""))
    , m_cancelBtn(new NavButton)
    , m_nextBtn(new NavButton)
    , m_errTip(new SystemInfoTip(this))
    , m_diskPartitionWidget(new FullDiskPartitionWidget)
    , m_diskPartitionDelegate(delegate)
{
    m_layout->setMargin(0);
    m_layout->setSpacing(10);

    m_errTip->hide();

    m_encryptCheck->setObjectName("check_box");
    m_encryptCheck->setCheckable(true);
    m_encryptCheck->setChecked(true);
    m_encryptCheck->setFocusPolicy(Qt::NoFocus);

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
    m_diskPartitionWidget->setFixedWidth(kWindowWidth);

    // add round progress bar
    RoundedProgressBar* spacingBar = new RoundedProgressBar;
    spacingBar->setFixedHeight(2);
    spacingBar->setFixedWidth(500);

    m_layout->addSpacing(10);
    m_layout->addWidget(spacingBar, 0, Qt::AlignHCenter);
    m_layout->addSpacing(10);

    // add encrypt checkbox
    m_encryptCheck->setFixedWidth(m_encryptEdit->width());
    m_layout->addWidget(m_encryptCheck, 0, Qt::AlignHCenter);
    m_layout->addSpacing(10);

    // add encrypt input
    m_encryptLbl->setFixedWidth(m_encryptEdit->width());
    m_encryptCheckLbl->setFixedWidth(m_encryptEdit->width());
    m_encryptLbl->setAlignment(Qt::AlignLeft);
    m_encryptCheckLbl->setAlignment(Qt::AlignLeft);
    m_layout->addWidget(m_encryptLbl, 0, Qt::AlignHCenter);
    m_layout->addWidget(m_encryptEdit, 0, Qt::AlignHCenter);
    m_layout->addSpacing(10);
    m_layout->addWidget(m_encryptCheckLbl, 0, Qt::AlignHCenter);
    m_layout->addWidget(m_encryptRepeatEdit, 0, Qt::AlignHCenter);

    m_layout->addStretch();

    // add buttons
    m_layout->addWidget(m_cancelBtn, 0, Qt::AlignHCenter);
    m_layout->addWidget(m_nextBtn, 0, Qt::AlignHCenter);

    setLayout(m_layout);

    m_editList << m_encryptEdit << m_encryptRepeatEdit;

    connect(m_cancelBtn, &NavButton::clicked, this, &Full_Disk_Encrypt_frame::cancel);
    connect(m_nextBtn, &NavButton::clicked, this, &Full_Disk_Encrypt_frame::onNextBtnClicked);
    connect(m_encryptCheck, &QCheckBox::clicked, this, &Full_Disk_Encrypt_frame::onEncryptUpdated);
    connect(m_encryptEdit, &LineEdit::textChanged, m_errTip, &SystemInfoTip::hide);
    connect(m_encryptRepeatEdit, &LineEdit::textChanged, m_errTip, &SystemInfoTip::hide);
    connect(KeyboardMonitor::instance(), &KeyboardMonitor::capslockStatusChanged, this, &Full_Disk_Encrypt_frame::updateEditCapsLockState);

    onEncryptUpdated(true);

    m_encryptEdit->setEchoMode(QLineEdit::Password);
    m_encryptRepeatEdit->setEchoMode(QLineEdit::Password);

    setStyleSheet(ReadFile(":/styles/full_encrypt_frame.css"));
    updateText();
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

void Full_Disk_Encrypt_frame::onNextBtnClicked()
{
    if (m_encryptCheck->isChecked()) {
        // check password

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
    }
    else {
        WriteFullDiskEncryptPassword("");
    }

    bool encrypt = m_encryptCheck->isChecked();
    FinalFullDiskResolution resolution;
    m_diskPartitionDelegate->getFinalDiskResolution(resolution);
    FinalFullDiskOptionList& option_list = resolution.option_list;
    for (FinalFullDiskOption& option : option_list) {
        option.encrypt = encrypt;
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
    m_cancelBtn->setVisible(checked);
}

void Full_Disk_Encrypt_frame::updateText()
{
    m_frameLbl->setText(tr("Encrypt This Disk"));
    m_frameSubLbl->setText(tr("Make sure you have backed up important data, then select the disk to install"));
    m_encryptCheck->setText(tr("Encrypt this disk"));
    m_encryptLbl->setText(tr("Password"));
    m_encryptCheckLbl->setText(tr("Repeat Password"));
    m_cancelBtn->setText(tr("Previous"));
    m_nextBtn->setText(tr("Start Installation"));
}

void Full_Disk_Encrypt_frame::updateEditCapsLockState(bool on) {
    for (LineEdit *edit : m_editList) {
        edit->setCapsLockVisible(on && edit->hasFocus());
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
          int index = DeviceIndex(m_diskPartitionDelegate->virtual_devices(), disk);
          if (index < 0) {
              continue;
          }
          const Device::Ptr device = m_diskPartitionDelegate->virtual_devices().at(index);
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
