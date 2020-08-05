#include "advanced_partition_frame.h"
#include "ui/ncurses_widgets/ncurses_label.h"
#include "ui/ncurses_widgets/ncurses_list_view.h"
#include "new_partition_frame.h"
#include "prepare_install_frame.h"

#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/models/partition_model.h"
#include "ui/delegates/advanced_partition_delegate.h"
#include "ui/delegates/lvm_partition_delegate.h"
#include "partition_table_warning_frame.h"
#include "base/command.h"

namespace installer {

void AdvancedPartitionFramePrivate::initUI()
{
    try {
        //FrameInterfacePrivate::initUI();
        setBackground(NcursesUtil::getInstance()->dialog_attr());
        this->drawShadow(true);
        this->box();

        QString strBack = ::QObject::tr("Back");
        QString strNext = ::QObject::tr("Next");

        //int buttonHeight = 3;
        //int buttonWidth = std::max(strNext.length(), strBack.length()) + 4;

        m_pBackButton = new NcursesButton(this, strBack, 3, 14, begy() + height() - 5, begx() + 5);
        m_pBackButton->drawShadow(true);
        m_pBackButton->box();
        m_pBackButton->setObjectName(strBack);
        m_pBackButton->setFocus(false);

        m_label_title = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_title->setFocusEnabled(false);

        m_label_tips = new NcursesLabel(this, 1, 1, begy(), begx());
        m_label_tips->setFocusEnabled(false);

        m_listViewPartitionMode = new NcursesListView(this, height() / 2 , width() / 2, begy(), begx() + width() / 4);
        m_listViewPartitionMode->setFocus(true);

        m_msgHeadLabel = new NcursesLabel(this, 1, width() - width() / 4 - 1, begy() + 2, begx());
        m_msgHeadLabel->setFocusEnabled(false);
        m_msgHeadLabel->setBackground(NcursesUtil::getInstance()->error_attr());

        m_errorLabel = new NcursesListView(this, 4, width() - width() / 4 - 1, begy(), begx() + width() / 4);
        m_errorLabel->setFocusEnabled(false);
        m_errorLabel->setSeelectMode(false);
        m_errorLabel->setBackground(NcursesUtil::getInstance()->error_attr());

        m_pNextButton = new NcursesButton(this, strNext, 3, 14, begy() + height() - 5, begx() + width() - 20);
        m_pNextButton->drawShadow(true);
        m_pNextButton->box();
        m_pNextButton->setObjectName(strNext);
        m_pNextButton->setFocus(false);

    } catch (NCursesException& e) {
        qCritical() << QString(e.message);
    }
}

void AdvancedPartitionFramePrivate::layout()
{
    try {
        int beginY = begy() + 2;
        m_label_title->mvwin(beginY, begx() + 1);
        m_label_title->adjustSizeByContext();

        beginY += m_label_title->height();
        m_label_tips->mvwin(beginY, begx() + 1);
        m_label_tips->adjustSizeByContext();

        beginY += m_label_tips->height();
        m_msgHeadLabel->mvwin(beginY, begx() + width() / 4 - 1);
        m_msgHeadLabel->adjustSizeByContext();

        beginY += m_msgHeadLabel->height();
        m_errorLabel->mvwin(beginY, begx() + width() / 4 - 1);

        beginY += m_errorLabel->height();
        m_listViewPartitionMode->mvwin(beginY + 3, begx() + (width() - m_listViewPartitionMode->width()) / 2);
        m_listViewPartitionMode->adjustSizeByContext();

    } catch (NCursesException& e) {
         qCritical() << QString(e.message);
    }
}

void AdvancedPartitionFramePrivate::updateTs()
{
    printTitle(::QObject::tr("Advanced"), width());
    m_label_title->setText("    " + ::QObject::tr("Make sure you have backed up important data, then select the disk to install."));
    m_label_tips->setText("    " + ::QObject::tr("Insert key: create or edit partition. Delete key: delete partition."));
    FrameInterfacePrivate::updateTs();
    layout();
}

void AdvancedPartitionFramePrivate::initConnection()
{
    connect(m_pBackButton, &NcursesButton::clicked, this, &AdvancedPartitionFramePrivate::doBackBtnClicked);
    connect(m_pNextButton, &NcursesButton::clicked, this, &AdvancedPartitionFramePrivate::doNextBtnClicked);
}

bool AdvancedPartitionFramePrivate::validate()
{
    return true;
}

void AdvancedPartitionFramePrivate::show()
{
    if(!m_isshow) {
        NCursesWindowBase::show();
        m_isshow = true;
    }
}

void AdvancedPartitionFramePrivate::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}

void AdvancedPartitionFramePrivate::onDeviceRefreshed(const DeviceList& devices)
{
    m_devices = devices;
    QStringList list;
    for (Device::Ptr device : devices) {
        list.append(GetDeviceModelCapAndPath(device));
        for (Partition::Ptr partition : device->partitions) {
            int gibi_size = ToGigByte(partition->getByteLength());
            QString diskUnit = "G";
            if (gibi_size == 0 ) {
                diskUnit ="M";
                gibi_size = ToMebiByte(partition->getByteLength());
            }
            QString freeDiskUnit = "G";
            int free_gibi_size = ToGigByte(partition->freespace);
            if (free_gibi_size == 0 ) {
                freeDiskUnit ="M";
                free_gibi_size = ToMebiByte(partition->freespace);
            }
            QString name(QFileInfo(partition->path).fileName());
            QString freeflag(partition->busy?"busy":"free");
            list.append( QString("|______%1(%2 %3)(%4%5)(%6)")
                         .arg(GetFsTypeName(partition->fs))
                         .arg(freeflag)
                         .arg(partition->mount_point)
                         .arg(gibi_size)
                         .arg(diskUnit)
                         .arg(name));
        }
    }

    m_listViewPartitionMode->setList(list);
    if (m_isshow) {
        m_listViewPartitionMode->show();
        m_listViewPartitionMode->setFocus(true);
    }
}

void AdvancedPartitionFramePrivate::setMsgHeadLable(QString& msg)
{
    m_msgHeadLabel->setText(msg);
    if (m_isshow) {
        m_msgHeadLabel->show();
    }
}

void AdvancedPartitionFramePrivate::setErrorLable(QStringList& error)
{
    m_errorLabel->setList(error);
    if (m_isshow) {
        m_errorLabel->show();
    }
}

void AdvancedPartitionFramePrivate::onKeyPress(int keycode)
{
    Partition::Ptr partition(getCurrentPartition());
    if(!partition) return ;
    if(partition->busy) return ;

    switch (keycode) {
    case KEY_IC:       
        if(partition->partition_number > -1) return ;
         m_currentchoicetype = 1;
        emit newPartition(partition);
        break;
    case KEY_DC:
        if(partition->partition_number < 0) return ;
        emit deletePartition(partition);
        break;
    }

}

void AdvancedPartitionFramePrivate::doBackBtnClicked()
{
    emit this->doBackBtnClickedSignal();
}

void AdvancedPartitionFramePrivate::doNextBtnClicked()
{
    emit this->doNectBtnClickedSignal();
}

void AdvancedPartitionFramePrivate::setchildFoursEnabel(bool enabel)
{
    m_pBackButton->setFocusEnabled(enabel);
    m_listViewPartitionMode->setFocusEnabled(enabel);
    m_pNextButton->setFocusEnabled(enabel);
}

void AdvancedPartitionFramePrivate::setCurrentchoicetype(int state)
{
    m_currentchoicetype = state;
}

void AdvancedPartitionFramePrivate::keyPresseEvent(int keycode)
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


Partition::Ptr AdvancedPartitionFramePrivate::getCurrentPartition()
{
    QStringList list;
    int indexItem(0);
    for (Device::Ptr device : m_devices) {
        indexItem++;
        for (Partition::Ptr partition : device->partitions) {
            if (indexItem == m_listViewPartitionMode->getCurrentIndex())
                return partition;
            indexItem++;
        }
    }
   return nullptr;
}

AdvancedPartitionFrame::AdvancedPartitionFrame(FrameInterface* parent,PartitionModel* model)
    : m_partitionModel(model)
{
    int h = LINES / 2;
    int w = COLS / 2;
    int beginY = (LINES - h - 2) / 2;
    int beginX = (COLS - w) / 2;
    m_private = new AdvancedPartitionFramePrivate(nullptr, h, w, beginY, beginX);

    m_delegate = new AdvancedPartitionDelegate(this);
    m_lvmDelegate = new LvmPartitionDelegate(this);
    m_currentDelegate =  m_delegate;
    Q_D(AdvancedPartitionFrame);
    connect(parent->getPrivate(), SIGNAL(keyEventTrigerSignal(int)), d, SLOT(keyPresseEvent(int)));
    connect(m_delegate, &AdvancedPartitionDelegate::deviceRefreshed, this, &AdvancedPartitionFrame::onDeviceRefreshed);
    connect(m_lvmDelegate, &LvmPartitionDelegate::deviceRefreshed, this, &AdvancedPartitionFrame::onDeviceRefreshed);
    connect(m_partitionModel, &PartitionModel::manualPartDone,this, &AdvancedPartitionFrame::onManualPartDone);
    connect(m_partitionModel, &PartitionModel::deviceRefreshed, m_delegate, &AdvancedPartitionDelegate::onDeviceRefreshed);
    connect(d, &AdvancedPartitionFramePrivate::newPartition, this, &AdvancedPartitionFrame::onNewPartition);
    connect(d, &AdvancedPartitionFramePrivate::deletePartition, this, &AdvancedPartitionFrame::onDeletePartitionTriggered);

    connect(d, &AdvancedPartitionFramePrivate::doBackBtnClicked, this, &AdvancedPartitionFrame::doBackBtnClicked);
    connect(d, &AdvancedPartitionFramePrivate::doBackBtnClickedSignal, this, &AdvancedPartitionFrame::doBackBtnClicked);
    connect(d, &AdvancedPartitionFramePrivate::doNectBtnClickedSignal, this, &AdvancedPartitionFrame::doNextBtnClicked);
    connect(d, &AdvancedPartitionFramePrivate::backToPreviousPage, [parent](){
        parent->hideAllChild();
        parent->show();
    });
    connect(d, &AdvancedPartitionFramePrivate::allIsFinished, [parent](){
        parent->setFrameState(FRAME_STATE_RUNNING);
        emit parent->getPrivate()->next();
    });
}

AdvancedPartitionFrame::~AdvancedPartitionFrame()
{

}

bool AdvancedPartitionFrame::init()
{
    if (m_currState == FRAME_STATE_NOT_START) {
        readConf();
        m_private->layout();
    }
    return true;
}

QString AdvancedPartitionFrame::getFrameName()
{
    return "AdvancedPartitionFrame";
}

void AdvancedPartitionFrame::clearErrorMessages() {
  m_validateStates.clear();
}

bool AdvancedPartitionFrame::validate() {
  // Clear error messages first.
  this->clearErrorMessages();

  m_validateStates = m_currentDelegate->validate();
  if (m_validateStates.isEmpty()) {
    return true;
  } else {
    this->showErrorMessages();

    return false;
  }
}

QString AdvancedPartitionFrame::validateStateToText(ValidateState state) {
  switch (state) {
    case ValidateState::BootFsInvalid: {
        const FsTypeList boot_fs_list = m_currentDelegate->getBootFsTypeList();
        QStringList fs_name_list;
        for (const FsType& fs_type : boot_fs_list) {
          fs_name_list.append(GetFsTypeName(fs_type));
        }
        const QString fs_name(fs_name_list.join("/"));
        return ::QObject::tr("The partition filesystem type of /boot directory "
              "can only be %1 ").arg(fs_name);
    }
    case ValidateState::BootPartNumberInvalid: {
        return ::QObject::tr("The partition of /boot directory should be "
              "the first partition on hard disk");
    }
    case ValidateState::EfiPartNumberinvalid: {
        return ::QObject::tr("The partition of /boot/efi directory should be "
              "the first partition on hard disk");
    }
    case ValidateState::BootTooSmall: {
        const int boot_recommended = GetSettingsInt(kPartitionDefaultBootSpace);
        return ::QObject::tr("/boot partition requires at least %1 MB")
            .arg( boot_recommended);
    }
    case ValidateState::BootBeforeLvm: {
        return ::QObject::tr("To create lvm, /boot partition is required");
    }
    case ValidateState::EfiMissing: {
        return ::QObject::tr("Add an EFI partition to continue");
    }
    case ValidateState::EfiTooSmall: {
        const int efi_recommended = GetSettingsInt(kPartitionDefaultEFISpace);
        return ::QObject::tr("/efi partition requires at least %1 MB")
            .arg(efi_recommended);
    }
    case ValidateState::RootMissing: {
        return ::QObject::tr("Add a root partition to continue");
    }
    case ValidateState::RootTooSmall: {
        const int root_required =
            GetSettingsInt(kPartitionRootMiniSpace);
        return ::QObject::tr("/root partition requires at least %1 GB")
            .arg(root_required);
    }
    case ValidateState::PartitionTooSmall: {
        const int partition_min_size_by_gb = GetSettingsInt(kPartitionOthersMinimumSize);
        return ::QObject::tr("%2 partition requires at least %1 GB")
            .arg(partition_min_size_by_gb)
            .arg(GetPartitionName(state->partition()->path));
    }
    case ValidateState::LvmPartNumberInvalid: {
        return ::QObject::tr("Add a logical partition to continue");
    }

    default: {
      // We shall never reach here.
      return "";
    }
  }
}

void AdvancedPartitionFrame::showErrorMessages() {
  QStringList errList;
  for (ValidateState state: m_validateStates) {
      errList.append(this->validateStateToText(state));
  }

  Q_D(AdvancedPartitionFrame);
  d->setErrorLable(errList);
  this->updateErrorMessageHeader();
}

void AdvancedPartitionFrame::updateErrorMessageHeader() {
  // Update error message header.
  const int err_count = m_validateStates.length();
  QString errMsg;
  // NOTE(xushaohua): Transifex does not ts plural format.
  if (!GetSettingsBool(kPartitionSkipSimplePartitionPage)) {
    if (err_count <= 1) {
      errMsg = ::QObject::tr("%1 error found, fix to continue installation or "
             "switch to simple mode").arg(err_count);
    } else {
      errMsg = ::QObject::tr("%1 errors found, fix to continue installation or "
             "switch to simple mode").arg(err_count);
    }
  } else {
    if (err_count <= 1) {
      errMsg = ::QObject::tr("%1 error found, fix to continue installation")
              .arg(err_count);
    } else {
      errMsg = ::QObject::tr("%1 error found, fix to continue installation")
              .arg(err_count);
    }
  }

  Q_D(AdvancedPartitionFrame);
  d->setMsgHeadLable(errMsg);
}

void AdvancedPartitionFrame::onDeviceRefreshed(const DeviceList& devices)
{
    Q_D(AdvancedPartitionFrame);
    QStringList err("");
    d->setErrorLable(err);
    QString msg("");
    d->setMsgHeadLable(msg);
    d->onDeviceRefreshed(devices);
}

void AdvancedPartitionFrame::onNewPartition(const Partition::Ptr& partition) {
    Q_D(AdvancedPartitionFrame);
    const QString device_path = partition->device_path;
    if (! m_currentDelegate->isPartitionTableMatch(device_path)) {
      requestNewTable(device_path);
      return;
    }

    if (m_currentDelegate->canAddPrimary(partition) ||
        m_currentDelegate->canAddLogical(partition)) {
      // Switch to NewPartitionFrame only if a new partition can be added.
        NewPartitionFrame * pNewPartitionFrame = new NewPartitionFrame(d, d->height(), d->width(), d->begy(), d->begx(), m_currentDelegate);
        pNewPartitionFrame->hide();
        connect(d, &AdvancedPartitionFramePrivate::keyEventTrigerSignal, pNewPartitionFrame, &NewPartitionFrame::keyPresseEvent);
        connect(pNewPartitionFrame, &NewPartitionFrame::finished,this, [=](){
            disconnect(d, &AdvancedPartitionFramePrivate::keyEventTrigerSignal, pNewPartitionFrame, &NewPartitionFrame::keyPresseEvent);
            d->removeChildWindows(pNewPartitionFrame);
            show();
            d->setchildFoursEnabel(true);
            d->setCurrentchoicetype(-1);
        });
        pNewPartitionFrame->setPartition(partition);
        pNewPartitionFrame->updateTs();
        d->setchildFoursEnabel(false);
        hide();
        pNewPartitionFrame->show();
    } else {
      qWarning() << "Can not add new partition any more" << partition;
      requestPartitionNumberLimitationFrame();
    }

}

void AdvancedPartitionFrame::onDeletePartitionTriggered(
    const Partition::Ptr partition) {

    removeOsProberDataByPath(partition->path);
    m_currentDelegate->deletePartition(partition);
    m_currentDelegate->refreshVisual();

}

void AdvancedPartitionFrame::doBackBtnClicked()
{
    if (m_currentDelegate == m_lvmDelegate) {
        m_delegate->resetOperations();
        m_lvmDelegate->resetOperations();
        m_currentDelegate = m_delegate;
        if (VgDevice::p_installer_VgDevice) {
            //由于智能指针会自动释放，此处不需要手动删除
            VgDevice::p_installer_VgDevice = nullptr;
        }
        AdvancedPartitionDelegate::install_Lvm_Status = Install_Lvm_Status::Lvm_No_Need;
        DeviceList devices= m_delegate->realDevices();
        //删除当前盘符
        for (Device::Ptr device : devices) {
            QStringList args("-h");
            QString outPut ="";
            args.append("/usr/lib/live/mount/medium");
            args.append("| grep");
            args.append(device->path);
            SpawnCmd("df", args, outPut);

            if (outPut.indexOf(device->path) < 0) continue;

            devices.removeOne(device);
            break;
        }

        emit m_currentDelegate->deviceRefreshed(devices);
        Q_D(AdvancedPartitionFrame);
        d->box();
        d->printTitle(::QObject::tr("Advanced"), d->width());
        d->refresh();
    } else {
        Q_D(AdvancedPartitionFrame);
        emit d->backToPreviousPage();
    }
}

void AdvancedPartitionFrame::doNextBtnClicked()
{
    if (!validate()) {
      return;
    }

    // check disk is raw
    QList<Device::Ptr> device;

    device = m_currentDelegate->getAllUsedDevice();


    if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Format_Pv) {
        Q_D(AdvancedPartitionFrame);
        d->printTitle(::QObject::tr("Edit LVM Disk"), d->width());
        onPrepareInstallFrameFinished();
    } else {
        Q_D(AdvancedPartitionFrame);

        PrepareInstallFrame * pPrepareInstallFrame = new PrepareInstallFrame(d, d->height(), d->width(), d->begy(), d->begx(), m_currentDelegate->getOptDescriptions());
        d->setCurrentchoicetype(1);
        pPrepareInstallFrame->hide();
        connect(d, &AdvancedPartitionFramePrivate::keyEventTrigerSignal, pPrepareInstallFrame, &PrepareInstallFrame::keyPresseEvent);
        connect(pPrepareInstallFrame, &PrepareInstallFrame::finished,this, [=](bool isOk){
            disconnect(d, &AdvancedPartitionFramePrivate::keyEventTrigerSignal, pPrepareInstallFrame, &PrepareInstallFrame::keyPresseEvent);
            d->removeChildWindows(pPrepareInstallFrame);
            show();
            d->setchildFoursEnabel(true);
            d->setCurrentchoicetype(-1);
            if(isOk) {
               onPrepareInstallFrameFinished();
               emit d->allIsFinished();
            }

        });
        pPrepareInstallFrame->updateTs();
        d->setchildFoursEnabel(false);
        hide();
        pPrepareInstallFrame->show();
    }

    /*
    dynamic_disk_warning_frame_->setWarningTip(::QObject::tr("The target disk is dynamic which will be formatted if proceeding. Please make a backup of your important files first."));


    if (!device.isEmpty() && isRawDevice(device)) {
      dynamic_disk_warning_frame_->setDevice(device);
      showDynamicDiskFrame();
      return;
    }

    if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_No_Need) {
        prepare_install_frame_->setInstallLvmTitel(false);
    } else {
        prepare_install_frame_->setInstallLvmTitel(true);
    }

      showPrepareInstallFrame();
    */
}

void AdvancedPartitionFrame::onPrepareInstallFrameFinished()
{
    if (!m_isShow) return ;
    bool found_boot = false;
    if (AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
        found_boot = m_delegate->setBootFlag();
    } else if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Install) {
        found_boot = m_lvmDelegate->setBootFlag();
    }

    if (!found_boot && AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
        qCritical() << "No boot partition found, we shall never reach here!";
        return;
    }

    // Get operation list.
    OperationList operations;
   if (AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install){
        operations = m_delegate->operations();
    } else if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Install){
        //注意一定要先添加老分区操作，否则会产生不可预料的后果
        operations = m_lvmDelegate->m_oldOperationList;
        operations.append(m_lvmDelegate->operations());
    }

   if (operations.isEmpty()) {
        qCritical() << "Operation list is empty";
        return;
    }
    else {
        m_partitionModel->manualPart(operations);
        if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Format_Pv) {
            m_lvmDelegate->m_oldOperationList = operations;
        }
    }
}

void AdvancedPartitionFrame::onManualPartDone(bool ok, const DeviceList& devices)
{
  if (!m_isShow) return ;
  if (ok) {

   if (Install_Lvm_Status::Lvm_Format_Pv == AdvancedPartitionDelegate::install_Lvm_Status) {
       m_currentDelegate =  m_lvmDelegate;
       m_lvmDelegate->onLvmPartDone(ok,devices);
        return ;
    } else if (Install_Lvm_Status::Lvm_Install == AdvancedPartitionDelegate::install_Lvm_Status) {
        m_lvmDelegate->onManualPartDone(devices);
    } else {
        m_delegate->onManualPartDone(devices);
    }
  }

  //runhook;
  Q_D(AdvancedPartitionFrame);
  emit d->startInstall();
}


void AdvancedPartitionFrame::setShowEnable( bool isShow) {
    m_isShow = isShow;
}

void AdvancedPartitionFrame::requestNewTable(const QString& device_path) {
    Q_D(AdvancedPartitionFrame);
    PartitionTableWarningFrame * partitionTableWarningFrame = new PartitionTableWarningFrame(d, d->height(), d->width(), d->begy(), d->begx(), m_partitionModel);
    partitionTableWarningFrame->hide();
    connect(d, &AdvancedPartitionFramePrivate::keyEventTrigerSignal, partitionTableWarningFrame, &PartitionTableWarningFrame::keyPresseEvent);
    connect(partitionTableWarningFrame, &PartitionTableWarningFrame::finished,this, [=](){
        disconnect(d, &AdvancedPartitionFramePrivate::keyEventTrigerSignal, partitionTableWarningFrame, &PartitionTableWarningFrame::keyPresseEvent);
        d->removeChildWindows(partitionTableWarningFrame);
        show();
        d->setchildFoursEnabel(true);
        d->setCurrentchoicetype(-1);        
    });
    partitionTableWarningFrame->setDevicePath(device_path);
    partitionTableWarningFrame->updateTs();
    d->setchildFoursEnabel(false);
    hide();
    partitionTableWarningFrame->show();
}

void AdvancedPartitionFrame::requestPartitionNumberLimitationFrame() {

}

void AdvancedPartitionFrame::readConf()
{

}

void AdvancedPartitionFrame::writeConf()
{

}

bool AdvancedPartitionFrame::handle()
{
    m_private->keyHandle();
    writeConf();
    return true;
}

}
