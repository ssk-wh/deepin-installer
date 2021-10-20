#include "full_disk_encrypt_frame.h"

#include "base/file_util.h"
#include "partman/device.h"
#include "ui/widgets/rounded_progress_bar.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/line_edit.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/system_info_tip.h"
#include "ui/delegates/partition_util.h"
#include "ui/utils/keyboardmonitor.h"
#include "ui/widgets/full_disk_partition_colorbar.h"
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
    const int kMainFrameHeight = 550;

    const int kContentWidth = 500;
    const int kFullDiskEncryptTitelFont=24;

    const int kMaxPasswordLength = 24;
}

Full_Disk_Encrypt_frame::Full_Disk_Encrypt_frame(FrameProxyInterface* frameProxyInterface
                                                 , FullDiskDelegate * delegate, QWidget *parent)
    : ChildFrameInterface(frameProxyInterface, parent)
    , m_layout(new QVBoxLayout(this))
    , m_frameLbl(new TitleLabel(""))
    , m_frameSubLbl(new QLabel)
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
    m_layout->setSpacing(5);
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
        diskLbl->setFixedSize(QSize(40, 40));
        diskLbl->setPixmap(installer::renderPixmap(":/images/drive-harddisk-64px.svg").scaled(40, 40));
        m_diskinfo[i].m_diskLbl = diskLbl;
        m_diskinfo[i].m_devicePathLbl = new QLabel(this);
        m_diskinfo[i].m_devicePathLbl->setStyleSheet("QLabel{font-size: 14px;"
                                                     "text-align: center;"
                                                     "color: #001a2e;}");

        m_diskinfo[i].m_deviceSizeLbl = new QLabel(this);
        m_diskinfo[i].m_deviceSizeLbl->setStyleSheet("QLabel{font-size: 12px;"
                                                     "text-align: center;"
                                                     "color: #526a7f;}");
        QVBoxLayout *diskInfoLayout = new QVBoxLayout;
        diskInfoLayout->setMargin(0);
        diskInfoLayout->setSpacing(0);
        diskInfoLayout->addWidget(m_diskinfo[i].m_diskLbl, 0, Qt::AlignHCenter);
        diskInfoLayout->addSpacing(4);
        diskInfoLayout->addWidget(m_diskinfo[i].m_devicePathLbl, 0, Qt::AlignHCenter);
        diskInfoLayout->addSpacing(4);
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

    m_layout->addSpacing(5);
    m_layout->addWidget(spacingBar, 0, Qt::AlignHCenter);
    m_layout->addSpacing(5);

    // add encrypt input
    m_encryptLbl->setAlignment(Qt::AlignLeft);
    m_encryptLbl->setFixedSize(150, 36);

    m_encryptEdit->layout()->setContentsMargins(0, 0, 0, 0);
    m_encryptEdit->layout()->setSpacing(10);
    m_encryptEdit->setContentsMargins(0, 0, 0, 0);
    m_encryptEdit->setFixedSize(350, 36);
    m_encryptEdit->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    m_encryptEdit->lineEdit()->setMaxLength(kMaxPasswordLength);

    QHBoxLayout *encryptLayout = new QHBoxLayout;
    encryptLayout->setContentsMargins(0, 0, 0, 0);
    encryptLayout->setSpacing(0);
    encryptLayout->addWidget(m_encryptLbl, 0, Qt::AlignLeft | Qt::AlignVCenter);
    encryptLayout->addWidget(m_encryptEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    m_encryptFrame = new QFrame;
    m_encryptFrame->setLayout(encryptLayout);
    m_encryptFrame->setFixedWidth(kContentWidth);

    m_encryptCheckLbl->setAlignment(Qt::AlignLeft);
    m_encryptCheckLbl->setFixedSize(150, 36);

    m_encryptRepeatEdit->layout()->setContentsMargins(0, 0, 0, 0);
    m_encryptRepeatEdit->layout()->setSpacing(10);
    m_encryptRepeatEdit->setContentsMargins(0, 0, 0, 0);
    m_encryptRepeatEdit->setFixedSize(350, 36);
    m_encryptRepeatEdit->setFixedSize(350, 36);
    m_encryptRepeatEdit->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    //m_encryptRepeatEdit->setFocusPolicy(Qt::TabFocus);
    m_encryptRepeatEdit->lineEdit()->setMaxLength(kMaxPasswordLength);

    QHBoxLayout *encryptCheckLayout = new QHBoxLayout;
    encryptCheckLayout->setContentsMargins(0, 0, 0, 0);
    encryptCheckLayout->setSpacing(0);
    encryptCheckLayout->addWidget(m_encryptCheckLbl, 0, Qt::AlignLeft | Qt::AlignVCenter);
    encryptCheckLayout->addWidget(m_encryptRepeatEdit, 0, Qt::AlignRight | Qt::AlignVCenter);
    m_encryptCheckFrame = new QFrame;
    m_encryptCheckFrame->setLayout(encryptCheckLayout);
    m_encryptCheckFrame->setFixedWidth(kContentWidth);

    m_layout->addWidget(m_encryptFrame, 0, Qt::AlignHCenter);
    m_layout->addSpacing(10);
    m_layout->addWidget(m_encryptCheckFrame, 0, Qt::AlignHCenter);
    m_layout->addStretch();

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
    //m_cancelBtn->setFocusPolicy(Qt::TabFocus);

    m_confirmBtn->setFixedSize(NEXTBTN_WIDTH, NEXTBTN_HEIGHT);
    //m_confirmBtn->setFocusPolicy(Qt::TabFocus);

    m_frameSubLbl->setAlignment(Qt::AlignCenter);
    m_frameSubLbl->setWordWrap(true);
    m_frameSubLbl->adjustSize();

    QHBoxLayout* tipLayout = new QHBoxLayout();
    tipLayout->setContentsMargins(0, 0, 0, 0);
    tipLayout->setSpacing(0);
    tipLayout->addWidget(m_frameSubLbl);

    m_layout->addLayout(tipLayout);
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

bool Full_Disk_Encrypt_frame::focusSwitch()
{
    if (m_current_focus_widget == nullptr) {
        this->setCurentFocus(m_confirmBtn);
    } else if(m_confirmBtn == m_current_focus_widget) {
        this->setCurentFocus(m_cancelBtn);
    } else if (m_cancelBtn == m_current_focus_widget) {
        this->setCurentFocus(m_encryptEdit->lineEdit());
    } else if (m_encryptEdit->lineEdit() == m_current_focus_widget) {
        this->setCurentFocus(m_encryptRepeatEdit->lineEdit());
    } else if (m_encryptRepeatEdit->lineEdit() == m_current_focus_widget) {
        this->setCurentFocus(m_confirmBtn);
    }
    return true;
}

bool Full_Disk_Encrypt_frame::doSpace()
{
    return true;
}

bool Full_Disk_Encrypt_frame::doSelect()
{
    if (m_confirmBtn == m_current_focus_widget) {
        emit m_confirmBtn->clicked();
    } else if (m_cancelBtn == m_current_focus_widget) {
        emit m_cancelBtn->clicked();
    }

    return true;
}

bool Full_Disk_Encrypt_frame::directionKey(int keyvalue)
{
    return true;
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

void Full_Disk_Encrypt_frame::hideEvent(QHideEvent *event)
{
    m_encryptEdit->clear();
    m_encryptRepeatEdit->clear();

    return ChildFrameInterface::hideEvent(event);
}

void Full_Disk_Encrypt_frame::initConnections()
{
    connect(m_cancelBtn, &QPushButton::clicked, this, &Full_Disk_Encrypt_frame::cancel);
    connect(m_confirmBtn, &QPushButton::clicked, this, &Full_Disk_Encrypt_frame::onNextBtnClicked);
    connect(m_encryptEdit, &DLineEdit::textChanged, m_errTip, &SystemInfoTip::hide);
    connect(m_encryptRepeatEdit, &DLineEdit::textChanged, m_errTip, &SystemInfoTip::hide);
    connect(m_encryptEdit, &DLineEdit::focusChanged, this, &Full_Disk_Encrypt_frame::encryptEditOnFocus);
    connect(m_encryptRepeatEdit, &DLineEdit::focusChanged, this, &Full_Disk_Encrypt_frame::encryptRepeatEditOnFocus);

    connect(m_close_button, &DIconButton::clicked, this, &Full_Disk_Encrypt_frame::cancel);
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

    m_passwd = m_encryptEdit->text();

    emit encryptFinished();
}

QString Full_Disk_Encrypt_frame::passwd() {
    return m_passwd;
}

void Full_Disk_Encrypt_frame::getFinalDiskResolution(FinalFullDiskResolution& resolution) {
    m_diskPartitionDelegate->getFinalDiskResolution(resolution);
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
    m_frameSubLbl->setText(::QObject::tr("Take care of your password, otherwise, all your data will be lost")
                           + "\n"
                           + ::QObject::tr("Make sure you have backed up important data, then select the disk to install"));
    m_encryptLbl->setText(::QObject::tr("Password").append(" :"));
    m_encryptCheckLbl->setText(::QObject::tr("Repeat Password").append(" :"));
    m_cancelBtn->setText(::QObject::tr("Cancel", "button"));
    m_confirmBtn->setText(::QObject::tr("Confirm", "button"));
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
    m_close_button = new DIconButton(this);
    m_close_button->setFixedSize(50, 50);
    m_close_button->setIconSize(QSize(50, 50));
    m_close_button->setIcon(QIcon(":/images/close_normal.svg"));
    m_close_button->setFlat(true);
}

void Full_Disk_Encrypt_frame::encryptEditOnFocus(bool ison)
{
    if (ison) {
        this->setCurentFocus(m_encryptEdit->lineEdit());
    }
}

void Full_Disk_Encrypt_frame::encryptRepeatEditOnFocus(bool ison)
{
    if (ison) {
        this->setCurentFocus(m_encryptRepeatEdit->lineEdit());
    }
}
