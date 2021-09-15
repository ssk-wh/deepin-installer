/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/frames/partition_frame.h"

#include "base/file_util.h"
#include "service/settings_name.h"
#include "ui/delegates/advanced_partition_delegate.h"
#include "ui/delegates/lvm_partition_delegate.h"
#include "ui/delegates/full_disk_delegate.h"
#include "ui/delegates/simple_partition_delegate.h"
#include "ui/frames/dynamic_disk_warning_frame.h"
#include "ui/delegates/partition_util.h"
#include "ui/frames/consts.h"
#include "ui/frames/inner/advanced_partition_frame.h"
#include "ui/frames/inner/lvm_partition_frame.h"
#include "ui/frames/inner/edit_partition_frame.h"
#include "ui/frames/inner/full_disk_frame.h"
#include "ui/frames/inner/new_partition_frame.h"
#include "ui/frames/inner/new_table_loading_frame.h"
#include "ui/frames/inner/new_table_warning_frame.h"
#include "ui/frames/inner/partition_loading_frame.h"
#include "ui/frames/inner/partition_number_limitation_frame.h"
#include "ui/frames/inner/partition_table_warning_frame.h"
#include "ui/frames/inner/prepare_install_frame.h"
#include "ui/frames/inner/select_bootloader_frame.h"
#include "ui/frames/inner/simple_partition_frame.h"
#include "ui/frames/inner/full_disk_encrypt_frame.h"
#include "ui/frames/warnning_frame.h"
#include "ui/models/partition_model.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/title_label.h"
#include "ui/widgets/pointer_button.h"
#include "ui/interfaces/frameinterfaceprivate.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"

#include <QButtonGroup>
#include <QEvent>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QProcess>
#include <QTextStream>
#include <QApplication>
#include <DButtonBox>

DWIDGET_USE_NAMESPACE

namespace installer {

namespace {

}  // namespace

class PartitionFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT

    friend PartitionFrame;

public:
    explicit PartitionFramePrivate(FrameInterface* parent)
        : FrameInterfacePrivate (parent)
        , q_ptr(qobject_cast<PartitionFrame* >(parent))
        ,partition_model_(new PartitionModel(this))
        ,advanced_delegate_(new AdvancedPartitionDelegate(this))
        ,lvm_delegate_(new LvmPartitionDelegate(this))
        ,full_disk_delegate_(new FullDiskDelegate(this))
        ,simple_partition_delegate_(new SimplePartitionDelegate(this))
    {}

    ~PartitionFramePrivate() override;

     void initConnections();
     void initUI();

     void setupDiskEncrypt(bool flag);

     // Check current partition mode is simple mode or not.
     bool isSimplePartitionMode();
     bool isFullDiskPartitionMode();

     bool isRawDevice(const QList<Device::Ptr> list);

     void onButtonGroupToggled(QAbstractButton *button);
     void onNextButtonClicked();
     void onFullDiskCryptoButtonClicked(bool encrypto);

     // Write partitioning settings to conf file and emit manualPartDone() signal.
     void onManualPartDone(bool ok, const DeviceList& devices);

     // Notify delegate to do manual part.
     void onPrepareInstallFrameFinished();

     void showEditPartitionFrame(const Partition::Ptr partition);
     void showMainFrame();
     void showNewPartitionFrame(const Partition::Ptr partition);
     void showNewTableLoadingFrame();
     void showNewTableWarningFrame(const QString& device_path);
     void showPartitionNumberLimitationFrame();
     void showPartitionTableWarningFrame(const QString& device_path);
     void showSelectBootloaderFrame();
     void showEncryptFrame();
     void showDynamicDiskFrame();
     void showPrepareInstallFrame();

     bool isEncrypt();
     bool isEnSaveData();

     PartitionFrame* q_ptr=nullptr;

     AdvancedPartitionFrame* advanced_partition_frame_ = nullptr;
     LvmPartitionFrame* lvm_partition_frame_ = nullptr;
     EditPartitionFrame* edit_partition_frame_ = nullptr;
     EditPartitionFrame* edit_lvm_partition_frame_ = nullptr;
     FullDiskFrame* full_disk_partition_frame_ = nullptr;
     NewPartitionFrame* new_partition_frame_ = nullptr;
     NewPartitionFrame* new_lvm_partition_frame_ = nullptr;
     NewTableLoadingFrame* new_table_loading_frame_ = nullptr;
     NewTableWarningFrame* new_table_warning_frame_ = nullptr;
     PartitionLoadingFrame* partition_loading_frame_ = nullptr;
     PartitionNumberLimitationFrame* partition_number_limitation_frame_ = nullptr;
     PartitionTableWarningFrame* partition_table_warning_frame_ = nullptr;
     PrepareInstallFrame* prepare_install_frame_ = nullptr;
     SelectBootloaderFrame* select_bootloader_frame_ = nullptr;
     SimplePartitionFrame* simple_partition_frame_ = nullptr;
     Full_Disk_Encrypt_frame* full_disk_encrypt_frame_ = nullptr;
     DynamicDiskWarningFrame* dynamic_disk_warning_frame_ = nullptr;
     WarnningFrame*   save_data_pop_widget = nullptr;
     WarnningFrame*   swap_warnning_frame = nullptr;

     TitleLabel* title_label_ = nullptr;
     CommentLabel* comment_label_ = nullptr;
     QFrame* main_frame_ = nullptr;
     QStackedLayout* partition_stacked_layout_ = nullptr;
     QStackedLayout* main_layout_ = nullptr;
     DButtonBox* m_buttonGroup = nullptr;
     DButtonBoxButton* full_disk_frame_button_ = nullptr;
     DButtonBoxButton* simple_frame_button_ = nullptr;
     DButtonBoxButton* advanced_frame_button_ = nullptr;
     QWidget* m_buttonBoxWidget = nullptr;

     PartitionModel* partition_model_ = nullptr;
     AdvancedPartitionDelegate* advanced_delegate_ = nullptr;
     LvmPartitionDelegate* lvm_delegate_ = nullptr;
     FullDiskDelegate* full_disk_delegate_ = nullptr;
     SimplePartitionDelegate* simple_partition_delegate_ = nullptr;

     QHBoxLayout* next_layout = nullptr;
     bool m_isOnButtonBoxWidget = false;
};

PartitionFrame::PartitionFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new PartitionFramePrivate(this))
{
    setObjectName("partition_frame");

    m_private->initUI();
    m_private->initConnections();
    // 提前写入占位符，防止json中没有配置数据分区, 和手动分区屏蔽fstab写记录
    WriteSystemDataMountPoint("NO_DATA_MOUNT_POINT_PLACEHOLDER");
}

PartitionFrame::~PartitionFrame()
{

}

void PartitionFrame::init()
{
    // Read if_do_recovery in ini config and write DI_IF_DO_RECOVERY to conf for backup shell using.
    // Then if use checked in PrepareInstallFrame page, DI_IF_DO_RECOVERY will be rewrite.
    WriteIfDoRecovery(GetSettingsBool(kIfDoRecovery));
}

void PartitionFrame::finished()
{
}

bool PartitionFrame::shouldDisplay() const{
    return !GetSettingsBool(kSkipPartitionPage) && !GetSettingsBool("DI_LUPIN");
}

QString PartitionFrame::returnFrameName() const
{
    return ::QObject::tr("Create Partitions");
}

void PartitionFrame::autoPart() {
    if (GetSettingsBool("DI_LUPIN")) {
        emit finished();
        emit autoPartDone(true);
    }
    else if (m_private->isEncrypt()) {
        WriteFullDiskMode(true);
        m_private->partition_model_->autoPart();
    }
    else if (m_private->full_disk_delegate_->isLvm() && !GetSettingsBool(kPartitionDoAutoPart)) {
        m_private->setupDiskEncrypt(false);
        m_private->partition_model_->autoPart();
    }
    else if (m_private->isEnSaveData()) {
        m_private->setupDiskEncrypt(false);
        m_private->partition_model_->autoPart();
    }
    else {
        m_private->full_disk_delegate_->setAutoInstall(true);
        scanDevices();
    }
}

void PartitionFrame::scanDevices()  const{
    m_private->partition_model_->scanDevices();
}

void PartitionFrame::onAutoInstallPrepareFinished(bool finished)
{
    if (!finished) {
        qWarning() << Q_FUNC_INFO << "install failed!";
        return;
    }

    qInfo() << Q_FUNC_INFO << "set BootFlag: " << m_private->full_disk_delegate_->setBootFlag();

    OperationList list = m_private->full_disk_delegate_->operations();

    if (m_private->full_disk_delegate_->isLvm()) {
        m_private->setupDiskEncrypt(false);
        m_private->partition_model_->autoPart();
    } else if (GetSettingsBool(kPartitionDoAutoPartCrypt)) {
        m_private->setupDiskEncrypt(true);
        WriteFullDiskEncryptPassword(GetSettingsString(kDiskCryptPassword));
        m_private->partition_model_->autoPart();
    }
    else {
        m_private->partition_model_->manualPart(list);
    }
}

void PartitionFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
      if (m_private->partition_stacked_layout_->currentWidget() ==  m_private->lvm_partition_frame_) {
          m_private->title_label_->setText(::QObject::tr("Edit LVM Disk"));
          m_private->comment_label_->setText(
              ::QObject::tr("Make a backup of your important data and then continue"));
          m_private->nextButton->setText(::QObject::tr("Ready to Install"));
      } else {
          m_private->title_label_->setText(::QObject::tr("Create Partitions"));
          m_private->comment_label_->setText(
              ::QObject::tr("Make sure you have backed up important data, then select the disk to install"));
          m_private->nextButton->setText(::QObject::tr("Next"));
      }

    m_private->simple_frame_button_->setText(::QObject::tr("Simple"));
    m_private->advanced_frame_button_->setText(::QObject::tr("Advanced"));
    m_private->full_disk_frame_button_->setText(::QObject::tr("Full Disk"));
  } else {
      FrameInterface::changeEvent(event);
  }
}

void PartitionFrame::showEvent(QShowEvent *event)
{
    setFocus();
    return FrameInterface::showEvent(event);
}

bool PartitionFrame::focusSwitch()
{
    if (m_current_focus_widget == nullptr) {
        this->setCurentFocus(m_private->nextButton);
    } else if (m_private->lvm_partition_frame_->isVisible()) {
        //当LVM分区编辑界面显示时，限制键盘事件在LVM分区界面里处理
        if (m_private->lvm_partition_frame_->isLastButtonHasFocus()) {
            this->setCurentFocus(m_private->nextButton);
        } else if (m_private->nextButton == m_current_focus_widget) {
            this->setCurentFocus(m_private->lvm_partition_frame_);
            m_private->lvm_partition_frame_->focusSwitch();
        } else if (m_private->lvm_partition_frame_->focusSwitch()) {
            m_private->lvm_partition_frame_->setLastButtonFocus();
        }
    } else {
        if (m_private->nextButton == m_current_focus_widget) {
            if (m_private->prepare_install_frame_->isVisible()) {
                m_private->prepare_install_frame_->focusSwitch();
            } else {
                this->setCurentFocus(m_private->m_buttonBoxWidget);
                m_private->m_isOnButtonBoxWidget = true;
            }
        } else if (m_private->m_buttonBoxWidget == m_current_focus_widget) {
            //焦点落在分区模式切换控件上，被选中的按钮确定当前属于哪种分区模式
            if (m_private->advanced_frame_button_->isChecked()) {
                if (m_private->edit_partition_frame_->isVisible()) {
                    m_private->edit_partition_frame_->focusSwitch();
                } else if (m_private->edit_lvm_partition_frame_->isVisible()) {
                    m_private->edit_partition_frame_->focusSwitch();
                } else if(m_private->new_partition_frame_->isVisible()) {
                    m_private->new_partition_frame_->focusSwitch();
                } else if(m_private->new_lvm_partition_frame_->isVisible()) {
                    m_private->new_lvm_partition_frame_->focusSwitch();
                } else if (m_private->select_bootloader_frame_->isVisible()) {
                    m_private->select_bootloader_frame_->focusSwitch();
                } else {
                    if (m_private->advanced_partition_frame_->focusSwitch()) {
                        this->setCurentFocus(m_private->nextButton);
                    } else {
                        m_private->m_isOnButtonBoxWidget = false;
                    }
                }
            } else if (m_private->full_disk_frame_button_->isChecked()) {
                //this->setCurentFocus(m_private->full_disk_partition_frame_);
                //当前处于全盘分区界面
                if (m_private->full_disk_partition_frame_->focusSwitch()) {
                    if (m_private->full_disk_encrypt_frame_->isVisible()) {
                        this->setCurentFocus(m_private->full_disk_encrypt_frame_);
                    } else {
                        this->setCurentFocus(m_private->nextButton);
                    }
                } else {
                    m_private->m_isOnButtonBoxWidget = false;
                }
            }
        } else if (m_private->full_disk_encrypt_frame_ == m_current_focus_widget) {
            if (m_private->full_disk_encrypt_frame_->isVisible()) {
                m_private->full_disk_encrypt_frame_->focusSwitch();
            } else {
                this->setCurentFocus(m_private->nextButton);
            }
        }
    }

    return true;
}

bool PartitionFrame::doSpace()
{
    if (m_private->lvm_partition_frame_->isVisible()) {
        //相应LVM分区界面的空格事件
        m_private->lvm_partition_frame_->doSpace();
    } else if (m_private->prepare_install_frame_->isVisible()) {
        // 处理准备安装界面的空格事件处理
        m_private->prepare_install_frame_->doSpace();
    } else {
        if (m_private->advanced_frame_button_->isChecked()) {
            //当前处于手动分区界面
            if (m_private->edit_partition_frame_->isVisible()) {
                m_private->edit_partition_frame_->doSpace();
            } else if (m_private->edit_lvm_partition_frame_->isVisible()) {
                m_private->edit_partition_frame_->doSpace();
            } else if (m_private->new_partition_frame_->isVisible()) {
                m_private->new_partition_frame_->doSpace();
            } else if (m_private->new_lvm_partition_frame_->isVisible()) {
                m_private->new_lvm_partition_frame_->doSpace();
            } else if (m_private->select_bootloader_frame_->isVisible()) {
                m_private->select_bootloader_frame_->doSpace();
            } else {
                m_private->advanced_partition_frame_->doSpace();
            }
        } else if (m_private->full_disk_frame_button_->isChecked()) {
            m_private->full_disk_partition_frame_->doSpace();
        }
    }
    return true;
}

bool PartitionFrame::doSelect()
{
    if (m_private->nextButton == m_current_focus_widget) {
        if (m_private->prepare_install_frame_->isVisible()) {
            m_private->prepare_install_frame_->doSelect();
        } else {
            emit m_private->nextButton->clicked();
        }
    } else {
        if (m_private->lvm_partition_frame_->isVisible()) {
            if (m_private->lvm_partition_frame_->isLastButtonHasFocus()) {
                m_private->lvm_partition_frame_->clickLastButton();
            } else {
                m_private->lvm_partition_frame_->doSelect();
            }
        } else {
            if (m_private->advanced_frame_button_->isChecked()) {
                if (m_private->edit_partition_frame_->isVisible()) {
                    m_private->edit_partition_frame_->doSelect();
                } else if (m_private->edit_lvm_partition_frame_->isVisible()) {
                    m_private->edit_partition_frame_->doSelect();
                } else if(m_private->new_partition_frame_->isVisible()) {
                    m_private->new_partition_frame_->doSelect();
                } else if(m_private->new_lvm_partition_frame_->isVisible()) {
                    m_private->new_lvm_partition_frame_->doSelect();
                } else if (m_private->select_bootloader_frame_->isVisible()) {
                    m_private->select_bootloader_frame_->doSelect();
                } else {
                    m_private->advanced_partition_frame_->doSelect();
                }
            } else if (m_private->full_disk_frame_button_->isChecked()) {
                m_private->full_disk_partition_frame_->doSelect();
            }
        }
    }
    return true;
}

bool PartitionFrame::directionKey(int keyvalue)
{
    switch (keyvalue) {
    case Qt::Key_Up: {

        }
        break;
    case Qt::Key_Down: {

        }
        break;
    case Qt::Key_Left: {
            if (!m_private->lvm_partition_frame_->isVisible()) {
                //这里处理当LVM界面显示的时候，按下键盘左右键，界面会跳转到其他界面的bug
                if ( m_private->m_isOnButtonBoxWidget ) {
                    m_private->advanced_frame_button_->setChecked(true);
                    m_private->full_disk_frame_button_->setChecked(false);
                    m_private->partition_stacked_layout_->setCurrentWidget(m_private->advanced_partition_frame_);
                }
            }
        }
        break;
    case Qt::Key_Right: {
            if (!m_private->lvm_partition_frame_->isVisible()) {
                if ( m_private->m_isOnButtonBoxWidget ) {
                    m_private->advanced_frame_button_->setChecked(false);
                    m_private->full_disk_frame_button_->setChecked(true);
                    m_private->partition_stacked_layout_->setCurrentWidget(m_private->full_disk_partition_frame_);
                }
            }
        }
        break;
    }

    if (m_private->lvm_partition_frame_->isVisible()) {
        m_private->lvm_partition_frame_->directionKey(keyvalue);
    } else {
        if (m_private->advanced_frame_button_->isChecked() && (!m_private->m_isOnButtonBoxWidget)) {
            if (m_private->edit_partition_frame_->isVisible()) {
                m_private->edit_partition_frame_->directionKey(keyvalue);
            } else if (m_private->edit_lvm_partition_frame_->isVisible()) {
                m_private->edit_partition_frame_->directionKey(keyvalue);
            } else if(m_private->new_partition_frame_->isVisible()) {
                m_private->new_partition_frame_->directionKey(keyvalue);
            } else if(m_private->new_lvm_partition_frame_->isVisible()) {
                m_private->new_lvm_partition_frame_->directionKey(keyvalue);
            } else if (m_private->select_bootloader_frame_->isVisible()) {
                m_private->select_bootloader_frame_->directionKey(keyvalue);
            } else {
                m_private->advanced_partition_frame_->directionKey(keyvalue);
            }
        } else if (m_private->full_disk_frame_button_->isChecked() && (!m_private->m_isOnButtonBoxWidget)) {
            m_private->full_disk_partition_frame_->directionKey(keyvalue);
        } else if (m_private->nextButton == m_current_focus_widget) {
            if (m_private->prepare_install_frame_->isVisible()) {
                m_private->prepare_install_frame_->directionKey(keyvalue);
            }
        }
    }

    return true;
}

PartitionFramePrivate::~PartitionFramePrivate()
{

}

void PartitionFramePrivate::initConnections() {
  disconnect(nextButton, nullptr, nullptr, nullptr);
  connect(nextButton, &QPushButton::clicked, this, [=] {
       if (partition_stacked_layout_->currentWidget() == full_disk_partition_frame_ && this->isEncrypt()) {
           showEncryptFrame();
       }
       else {
           onNextButtonClicked();
       }
  });

  connect(m_buttonGroup, &DButtonBox::buttonClicked, this, &PartitionFramePrivate::onButtonGroupToggled);

  // Show main frame when device is refreshed.
  connect(partition_model_, &PartitionModel::deviceRefreshed,
          this, &PartitionFramePrivate::showMainFrame);
  connect(partition_model_, &PartitionModel::autoPartDone,
          q_ptr, &PartitionFrame::autoPartDone);
  connect(partition_model_, &PartitionModel::manualPartDone,
          this, &PartitionFramePrivate::onManualPartDone);

  connect(advanced_partition_frame_,
          &AdvancedPartitionFrame::requestEditPartitionFrame,
          this, &PartitionFramePrivate::showEditPartitionFrame);
  connect(advanced_partition_frame_,
          &AdvancedPartitionFrame::requestNewPartitionFrame,
          this, &PartitionFramePrivate::showNewPartitionFrame);
  connect(advanced_partition_frame_, &AdvancedPartitionFrame::requestNewTable,
          this, &PartitionFramePrivate::showPartitionTableWarningFrame);
  connect(advanced_partition_frame_,
          &AdvancedPartitionFrame::requestPartitionNumberLimitationFrame,
          this, &PartitionFramePrivate::showPartitionNumberLimitationFrame);
  connect(advanced_partition_frame_,
          &AdvancedPartitionFrame::requestSelectBootloaderFrame,
          this, &PartitionFramePrivate::showSelectBootloaderFrame);

  connect(lvm_partition_frame_,
          &AdvancedPartitionFrame::requestEditPartitionFrame,
          this, &PartitionFramePrivate::showEditPartitionFrame);
  connect(lvm_partition_frame_,
          &AdvancedPartitionFrame::requestNewPartitionFrame,
          this, &PartitionFramePrivate::showNewPartitionFrame);
  connect(lvm_partition_frame_, &AdvancedPartitionFrame::requestNewTable,
          this, &PartitionFramePrivate::showPartitionTableWarningFrame);
  connect(lvm_partition_frame_,
          &AdvancedPartitionFrame::requestPartitionNumberLimitationFrame,
          this, &PartitionFramePrivate::showPartitionNumberLimitationFrame);
  connect(lvm_partition_frame_,
          &AdvancedPartitionFrame::requestSelectBootloaderFrame,
          this, &PartitionFramePrivate::showSelectBootloaderFrame);
  connect(lvm_partition_frame_, &LvmPartitionFrame::aborted,
          this, &PartitionFramePrivate::showMainFrame);

  connect(edit_partition_frame_, &EditPartitionFrame::finished, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->repaint();
  });
  connect(edit_lvm_partition_frame_, &EditPartitionFrame::finished, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->repaint();
  });
  connect(new_partition_frame_, &NewPartitionFrame::finished, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->repaint();
  });
  connect(new_lvm_partition_frame_, &NewPartitionFrame::finished, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->repaint();
  });

  connect(new_table_warning_frame_, &NewTableWarningFrame::canceled,
          this, &PartitionFramePrivate::showMainFrame);
  connect(new_table_warning_frame_, &NewTableWarningFrame::confirmed,
          this, &PartitionFramePrivate::showNewTableLoadingFrame);
  connect(new_table_warning_frame_, &NewTableWarningFrame::confirmed,
          partition_model_, &PartitionModel::createPartitionTable);

  connect(partition_number_limitation_frame_,
          &PartitionNumberLimitationFrame::finished,
          this, &PartitionFramePrivate::showMainFrame);

  connect(partition_table_warning_frame_, &PartitionTableWarningFrame::canceled,
          this, &PartitionFramePrivate::showMainFrame);
  connect(partition_table_warning_frame_,
          &PartitionTableWarningFrame::confirmed,
          this, &PartitionFramePrivate::showNewTableWarningFrame);
  connect(partition_table_warning_frame_, &PartitionTableWarningFrame::reboot,
          q_ptr, &PartitionFrame::reboot);

  connect(prepare_install_frame_, &PrepareInstallFrame::aborted,
          this, &PartitionFramePrivate::showMainFrame);
  connect(prepare_install_frame_, &PrepareInstallFrame::finished, this, [=] {
      if ((!GetSettingsBool(KPartitionSkipFullCryptPage) && this->isEncrypt()) || this->isEnSaveData() || full_disk_delegate_->isLvm()) {
        q_ptr->autoPart();
        q_ptr->m_proxy->nextFrame();
      }
      else {
        onPrepareInstallFrameFinished();
      }
  });

  connect(select_bootloader_frame_, &SelectBootloaderFrame::bootloaderUpdated,
          advanced_partition_frame_,
          &AdvancedPartitionFrame::setBootloaderPath);
  connect(select_bootloader_frame_, &SelectBootloaderFrame::bootloaderUpdated,
          advanced_delegate_, &AdvancedPartitionDelegate::setBootloaderPath);
  connect(select_bootloader_frame_, &SelectBootloaderFrame::finished, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->repaint();
  });
  connect(advanced_delegate_, &AdvancedPartitionDelegate::deviceRefreshed,
          select_bootloader_frame_, &SelectBootloaderFrame::deviceRefreshed);

  connect(partition_model_, &PartitionModel::deviceRefreshed,
          advanced_delegate_, &AdvancedPartitionDelegate::onDeviceRefreshed);

  connect(select_bootloader_frame_, &SelectBootloaderFrame::bootloaderUpdated,
          lvm_partition_frame_,
          &AdvancedPartitionFrame::setBootloaderPath);
  connect(select_bootloader_frame_, &SelectBootloaderFrame::finished,
          this, &PartitionFramePrivate::showMainFrame);
  connect(lvm_delegate_, &AdvancedPartitionDelegate::deviceRefreshed,
          select_bootloader_frame_, &SelectBootloaderFrame::deviceRefreshed);

  connect(partition_model_, &PartitionModel::deviceRefreshed,
          lvm_delegate_, &AdvancedPartitionDelegate::onDeviceRefreshed);

  if (!GetSettingsBool(kPartitionSkipSimplePartitionPage)) {
    connect(partition_model_, &PartitionModel::deviceRefreshed,
            simple_partition_delegate_,
            &SimplePartitionDelegate::onDeviceRefreshed);

  }
  if (!GetSettingsBool(kPartitionSkipFullDiskPartitionPage)) {
    connect(partition_model_, &PartitionModel::deviceRefreshed,
            full_disk_delegate_, &FullDiskDelegate::onDeviceRefreshed);
  }

  // TODO(Shaohua): Show warning page both in full-disk frame and
  // simple-partition frame.
  connect(simple_partition_frame_, &SimplePartitionFrame::requestNewTable,
          this, &PartitionFramePrivate::showPartitionTableWarningFrame);

  connect(full_disk_encrypt_frame_, &Full_Disk_Encrypt_frame::cancel, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->repaint();
  });

  connect(full_disk_encrypt_frame_, &Full_Disk_Encrypt_frame::encryptFinished, q_ptr, [=] {
      //      q_ptr->autoPart();
      //      q_ptr->m_proxy->nextFrame();
      setupDiskEncrypt(this->isEncrypt()); // 设置全盘分区场景下的全盘加密

      q_ptr->m_proxy->hideChildFrame();
      q_ptr->repaint();
      showPrepareInstallFrame();
  });

  connect(dynamic_disk_warning_frame_, &DynamicDiskWarningFrame::requestCancel, this,
          &PartitionFramePrivate::showMainFrame);

  connect(dynamic_disk_warning_frame_, &DynamicDiskWarningFrame::requestNext, this,
          &PartitionFramePrivate::showPrepareInstallFrame);

  connect(full_disk_partition_frame_, &FullDiskFrame::showDeviceInfomation,
          full_disk_encrypt_frame_, &Full_Disk_Encrypt_frame::onShowDeviceInfomation);

  connect(full_disk_partition_frame_, &FullDiskFrame::cryptoStateChanged,
          this, &PartitionFramePrivate::onFullDiskCryptoButtonClicked);

  connect(full_disk_partition_frame_, &FullDiskFrame::enableNextButton, [=](const bool& enable) {
      nextButton->setEnabled(enable);
  });

  connect(full_disk_partition_frame_, &FullDiskFrame::showSaveDataPopWidget, this, [=] {
     q_ptr->m_proxy->showChildFrame(save_data_pop_widget);
  });

  connect(full_disk_delegate_, &FullDiskDelegate::requestAutoInstallFinished, q_ptr, &PartitionFrame::onAutoInstallPrepareFinished);

  connect(swap_warnning_frame, &WarnningFrame::quitEntered, this, [=] {
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->repaint();
  });

  connect(save_data_pop_widget, &WarnningFrame::quitEntered, this, [=] {
      full_disk_partition_frame_->saveDataStateChanged(true);
      full_disk_partition_frame_->showSaveDataCheck(true);
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->repaint();
  });
  connect(save_data_pop_widget, &WarnningFrame::quitCanceled,this, [=] {
      full_disk_partition_frame_->saveDataStateChanged(false);
      full_disk_partition_frame_->showSaveDataCheck(false);
      q_ptr->m_proxy->hideChildFrame();
      q_ptr->repaint();
  });

    Q_EMIT full_disk_frame_button_->click();
}

void PartitionFramePrivate::initUI() {
  advanced_partition_frame_ =
      new AdvancedPartitionFrame(advanced_delegate_, q_ptr);
  lvm_partition_frame_  = new LvmPartitionFrame(lvm_delegate_, q_ptr);
  edit_partition_frame_ = new EditPartitionFrame(q_ptr->m_proxy, advanced_delegate_);
  edit_lvm_partition_frame_ = new EditPartitionFrame(q_ptr->m_proxy, lvm_delegate_);
  full_disk_partition_frame_ = new FullDiskFrame(full_disk_delegate_, q_ptr);
  new_partition_frame_ = new NewPartitionFrame(q_ptr->m_proxy, advanced_delegate_);
  new_lvm_partition_frame_ = new NewPartitionFrame(q_ptr->m_proxy, lvm_delegate_);
  new_table_loading_frame_ = new NewTableLoadingFrame(q_ptr);
  new_table_warning_frame_ = new NewTableWarningFrame(q_ptr);
  partition_loading_frame_ = new PartitionLoadingFrame(q_ptr);
  partition_number_limitation_frame_ = new PartitionNumberLimitationFrame(q_ptr);
  partition_table_warning_frame_ = new PartitionTableWarningFrame(q_ptr);
  prepare_install_frame_ = new PrepareInstallFrame(q_ptr);
  select_bootloader_frame_ = new SelectBootloaderFrame(q_ptr->m_proxy);
  simple_partition_frame_ =
      new SimplePartitionFrame(simple_partition_delegate_, q_ptr);

  full_disk_encrypt_frame_ = new Full_Disk_Encrypt_frame(q_ptr->m_proxy, full_disk_delegate_);

  dynamic_disk_warning_frame_ = new DynamicDiskWarningFrame(q_ptr);

  save_data_pop_widget = new WarnningFrame(nullptr);
  save_data_pop_widget->useTitle(false);
  save_data_pop_widget->setComment("/data/home is detected. If data is not saved, the directory will be formatted. Please confirm whether to format or retain the directory.");
  save_data_pop_widget->setEnterButtonText("Save");
  save_data_pop_widget->setCancelButtonText("Format");
  save_data_pop_widget->hide();

  swap_warnning_frame = new WarnningFrame(q_ptr->m_proxy);
  swap_warnning_frame->useCancelButton(false);
  swap_warnning_frame->setTitle("Friendly Note");
  swap_warnning_frame->setComment("No swap partition created, which may affect system performance");
  swap_warnning_frame->hide();

  title_label_ = new TitleLabel(::QObject::tr("Create Partitions"));
  title_label_->setObjectName("title_label_");
  comment_label_ = new CommentLabel(
      ::QObject::tr("Make sure you have backed up important data, then select the disk to install"));
  QHBoxLayout* comment_layout = new QHBoxLayout();
  comment_layout->setContentsMargins(0, 0, 0, 0);
  comment_layout->setSpacing(0);
  comment_layout->addWidget(comment_label_);

  m_buttonGroup = new DButtonBox(q_ptr);
  simple_frame_button_ = new DButtonBoxButton(::QObject::tr("Simple"), q_ptr);
  simple_frame_button_->setMinimumWidth(86);
  //simple_frame_button_->setFocusPolicy(Qt::NoFocus);
  advanced_frame_button_ = new DButtonBoxButton(::QObject::tr("Advanced"), q_ptr);
  advanced_frame_button_->setMinimumWidth(86);
  advanced_frame_button_->setStyleSheet("QWidget::checked{ color:rgb(255,255,255); }");
  //advanced_frame_button_->setFocusPolicy(Qt::NoFocus);
  full_disk_frame_button_ = new DButtonBoxButton(::QObject::tr("Full Disk"), q_ptr);
  full_disk_frame_button_->setMinimumWidth(86);
  full_disk_frame_button_->setStyleSheet("QWidget::checked{ color:rgb(255,255,255); }");
  //full_disk_frame_button_->setFocusPolicy(Qt::NoFocus);

  if (GetSettingsBool(kPartitionSkipFullDiskPartitionPage)) {
      m_buttonGroup->setButtonList({advanced_frame_button_}, true);
  } else {
      m_buttonGroup->setButtonList({advanced_frame_button_, full_disk_frame_button_}, true);
  }

  m_buttonGroup->setVisible(true);

  QHBoxLayout* button_layout = new QHBoxLayout();
  button_layout->setContentsMargins(0, 0, 0, 0);
  button_layout->setSpacing(0);
  button_layout->addStretch();
  button_layout->addWidget(m_buttonGroup, 0, Qt::AlignCenter);
  button_layout->addStretch();

  m_buttonBoxWidget = new QWidget;
  m_buttonBoxWidget->setObjectName("buttonBoxWidget");
  m_buttonBoxWidget->setStyleSheet("QWidget#buttonBoxWidget::focus{border:1px solid; border-color:rgb(1, 128, 255); border-radius:5px; padding:2px 4px;}");
  m_buttonBoxWidget->setLayout(button_layout);

  partition_stacked_layout_ = new QStackedLayout();
  partition_stacked_layout_->setContentsMargins(0, 0, 0, 0);
  partition_stacked_layout_->setSpacing(0);

  partition_stacked_layout_->addWidget(simple_partition_frame_);
  partition_stacked_layout_->addWidget(advanced_partition_frame_);
  partition_stacked_layout_->addWidget(full_disk_partition_frame_);
  partition_stacked_layout_->addWidget(lvm_partition_frame_);

  if (GetSettingsBool(kPartitionSkipSimplePartitionPage)) {
      simple_frame_button_->hide();
      simple_frame_button_->setChecked(false);
      simple_partition_frame_->hide();
      full_disk_frame_button_->setChecked(true);
      partition_stacked_layout_->setCurrentWidget(full_disk_partition_frame_);
  }
  else {
      simple_frame_button_->setChecked(true);
  }

  if (GetSettingsBool(kPartitionSkipFullDiskPartitionPage)) {
      full_disk_frame_button_->hide();
      full_disk_frame_button_->setChecked(false);
      full_disk_partition_frame_->hide();
  }

  if (GetSettingsBool(kPartitionSkipFullDiskPartitionPage) &&
          GetSettingsBool(kPartitionSkipSimplePartitionPage)) {
      advanced_frame_button_->setChecked(true);
      partition_stacked_layout_->setCurrentWidget(advanced_partition_frame_);
  }

  QHBoxLayout* partition_stacked_wrapper_layout = new QHBoxLayout();
  partition_stacked_wrapper_layout->setContentsMargins(0, 0, 0, 0);
  partition_stacked_wrapper_layout->setSpacing(0);
  partition_stacked_wrapper_layout->addLayout(partition_stacked_layout_);

  // and advanced partition page.
  nextButton->setText(::QObject::tr("Start installation"));
  next_layout = new QHBoxLayout();
  next_layout->setContentsMargins(0, 0, 0, 0);
  next_layout->addWidget(nextButton);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(title_label_, 0, Qt::AlignHCenter);
  layout->addSpacing(kMainLayoutSpacing);
  layout->addLayout(comment_layout);
  layout->addSpacing(kMainLayoutSpacing);
  //layout->addLayout(button_layout);
  layout->addWidget(m_buttonBoxWidget, 0, Qt::AlignCenter);
  layout->addSpacing(20 + kMainLayoutSpacing);
  layout->addLayout(partition_stacked_wrapper_layout);
  layout->addLayout(next_layout);

  main_frame_ = new QFrame();
  main_frame_->setContentsMargins(0, 0, 0, 0);
  main_frame_->setLayout(layout);

  main_layout_ = new QStackedLayout();
  main_layout_->setContentsMargins(0, 0, 0, 0);
  // Keep PartitionLoadingFrame is the first display page.
  main_layout_->addWidget(partition_loading_frame_);
  main_layout_->addWidget(main_frame_);
  main_layout_->addWidget(new_table_loading_frame_);
  main_layout_->addWidget(new_table_warning_frame_);
  main_layout_->addWidget(partition_number_limitation_frame_);
  main_layout_->addWidget(partition_table_warning_frame_);
  main_layout_->addWidget(prepare_install_frame_);
  main_layout_->addWidget(dynamic_disk_warning_frame_);

  centerLayout->addLayout(main_layout_);
  q_ptr->setContentsMargins(0, 0, 0, 0);
  //q_ptr->setFocusPolicy(Qt::TabFocus);
  q_ptr->setLayout(centerLayout);
}

// 设置是否启用全盘加密
void PartitionFramePrivate::setupDiskEncrypt(bool flag)
{
    if (flag) {
        WriteFullDiskEncryptPassword(full_disk_encrypt_frame_->passwd());
    } else {
        WriteFullDiskEncryptPassword("");
    }

    FinalFullDiskResolution resolution;
    full_disk_encrypt_frame_->getFinalDiskResolution(resolution);
    FinalFullDiskOptionList& option_list = resolution.option_list;
    for (FinalFullDiskOption& option : option_list) {
        option.encrypt = flag;
    }
    WriteFullDiskResolution(resolution);
}

bool PartitionFramePrivate::isFullDiskPartitionMode() {
  return full_disk_frame_button_->isChecked();
}

bool PartitionFramePrivate::isSimplePartitionMode() {
  return simple_frame_button_->isChecked();
}

bool PartitionFramePrivate::isRawDevice(const QList<Device::Ptr> list) {
    for (const Device::Ptr device : list) {
        QProcess process;
        process.start("fdisk", QStringList() << "-l" << device->path);
        process.waitForFinished();
        const QString& result = process.readAllStandardOutput();

        if (result.isEmpty()) {
            continue;
        }

        QTextStream stream(result.toUtf8());
        QString     line;
        while (stream.readLineInto(&line)) {
            const QStringList& args = line.simplified().split(" ");

            if (args.length() < 2) {
              continue;
            }

            // SFS(42h): https://en.wikipedia.org/wiki/Partition_type
            if (args.last().contains("SFS") || args[args.length() - 2] == "42") {
                return true;
            }
        }
    }

    return false;
}

void PartitionFramePrivate::onButtonGroupToggled(QAbstractButton *button)
{
    // Whenever the installation mode is switched, the next button is set to enable.
    // Further space capacity judgment is left to the specific installation method.
    nextButton->setEnabled(true);

#ifdef QT_DEBUG_test
  showPartitionNumberLimitationFrame();
#else
    bool isDiskEncrypt = false;
    if (button == full_disk_frame_button_){
        qDebug() << "on fulldisk button toggled";
        isDiskEncrypt = this->isEncrypt();
        if (!GetSettingsBool(kPartitionSkipFullDiskPartitionPage)) {
            partition_stacked_layout_->setCurrentWidget(full_disk_partition_frame_);
        }
    }
    else if (button == simple_frame_button_){
        qDebug() << "on simple button toggled";
        partition_stacked_layout_->setCurrentWidget(simple_partition_frame_);
    }
    else {
        qDebug() << "on advanced button toggled";
        // Refresh device list before showing advanced partition frame.
        // Because mount-point of partitions might have be updated.
        advanced_delegate_->refreshVisual();
        partition_stacked_layout_->setCurrentWidget(advanced_partition_frame_);
    }

    setupDiskEncrypt(isDiskEncrypt);  // 设置手动分区和全盘分区切换时清理全盘加密资源
#endif // QT_DEBUG
}

void PartitionFramePrivate::onNextButtonClicked() {
  WriteFullDiskMode(isFullDiskPartitionMode());
  prepare_install_frame_->setCreateRecovery(isFullDiskPartitionMode());

  if (isSimplePartitionMode()) {
    // Validate simple partition frame.
    if (!simple_partition_frame_->validate()) {
      return;
    }
  } else if (isFullDiskPartitionMode()) {
    if (!full_disk_partition_frame_->validate()) {
      return;
    }
  } else if (AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
    // Validate advanced partition frame.
    if (!advanced_partition_frame_->validate()) {
      return;
    }
  } else if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Install) {
    // Validate advanced partition frame.
    if (!lvm_partition_frame_->validate()) {
      return;
    }
  }

  // check disk is raw
  QList<Device::Ptr> device;
  if (isSimplePartitionMode()) {
     device << simple_partition_frame_->selectedDevice();
     dynamic_disk_warning_frame_->setWarningTip(::QObject::tr("The target disk is dynamic, and your data may be lost if proceeding. Please make a backup of your important files first."));
  }
  else if (!isFullDiskPartitionMode()) {
    device = advanced_partition_frame_->getAllUsedDevice();
    dynamic_disk_warning_frame_->setWarningTip(::QObject::tr("The target disk is dynamic which will be formatted if proceeding. Please make a backup of your important files first."));
    static bool isFirstWarning = true;
    if (isFirstWarning && !AdvancedPartitionDelegate::swapOk && AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Format_Pv) {
        q_ptr->m_proxy->showChildFrame(swap_warnning_frame);
        isFirstWarning = false;
        qApp->setActiveWindow(q_ptr);
        return;
    } else if (isFirstWarning && AdvancedPartitionDelegate::swapOk && AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Format_Pv) {
        isFirstWarning = false;
    }
  }

  if (!device.isEmpty() && isRawDevice(device)) {
    dynamic_disk_warning_frame_->setDevice(device);
    showDynamicDiskFrame();
    return;
  }

    showPrepareInstallFrame();

    if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Format_Pv) {
       emit prepare_install_frame_->finished();
       lvm_partition_frame_->updateLayout(next_layout, ::QObject::tr("Back", "button"));
       nextButton->setText(::QObject::tr("Ready to Install"));
       emit q_ptr->coverMainWindowFrameLabelsView(false);
    }
}

void PartitionFramePrivate::onFullDiskCryptoButtonClicked(bool encrypto)
{
//    nextButton->setText(encrypto ? ::QObject::tr("Next") :
//                                   ::QObject::tr("Start installation"));
}

void PartitionFramePrivate::onManualPartDone(bool ok, const DeviceList& devices) {
  if (ok) {

    // Write settings to file.
    if (isSimplePartitionMode()) {
      simple_partition_delegate_->onManualPartDone(devices);
    } else if (isFullDiskPartitionMode()) {
      full_disk_delegate_->onManualPartDone(devices);
    } else if (Install_Lvm_Status::Lvm_Format_Pv == AdvancedPartitionDelegate::install_Lvm_Status) {
        lvm_delegate_->onLvmPartDone(ok,devices);
        m_buttonGroup->setVisible(false);
        partition_stacked_layout_->setCurrentWidget(lvm_partition_frame_);
        main_layout_->setCurrentWidget(main_frame_);
        title_label_->setText(::QObject::tr("Edit LVM Disk"));
        comment_label_->setText(
            ::QObject::tr("Make a backup of your important data and then continue"));
        nextButton->setText(::QObject::tr("Ready to Install"));
        return ;
    } else if (Install_Lvm_Status::Lvm_Install == AdvancedPartitionDelegate::install_Lvm_Status) {
        lvm_delegate_->onManualPartDone(devices);
    } else {
      advanced_delegate_->onManualPartDone(devices);
    }
  }

  emit q_ptr->manualPartDone(ok);
}

void PartitionFramePrivate::onPrepareInstallFrameFinished() {
    // First, update boot flag.
    bool found_boot = false;
    if (isSimplePartitionMode()) {
        found_boot = simple_partition_delegate_->setBootFlag();
    } else if (isFullDiskPartitionMode() && !this->isEncrypt()){
        found_boot = full_disk_delegate_->setBootFlag();
    } else if (AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
        found_boot = advanced_delegate_->setBootFlag();
    } else if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Install) {
        found_boot = lvm_delegate_->setBootFlag();
    }

    if (!found_boot && !isFullDiskPartitionMode() &&AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
        qCritical() << "No boot partition found, we shall never reach here!";
        return;
    }

    // Get operation list.
    OperationList operations;
    if (isSimplePartitionMode()) {
        operations = simple_partition_delegate_->operations();
    } else if (isFullDiskPartitionMode() && !this->isEncrypt()) {
        operations = full_disk_delegate_->operations();
    } else if (AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install){
        operations = advanced_delegate_->operations();
    } else if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Install){
        //注意一定要先添加老分区操作，否则会产生不可预料的后果
        operations = lvm_delegate_->m_oldOperationList;
        operations.append(lvm_delegate_->operations());
    }

    // full disk encrypt operations is empty.
    if (isFullDiskPartitionMode() && this->isEncrypt()) {
        q_ptr->m_proxy->nextFrame();
    }
    else if (operations.isEmpty()) {
        qCritical() << "Operation list is empty";
        return;
    }
    else {
        partition_model_->manualPart(operations);
        if (AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Format_Pv) {
            q_ptr->m_proxy->nextFrame();
        } else {
            lvm_delegate_->m_oldOperationList = operations;
            advanced_partition_frame_->hide();
        }
    }
}

void PartitionFramePrivate::showEditPartitionFrame(const Partition::Ptr partition) {
    if (Install_Lvm_Status::Lvm_Install == AdvancedPartitionDelegate::install_Lvm_Status) {
        edit_lvm_partition_frame_->setPartition(partition);
        q_ptr->m_proxy->showChildFrame(edit_lvm_partition_frame_);
    } else {
        edit_partition_frame_->setPartition(partition);
        q_ptr->m_proxy->showChildFrame(edit_partition_frame_);
    }
}

void PartitionFramePrivate::showMainFrame() {
  if (main_layout_->currentWidget() == main_frame_
          && partition_stacked_layout_->currentWidget() == lvm_partition_frame_) {
      m_buttonGroup->setVisible(true);
      advanced_delegate_->onDeviceRefreshed(advanced_delegate_->realDevices());
      partition_stacked_layout_->setCurrentWidget(advanced_partition_frame_);
      main_layout_->setCurrentWidget(main_frame_);
      title_label_->setText(::QObject::tr("Create Partitions"));
      nextButton->setText(::QObject::tr("Next"));
      comment_label_->setText(
          ::QObject::tr("Make sure you have backed up important data, then select the disk to install"));
  }

  main_layout_->setCurrentWidget(main_frame_);

  q_ptr->setCurentFocus(m_buttonBoxWidget);
  emit q_ptr->coverMainWindowFrameLabelsView(false);
}

void PartitionFramePrivate::showNewPartitionFrame(
    const Partition::Ptr partition) {
    if (Install_Lvm_Status::Lvm_Install == AdvancedPartitionDelegate::install_Lvm_Status) {
        new_lvm_partition_frame_->setPartition(partition);
        q_ptr->m_proxy->showChildFrame(new_lvm_partition_frame_);
    } else {
        new_partition_frame_->setPartition(partition);
        q_ptr->m_proxy->showChildFrame(new_partition_frame_);
    }
}

void PartitionFramePrivate::showNewTableLoadingFrame() {
  main_layout_->setCurrentWidget(new_table_loading_frame_);

  emit q_ptr->coverMainWindowFrameLabelsView(true);
}

void PartitionFramePrivate::showNewTableWarningFrame(const QString& device_path) {
  DeviceList devices;
  if (isSimplePartitionMode()) {
    devices = simple_partition_delegate_->realDevices();
  } else if (isFullDiskPartitionMode()) {
    qCritical() << "Never show new table warning frame for simple disk frame";
    return;
  } else if (AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
    devices = advanced_delegate_->realDevices();
  } else if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Install) {
    devices = lvm_delegate_->realDevices();
  }

  const int device_index = DeviceIndex(devices, device_path);
  Q_ASSERT(device_index > -1);
  if (device_index == -1) {
    qCritical() << "Failed to find device at:" << device_path;
    return;
  }
  const Device::Ptr device = devices.at(device_index);
  const QString device_info = GetDeviceModelCapAndPath(device);

  new_table_warning_frame_->setDevicePath(device_path, device_info);
  main_layout_->setCurrentWidget(new_table_warning_frame_);

  emit q_ptr->coverMainWindowFrameLabelsView(true);
}

void PartitionFramePrivate::showPartitionNumberLimitationFrame() {
  main_layout_->setCurrentWidget(partition_number_limitation_frame_);

  emit q_ptr->coverMainWindowFrameLabelsView(true);
}

void PartitionFramePrivate::showPartitionTableWarningFrame(
    const QString& device_path) {
  partition_table_warning_frame_->setDevicePath(device_path);
  main_layout_->setCurrentWidget(partition_table_warning_frame_);

  emit q_ptr->coverMainWindowFrameLabelsView(false);
}

void PartitionFramePrivate::showSelectBootloaderFrame() {
    q_ptr->m_proxy->showChildFrame(select_bootloader_frame_);
}

void PartitionFramePrivate::showEncryptFrame()
{
    if (full_disk_partition_frame_->validate()) {
        if (!GetSettingsBool(KPartitionSkipFullCryptPage) && this->isEncrypt()) {
            q_ptr->m_proxy->showChildFrame(full_disk_encrypt_frame_);
        }
        else {
            q_ptr->autoPart();
            onPrepareInstallFrameFinished();
        }
    }
}

void PartitionFramePrivate::showDynamicDiskFrame() {
  main_layout_->setCurrentWidget(dynamic_disk_warning_frame_);

  emit q_ptr->coverMainWindowFrameLabelsView(true);
}

void PartitionFramePrivate::showPrepareInstallFrame()
{
    QStringList descriptions;
    if (isSimplePartitionMode()) {
      descriptions = simple_partition_delegate_->getOptDescriptions();
    }
    else if (isFullDiskPartitionMode()) {
        descriptions = full_disk_delegate_->getOptDescriptions();
    } else if (AdvancedPartitionDelegate::install_Lvm_Status != Install_Lvm_Status::Lvm_Install) {
      descriptions = advanced_delegate_->getOptDescriptions();
    } else if (AdvancedPartitionDelegate::install_Lvm_Status == Install_Lvm_Status::Lvm_Install) {
      descriptions = lvm_delegate_->getOptDescriptions();
    }

    qDebug() << "descriptions: " << descriptions;

    prepare_install_frame_->updateDescription(descriptions);
    main_layout_->setCurrentWidget(prepare_install_frame_);

    emit q_ptr->coverMainWindowFrameLabelsView(true);
}

bool PartitionFramePrivate::isEncrypt()
{
    return isFullDiskPartitionMode() && full_disk_partition_frame_->isEncrypt();
}

bool PartitionFramePrivate::isEnSaveData()
{
    return isFullDiskPartitionMode() && full_disk_partition_frame_->isEnSaveData();
}

}  // namespace installer
#include "partition_frame.moc"
