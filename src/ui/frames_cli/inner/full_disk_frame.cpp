#include "full_disk_frame.h"
#include "ui/ncurses_widgets/ncurses_checkbox_list.h"
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

        QString strBack = ::QObject::tr("Back");
        QString strNext = ::QObject::tr("Next");

        m_pBackButton = new NcursesButton(this, strBack, 3, 14, begy() + height() - 5, begx() + 5);
        m_pBackButton->drawShadow(true);
        m_pBackButton->box();
        m_pBackButton->setObjectName(strBack);
        m_pBackButton->hide();

        m_label_title = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_title->setFocusEnabled(false);
        m_label_title->hide();

        m_label_systemdisk = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_systemdisk->setFocusEnabled(false);
        m_label_systemdisk->hide();

        m_systemdisklist = new NcursesCheckBoxList(this, (height() - 10) / 2, width() / 2, begy() + 2, begx() + width() / 4);
        m_systemdisklist->setListType(NcursesCheckBoxList::BASICENVIRONMENT);
        m_systemdisklist->setSingleSelect(true);
        m_systemdisklist->setRealSelect(true);
        m_systemdisklist->setSingleSelect(true);
        m_systemdisklist->hide();

        m_label_datadisk = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_datadisk->setFocusEnabled(false);
        m_label_datadisk->hide();

        m_datadisklist = new NcursesCheckBoxList(this, (height() - 10) / 2, width() / 2, begy() + 2, begx() + width() / 4);
        m_datadisklist->setListType(NcursesCheckBoxList::OTHER);
        m_datadisklist->setSingleSelect(true);
        m_datadisklist->hide();

        m_pNextButton = new NcursesButton(this, strNext, 3, 14, begy() + height() - 5, begx() + width() - 20);
        m_pNextButton->drawShadow(true);
        m_pNextButton->box();
        m_pNextButton->setObjectName(strNext);
        m_pNextButton->setFocus(false);


    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }
}

void FullDiskFramePrivate::layout()
{
    try {
        int beginY = begy();
        m_label_title->adjustSizeByContext();
        m_label_title->mvwin(beginY + 2, begx() + 1);

        m_label_systemdisk->adjustSizeByContext();
        m_label_systemdisk->mvwin(begy() + 4, begx() + width() / 4);
        m_systemdisklist->adjustSizeByContext();
        m_systemdisklist->mvwin(begy() + 5, begx() + width() / 4);

        m_label_datadisk->adjustSizeByContext();
        m_label_datadisk->mvwin(begy() + m_systemdisklist->height() + 7, begx() + width() / 4);
        m_datadisklist->adjustSizeByContext();
        m_datadisklist->mvwin(begy() + m_systemdisklist->height() + 8, begx() + width() / 4);

    } catch (NCursesException& e) {
         qCritical() << QString(e.message);
    }

}

void FullDiskFramePrivate::updateTs()
{
    printTitle(::QObject::tr("Full Disk"), width());
    m_label_title->setText("    " + ::QObject::tr("Make sure you have backed up important data, then select the disk to install."));
    m_label_systemdisk->setText(::QObject::tr("System Disk:"));
    m_label_datadisk->setText(::QObject::tr("Data Disk:"));
    FrameInterfacePrivate::updateTs();
    layout();
}

void FullDiskFramePrivate::initConnection()
{
    connect(m_pBackButton, &NcursesButton::clicked, this, &FullDiskFramePrivate::doBackBtnClicked);
    connect(m_pNextButton, &NcursesButton::clicked, this, &FullDiskFramePrivate::doNextBtnClicked);
    connect(m_systemdisklist, &NcursesCheckBoxList::signal_KeyTriger, this, &FullDiskFramePrivate::systemDisklistKeyTriger);
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
    emit doBackBtnClickedSignal();
}

void FullDiskFramePrivate::doNextBtnClicked()
{
    emit doNectBtnClickedSignal();
}

void FullDiskFramePrivate::setSystemDiskList(QStringList &info)
{
    m_deviceList.clear();
    m_deviceList = info;

    QVector<QPair<QString, QString>> testinfo;
    foreach (QString teststr, m_deviceList) {
        QPair<QString, QString> infoitem;
        infoitem.first = teststr;
        infoitem.second = teststr;
        testinfo.push_back(infoitem);
    }

    if (testinfo.size() > 0) {
        m_systemdisklist->setList(testinfo, false, false);
        systemDisklistKeyTriger(0, 0, 0);
    }

    if (m_isshow) {
        showListView();
    }
}

void FullDiskFramePrivate::setDataDiskList(QStringList &info)
{
    QVector<QPair<QString, QString>> testinfo;
    foreach (QString teststr, info) {
        QPair<QString, QString> infoitem;
        infoitem.first = teststr;
        infoitem.second = teststr;
        testinfo.push_back(infoitem);
    }

    if (testinfo.size() > 0) {
        m_datadisklist->setList(testinfo, false, false);
    }
}

void FullDiskFramePrivate::showListView()
{
    if(m_isshow) {
        m_systemdisklist->show();
        m_datadisklist->show();
    }
}

void FullDiskFramePrivate::setchildFoursEnabel(bool enabel)
{
    m_pBackButton->setFocusEnabled(enabel);
    m_systemdisklist->setFocusEnabled(enabel);
    m_datadisklist->setFocusEnabled(enabel);
    m_pNextButton->setFocusEnabled(enabel);
}

void FullDiskFramePrivate::keyPresseEvent(int keycode)
{
    if(!m_isshow) {
        if(m_currentchoicetype != -1){
            emit keyEventTrigerSignal(keycode);
        }
        return;
    } else {
        if(m_currentchoicetype != -1){
            emit keyEventTrigerSignal(keycode);
        } else {
            FrameInterfacePrivate::keyEventTriger(keycode);
            emit keyEventTrigerSignal(keycode);
        }
    }
}

void FullDiskFramePrivate::systemDisklistKeyTriger(int keycode, int listtype, int index)
{
    if (m_deviceList.size() > 0) {
        QVector<QPair<QString, QString>> testinfo;
        for (int i = 0; i < m_deviceList.size(); i++) {
            if (i != index) {
                QPair<QString, QString> infoitem;
                infoitem.first = m_deviceList.at(i);
                infoitem.second = m_deviceList.at(i);
                testinfo.push_back(infoitem);
            }
        }

        if( testinfo.size() > 0) {
            m_datadisklist->getSelectItems();
            m_datadisklist->setList(testinfo, false, false);
            if (m_isshow) {
                m_datadisklist->show();
            }

        }
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
    connect(d, &FullDiskFramePrivate::doBackBtnClickedSignal, this, &FullDiskFrame::doBackBtnClicked);
    connect(d, &FullDiskFramePrivate::doNectBtnClickedSignal, this, &FullDiskFrame::doNextBtnClicked);

    connect(d, &FullDiskFramePrivate::backToPreviousPage, [parent](){
        parent->hideAllChild();
        parent->show();
    });

    connect(d, &FullDiskFramePrivate::allIsFinished, [parent, this](){
        parent->setFrameState(FRAME_STATE_RUNNING);
        emit parent->getPrivate()->next();
        this->doFullDiskPartition();
    });


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
    if (!m_isShow) return false;

    Q_D(FullDiskFrame);
    if (d->getSystemDiskList()->size() > 0) {
        QString testname = d->getSystemDiskList()->getCurrentSingleSelectText();
        for (Device::Ptr device : m_DeviceList) {
            if(!testname.compare(GetDeviceModelCapAndPath(device))) {
                m_delegate->addSystemDisk(device->path);
                break;
            }
        }
    } else {
        return false;
    }

    if (d->getDataDiskList()->size() > 0) {
        QString testname_datadisk = d->getDataDiskList()->getCurrentSingleSelectText();
        for (Device::Ptr device : m_DeviceList) {
            if(!testname_datadisk.compare(GetDeviceModelCapAndPath(device))) {
                m_delegate->addDataDisk(device->path);
                break;
            }
        }
    }


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
    const qint64 root_required = GetSettingsInt(kPartitionRootMiniSpace);
    const qint64 root_required_bytes = kGibiByte * root_required;
    if (device->getByteLength() < root_required_bytes) {
        qWarning() << QString("MULTIDISK: disk too small:size:{%1}.").arg(device->getByteLength());
        return false;
    }

    if (!m_delegate->formatWholeDeviceMultipleDisk()) {
        qWarning() << "MULTIDISK: Failed to formatWholeDeviceMultipleDisk.";
        return false;
    }

//    if (!device.isEmpty() && isRawDevice(device)) {
//      dynamic_disk_warning_frame_->setDevice(device);
//      showDynamicDiskFrame();
//      return;
//    }

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
    m_delegate->setBootFlag();
    //OperationList operations = m_delegate->operations();
    //m_partitionModel->manualPart(operations);

    return true;
}

void FullDiskFrame::onManualPartDone(bool ok, const DeviceList &devices)
{
    if (!m_isShow) {
        return;
    }

    if (ok) {
        m_delegate->onManualPartDone(devices);
        Q_D(FullDiskFrame);
        emit d->startInstall();
    } else {
        qWarning() << Q_FUNC_INFO << "onManualPartDone failed";
    }
}

void FullDiskFrame::setShowEnable(bool isShow)
{
    m_isShow = isShow;
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
}

void FullDiskFrame::doBackBtnClicked()
{
    Q_D(FullDiskFrame);
    emit d->backToPreviousPage();
}

void FullDiskFrame::doNextBtnClicked()
{
    if (!doFullDiskPartition()) {
        return;
    }

    Q_D(FullDiskFrame);
    PrepareInstallFrame * pPrepareInstallFrame = new PrepareInstallFrame(d, d->height(), d->width(), d->begy(), d->begx(), m_delegate->getOptDescriptions());
    d->setCurrentchoicetype(1);
    pPrepareInstallFrame->hide();
    connect(d, &FullDiskFramePrivate::keyEventTrigerSignal, pPrepareInstallFrame, &PrepareInstallFrame::keyPresseEvent);
    connect(pPrepareInstallFrame, &PrepareInstallFrame::finished, this, [=](bool isOk){
        disconnect(d, &FullDiskFramePrivate::keyEventTrigerSignal, pPrepareInstallFrame, &PrepareInstallFrame::keyPresseEvent);
        d->removeChildWindows(pPrepareInstallFrame);
        show();
        d->setchildFoursEnabel(true);
        d->setCurrentchoicetype(-1);
        if(isOk) {
           emit d->allIsFinished();
            OperationList operations = m_delegate->operations();
            m_partitionModel->manualPart(operations);
        }

    });
    pPrepareInstallFrame->updateTs();
    d->setchildFoursEnabel(false);
    hide();
    pPrepareInstallFrame->show();
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
