#include "full_disk_frame.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/models/partition_model.h"
#include "ui/delegates/full_disk_delegate.h"
#include "ui/frames_cli/inner/prepare_install_frame.h"


namespace installer {

void FullDiskFramePrivate::initUI()
{
    try {
        //FrameInterfacePrivate::initUI();
        setBackground(NcursesUtil::getInstance()->dialog_attr());
        this->drawShadow(true);
        this->box();

        QString strBack = QObject::tr("back");
        QString strNext = QObject::tr("next");

        int buttonHeight = 3;
        int buttonWidth = std::max(strNext.length(), strBack.length()) + 4;

        m_pBackButton = new NcursesButton(this, strBack, buttonHeight,
                                          buttonWidth, begy() + height() - buttonHeight - 2, begx() + 5);
        m_pBackButton->drawShadow(true);
        m_pBackButton->box();
        m_pBackButton->setObjectName(strBack);

        m_pNextButton = new NcursesButton(this, strNext, buttonHeight,
                                        buttonWidth, begy() + height() - buttonHeight - 2, begx() + width() - buttonWidth - 13);
        m_pNextButton->drawShadow(true);
        m_pNextButton->box();
        m_pNextButton->setObjectName(strNext);


        m_label_title = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_title->setFocusEnabled(false);

        m_label_systemdisk = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_systemdisk->setFocusEnabled(false);

        m_systemdisklist = new NcursesListView(this, (height() - 10) / 2, width() / 2, begy() + 2, begx() + width() / 4);

        m_label_datadisk = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_datadisk->setFocusEnabled(false);

        m_datadisklist = new NcursesListView(this, (height() - 10) / 2, width() / 2, begy() + 2, begx() + width() / 4);

    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }
}

void FullDiskFramePrivate::layout()
{
    try {
        int beginY = begy();
        m_label_title->adjustSizeByContext();
        m_label_title->mvwin(beginY, begx() + (width() - m_label_title->width()) / 2);

        m_label_systemdisk->adjustSizeByContext();
        m_label_systemdisk->mvwin(begy() + 2, begx() + width() / 4);
        m_systemdisklist->adjustSizeByContext();
        m_systemdisklist->mvwin(begy() + 3, begx() + width() / 4);

        m_label_datadisk->adjustSizeByContext();
        m_label_datadisk->mvwin(begy() + m_systemdisklist->height() + 5, begx() + width() / 4);
        m_datadisklist->adjustSizeByContext();
        m_datadisklist->mvwin(begy() + m_systemdisklist->height() + 6, begx() + width() / 4);

    } catch (NCursesException& e) {
         qCritical() << QString(e.message);
    }

}

void FullDiskFramePrivate::updateTs()
{
    m_label_title->setText(tr("full disk"));
    m_label_systemdisk->setText(tr("system disk:"));
    m_label_datadisk->setText(tr("data disk:"));
    FrameInterfacePrivate::updateTs();
    layout();
}

void FullDiskFramePrivate::initConnection()
{
    connect(m_pBackButton, &NcursesButton::clicked, this, &FullDiskFramePrivate::doBackBtnClicked);
    connect(m_pNextButton, &NcursesButton::clicked, this, &FullDiskFramePrivate::doNextBtnClicked);
}

bool FullDiskFramePrivate::validate()
{
    return true;
}

void FullDiskFramePrivate::show()
{
    if(!m_isshow) {
        NCursesWindowBase::show();
        m_isshow = true;
    }
}

void FullDiskFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void FullDiskFramePrivate::doBackBtnClicked()
{
    emit backToPreviousPage();
}

void FullDiskFramePrivate::doNextBtnClicked()
{
    emit allIsFinished();
}

void FullDiskFramePrivate::setSystemDiskList(QStringList &info)
{
    m_systemdisklist->setList(info);

}

void FullDiskFramePrivate::setDataDiskList(QStringList &info)
{
    m_datadisklist->setList(info);
}

void FullDiskFramePrivate::showListView()
{
    if(m_isshow) {
        m_systemdisklist->show();
        m_datadisklist->show();
    }
}

void FullDiskFramePrivate::keyPresseEvent(int keycode)
{
    if(!m_isshow) {
        return;
    }
    if(m_currentchoicetype != -1){
        emit keyEventTrigerSignal(keycode);
    } else {
        FrameInterfacePrivate::keyEventTriger(keycode);
        emit keyEventTrigerSignal(keycode);
    }
}


FullDiskFrame::FullDiskFrame(FrameInterface* parent, PartitionModel* model)
    : m_partitionModel(model),
      m_prepareInstallFrame(nullptr)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new FullDiskFramePrivate(nullptr, h, w, beginY, beginX);
    m_delegate = new FullDiskDelegate(this);
    //m_prepareInstallFrame = new PrepareInstallFrame(nullptr, h, w, beginY, beginX);

    Q_D(FullDiskFrame);
    connect(parent->getPrivate(), SIGNAL(keyEventTrigerSignal(int)), d, SLOT(keyPresseEvent(int)));
    connect(m_delegate, &FullDiskDelegate::deviceRefreshed, this, &FullDiskFrame::onDeviceRefreshed);
    connect(m_partitionModel, &PartitionModel::deviceRefreshed, m_delegate, &FullDiskDelegate::onDeviceRefreshed);
    connect(m_partitionModel, &PartitionModel::manualPartDone,this, &FullDiskFrame::onManualPartDone);

    connect(d, &FullDiskFramePrivate::backToPreviousPage, [parent](){
        parent->hideAllChild();
        parent->show();
    });
    connect(d, &FullDiskFramePrivate::allIsFinished, [parent](){
        parent->setFrameState(FRAME_STATE_RUNNING);
        emit parent->getPrivate()->next();
    });
    connect(d, &FullDiskFramePrivate::allIsFinished, this, &FullDiskFrame::doFullDiskPartition);
}

FullDiskFrame::~FullDiskFrame()
{

}

bool FullDiskFrame::init()
{
    if (m_currState == FRAME_STATE_NOT_START) {
        readConf();
        m_private->layout();
    }
    return true;
}

QString FullDiskFrame::getFrameName()
{
    return "FullDiskFrame";
}

bool FullDiskFrame::doFullDiskPartition()
{
    Q_D(FullDiskFrame);
    QString testname = d->getSystemDiskList()->getCurrenItem();
    for (Device::Ptr device : m_DeviceList) {
        if(!testname.compare(GetDeviceModelCapAndPath(device))) {
            m_delegate->addSystemDisk(device->path);
            break;
        }
    }
    //m_errorTip->hide();
    //m_diskTooSmallTip->hide();
    if (m_delegate->selectedDisks().isEmpty()) {
        //m_errorTip->show();
        return false;
    }

    int index = DeviceIndex(m_delegate->virtualDevices(), m_delegate->selectedDisks()[0]);
    if (index < 0) {
        qWarning() << QString("MULTIDISK:DeviceIndex failed:{%1}").arg(m_delegate->selectedDisks()[0]);
        return false;
    }

    Device::Ptr device(new Device(*m_delegate->virtualDevices()[index]));
    const qint64 root_required = GetSettingsInt(kPartitionMinimumDiskSpaceRequired);
    const qint64 root_required_bytes = kGibiByte * root_required;
    if (device->getByteLength() < root_required_bytes) {
        //m_diskTooSmallTip->show();
        qWarning() << QString("MULTIDISK: disk too small:size:{%1}.").arg(device->getByteLength());
        return false;
    }

    if (!m_delegate->formatWholeDeviceMultipleDisk()) {
        //m_diskTooSmallTip->show();
        qWarning() << "MULTIDISK: Failed to formatWholeDeviceMultipleDisk.";
        return false;
    }

//    if (!device.isEmpty() && isRawDevice(device)) {
//      dynamic_disk_warning_frame_->setDevice(device);
//      showDynamicDiskFrame();
//      return;
//    }

    QStringList descriptions = m_delegate->getOptDescriptions();
    qDebug() << "descriptions: " << descriptions;

    //prepare_install_frame_->updateDescription(descriptions);
    //main_layout_->setCurrentWidget(prepare_install_frame_);

    /*// First, update boot flag.
    bool found_boot;
    if (isSimplePartitionMode()) {
        found_boot = simple_partition_delegate_->setBootFlag();
    } else if (isFullDiskPartitionMode() && !full_disk_partition_frame_->isEncrypt()){
        found_boot = full_disk_delegate_->setBootFlag();
    } else if (AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
        found_boot = advanced_delegate_->setBootFlag();
    } else if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Install) {
        found_boot = lvm_delegate_->setBootFlag();
    }

    if (!found_boot && !isFullDiskPartitionMode() &&AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
        qCritical() << "No boot partition found, we shall never reach here!";
        return;
    }*/

    // Get operation list.

    // full disk encrypt operations is empty.
    OperationList operations = m_delegate->operations();
    m_partitionModel->manualPart(operations);

    return true;
}

void FullDiskFrame::onManualPartDone(bool ok, const DeviceList &devices)
{
    Q_D(FullDiskFrame);
    emit d->startInstall();
}

void FullDiskFrame::onDeviceRefreshed(const DeviceList& devices)
{
    m_DeviceList = devices;
    Q_D(FullDiskFrame);
    QStringList listtestpath;
    for (Device::Ptr device : devices) {
        listtestpath.append(GetDeviceModelCapAndPath(device));
    }
    d->setSystemDiskList(listtestpath);
    d->setDataDiskList(listtestpath);
    d->showListView();
}

void FullDiskFrame::readConf()
{

}

void FullDiskFrame::writeConf()
{

}

bool FullDiskFrame::handle()
{
    m_private->keyHandle();
    writeConf();
    return true;
}


}
