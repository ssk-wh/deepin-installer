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
#include "ui/widgets/select_button.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QEvent>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

#define NEXTBTN_WIDTH 240
#define NEXTBTN_HEIGHT 36

using namespace installer;

namespace {
    const int kMainFrameWidth = 600;
    const int kMainFrameHeight = 500;

    const int kContentWidth = 500;
    const int kFullDiskEncryptTitelFont=24;
}

Full_Disk_Encrypt_frame::Full_Disk_Encrypt_frame(FrameProxyInterface* frameProxyInterface
                                                 , FullDiskDelegate * delegate, QWidget *parent)
    : ChildFrameInterface(frameProxyInterface, parent)
    , m_layout(new QVBoxLayout(this))
    , m_frameLbl(new TitleLabel(""))
    , m_frameSubLbl(new QLabel)
    , m_tilabel(new QLabel)
    , m_encryptLbl(new QLabel)
    , m_encryptCheckLbl(new QLabel)
    , m_encryptEdit(new DPasswordEdit)
    , m_encryptRepeatEdit(new DPasswordEdit)
    , m_cancelBtn(new SelectButton)
    , m_confirmBtn(new DSuggestButton)
    , m_errTip(new SystemInfoTip(this))
    , m_diskPartitionWidget(new FullDiskPartitionWidget)
    , m_diskPartitionDelegate(delegate)
{
    // add close button
    setupCloseButton();

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(10);
    m_errTip->hide();
    setObjectName("FullDiskEncryptFrame");
    m_layout->addStretch();

    // add encrypt label
    QFont font;
    font.setPixelSize(kFullDiskEncryptTitelFont);
    m_frameLbl->setFont(font);
    m_layout->addWidget(m_frameLbl, 0, Qt::AlignHCenter);

    QHBoxLayout * hboxlayout = new QHBoxLayout();
    hboxlayout->addStretch();
    for(int i = 0; i < FULL_DISK_DISK_MAX_COUNT; i++) {
        QLabel *diskLbl = new QLabel;
        diskLbl->setFixedSize(QSize(50, 50));
        diskLbl->setPixmap(installer::renderPixmap(":/images/drive-harddisk-64px.svg").scaledToHeight(50));
        m_diskinfo[i].m_diskLbl = diskLbl;
        m_diskinfo[i].m_devicePathLbl = new QLabel();
        m_diskinfo[i].m_deviceSizeLbl = new QLabel();
        QVBoxLayout *diskInfoLayout = new QVBoxLayout;
        diskInfoLayout->setMargin(0);
        diskInfoLayout->setSpacing(0);
        diskInfoLayout->addWidget(m_diskinfo[i].m_diskLbl, 0, Qt::AlignHCenter);
        diskInfoLayout->addSpacing(6);
        diskInfoLayout->addWidget(m_diskinfo[i].m_devicePathLbl, 0, Qt::AlignHCenter);
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
    spacingBar->setFixedWidth(kContentWidth);

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
    m_encryptFrame = new QFrame;
    m_encryptFrame->setLayout(encryptLayout);
    m_encryptLbl->setFixedWidth(150);
    m_encryptEdit->setFixedWidth(350);
    m_encryptEdit->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    m_encryptEdit->setFocusPolicy(Qt::TabFocus);
    m_encryptFrame->setFixedWidth(kContentWidth);

    QHBoxLayout *encryptCheckLayout = new QHBoxLayout;
    encryptCheckLayout->setContentsMargins(0, 0, 0, 0);
    encryptCheckLayout->setSpacing(0);
    encryptCheckLayout->addWidget(m_encryptCheckLbl, 0, Qt::AlignLeft | Qt::AlignVCenter);
    encryptCheckLayout->addWidget(m_encryptRepeatEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    m_encryptCheckFrame = new QFrame;
    m_encryptCheckFrame->setLayout(encryptCheckLayout);
    m_encryptCheckLbl->setFixedWidth(150);
    m_encryptRepeatEdit->setFixedWidth(350);
    m_encryptRepeatEdit->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    m_encryptRepeatEdit->setFocusPolicy(Qt::TabFocus);
    m_encryptCheckFrame->setFixedWidth(kContentWidth);

    m_layout->addWidget(m_encryptFrame, 0, Qt::AlignHCenter);
    m_layout->addSpacing(10);
    m_layout->addWidget(m_encryptCheckFrame, 0, Qt::AlignHCenter);

    m_layout->addStretch();

    m_tilabel->setFixedWidth(kContentWidth);
    m_tilabel->setWordWrap(true);
    m_layout->addWidget(m_tilabel, 0, Qt::AlignHCenter);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(m_cancelBtn, 0, Qt::AlignHCenter | Qt::AlignRight);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(m_confirmBtn, 0, Qt::AlignHCenter | Qt::AlignLeft);
    QWidget *buttonWrapWidget = new QWidget;
    buttonWrapWidget->setLayout(buttonLayout);

    // add buttons
    m_cancelBtn->setFixedSize(NEXTBTN_WIDTH, NEXTBTN_HEIGHT);
    m_cancelBtn->setFocusPolicy(Qt::TabFocus);

    m_confirmBtn->setFixedSize(NEXTBTN_WIDTH, NEXTBTN_HEIGHT);
    m_confirmBtn->setFocusPolicy(Qt::TabFocus);

    m_layout->addWidget(buttonWrapWidget, 0, Qt::AlignHCenter);
    m_layout->addSpacing(10);

    setLayout(m_layout);
    setContentsMargins(0, 0, 0, 0);
    setFixedSize(QSize(kMainFrameWidth, kMainFrameHeight));

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

        if (m_close_button) {
            const int marginSize = this->layout()->margin();
            m_close_button->move(width() - m_close_button->width() - marginSize, marginSize);
            m_close_button->raise();
            m_close_button->show();
        }
    }

    return QWidget::changeEvent(event);
}

void Full_Disk_Encrypt_frame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 25, 25);
    painter.setClipPath(path);
    painter.fillRect(rect(), Qt::white);

    return QWidget::paintEvent(event);
}

void Full_Disk_Encrypt_frame::initConnections()
{
    connect(m_cancelBtn, &QPushButton::clicked, this, &Full_Disk_Encrypt_frame::cancel);
    connect(m_confirmBtn, &QPushButton::clicked, this, &Full_Disk_Encrypt_frame::onNextBtnClicked);
    connect(m_encryptEdit, &DLineEdit::textChanged, m_errTip, &SystemInfoTip::hide);
    connect(m_encryptRepeatEdit, &DLineEdit::textChanged, m_errTip, &SystemInfoTip::hide);
    connect(KeyboardMonitor::instance(), &KeyboardMonitor::capslockStatusChanged, this, &Full_Disk_Encrypt_frame::updateEditCapsLockState);

    connect(m_close_button, &DImageButton::clicked, this, &Full_Disk_Encrypt_frame::cancel);
#ifdef QT_DEBUG
    connect(m_close_button, &DImageButton::clicked, this, []{qDebug() << "close button!";});
#endif // QT_DEBUG

}

void Full_Disk_Encrypt_frame::onNextBtnClicked()
{
    if (m_encryptEdit->text().isEmpty()) {
        m_errTip->setText(::QObject::tr("Please input password"));
        m_errTip->setRelativePosition(m_encryptFrame->pos());
        m_errTip->showBottom(m_encryptEdit);
        return;
    }

    if (m_encryptEdit->text() != m_encryptRepeatEdit->text()) {
        m_errTip->setText(::QObject::tr("Passwords do not match"));
        m_errTip->setRelativePosition(m_encryptCheckFrame->pos());
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

    emit encryptFinished();
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
    m_frameLbl->setText(::QObject::tr("Encrypt This Disk"));
    m_frameSubLbl->setText(::QObject::tr("Make sure you have backed up important data, then select the disk to install"));
    m_encryptLbl->setText(::QObject::tr("Password").append(" :"));
    m_encryptCheckLbl->setText(::QObject::tr("Repeat Password").append(" :"));
    m_cancelBtn->setText(::QObject::tr("Cancel"));
    m_confirmBtn->setText(::QObject::tr("Confirm"));
    m_tilabel->setText(::QObject::tr("Take care of your password, otherwise, all your data will be lost"));
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
    m_diskinfo[index].m_deviceSizeLbl->setText(QString("%1 GB").arg(ToGigByte(device->getByteLength())));
    m_diskinfo[index].m_diskLbl->show();
    m_diskinfo[index].m_devicePathLbl->show();
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
        m_diskinfo[i].m_deviceSizeLbl->hide();
        i++;
    }
}

void Full_Disk_Encrypt_frame::setupCloseButton()
{
    // TODO: use titleBar implement.
    m_close_button = new DImageButton(this);
    m_close_button->setFocusPolicy(Qt::TabFocus);
    m_close_button->setFixedSize(40, 40);
    m_close_button->setNormalPic(":/images/close_normal.svg");
    m_close_button->setHoverPic(":/images/close_normal.svg");
    m_close_button->setPressPic(":/images/close_normal.svg");
}
