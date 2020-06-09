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

#include "ui/frames_cli/inner/partition_table_warning_frame.h"



#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "service/power_manager.h"
#include "ui/frames/consts.h"
#include "ui/delegates/advanced_partition_delegate.h"
#include "ui/delegates/partition_util.h"
#include "ui/models_cli/partition_type_model.h"
#include "ui/models/partition_model.h"

namespace installer {


PartitionTableWarningFrame::PartitionTableWarningFrame(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, PartitionModel* model)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      m_partitionModel(model) {
  this->setObjectName("new_partition_frame");
}

void PartitionTableWarningFrame::setDevicePath(const QString& device_path) {
  m_devicePath = device_path;

  this->initUI();
  this->initConnections();
}

void PartitionTableWarningFrame::show()
{
    //if(!m_isshow){
        NCursesWindowBase::show();
        m_isshow = true;


    //}
}

void PartitionTableWarningFrame::hide()
{
    NCursesWindowBase::hide();
    m_isshow = true;
}


void PartitionTableWarningFrame::initConnections() {
  connect(cancel_button_, &NcursesButton::clicked,
          this, &PartitionTableWarningFrame::finished);
  connect(create_button_, &NcursesButton::clicked,
          this, &PartitionTableWarningFrame::onCreateButtonClicked);
}

void PartitionTableWarningFrame::initUI() {
  title_label_ = new NcursesLabel(this, 1, 1, begy(), begx());
  title_label_->setFocusEnabled(false);  
  title_label_->setText(::QObject::tr("Warning"));

  m_commentLab = new NcursesLabel(this, 1, 40, begy(), begx());
  m_commentLab->setFocusEnabled(false);
  m_commentLab->setText(::QObject::tr("You have an EFI boot loader but an MBR disk, thus you cannot install UOS directly. "
                        "Please select one of the below solutions and continue."));
  m_warningBox = new NcursesListView(this, height() - 10, 30, begy(), begx());
  m_warningBox->setFocus(true);

  QStringList waringList;
  waringList.append(QString("1.Restart.%1. %2")
                    .arg(::QObject::tr("Reboot, enter BIOS, and disable UEFI"))
                    .arg(::QObject::tr("Exit BIOS, and install UOS again")));
  waringList.append(QString("2.Format the entire disk. %1")
                    .arg(::QObject::tr("Make a backup of all your data to avoid data loss")));
  waringList.append( QString("3.To select a disk. %1.")
                     .arg(::QObject::tr("Nothing to do")));

  m_warningBox->setList(waringList);

  QString strCancel = ::QObject::tr("Cancel");
  QString strCreate = ::QObject::tr("Create");
  int buttonHeight = 3;
  int buttonWidth = std::max(strCancel.length(), strCreate.length()) + 4;

  cancel_button_ = new NcursesButton(this, strCancel, buttonHeight, 14, begy() + height() - buttonHeight - 2, begx() + 5);
  create_button_ = new NcursesButton(this, strCreate, buttonHeight, 14, begy() + height() - buttonHeight - 2, begx() + width() - buttonWidth - 13);

  cancel_button_->drawShadow(true);
  create_button_->drawShadow(true);

  cancel_button_->box();
  create_button_->box();

  this->setFocus(true);

  m_showChild.push_back(m_warningBox);
  m_showChild.push_back(cancel_button_);
  m_showChild.push_back(create_button_);

}

void PartitionTableWarningFrame::updateTs()
{
    cancel_button_->setText(::QObject::tr("Cancel"));
    create_button_->setText(::QObject::tr("Create"));
    layout();
}

void PartitionTableWarningFrame::layout()
{
    title_label_->adjustSizeByContext();
    title_label_->mvwin(begy(), begx() + (width() - title_label_->width()) / 2);


    m_commentLab->adjustSizeByContext();
    m_commentLab->mvwin(begy() + 1, begx() + 1);
    m_warningBox->adjustSizeByContext();
    m_warningBox->mvwin(begy() + m_commentLab->height() + 1, begx() + 1);



    //cancel_button_->adjustSizeByContext();
    //cancel_button_->mvwin(begy() + height() -3 , begx() + (width() - title_label_->width()) / 2 - 20);

    //create_button_->adjustSizeByContext();
    //create_button_->mvwin(begy() + height() -3 , begx() + (width() - title_label_->width()) / 2 + 20);

    NCursesWindowBase::show();
}

void PartitionTableWarningFrame::onKeyPress(int keycode)
{
    switch (keycode) {
        case KEY_TAB:
        QVector<NCursesWindowBase* > showChild;
        for(NCursesWindowBase* child : m_showChild) {
            if(!child->hidden()) {
                showChild.append(child);
            }
        }

        for (NCursesWindowBase* child : showChild) {
            if(child->isOnFoucs()) {
                int size = showChild.size();
                int index = showChild.indexOf(child);

                int offset = 1;                

                int nextIndex = ( index + offset) % size;
                NCursesWindowBase* nextchild = showChild.at(nextIndex);

                if(nextchild != cancel_button_) {
                    cancel_button_->setFocus(false);
                }

                if(nextchild != create_button_) {
                    create_button_->setFocus(false);
                }
                nextchild->setFocus(true);
                return ;
            }
        }
        break;
    }
}

void PartitionTableWarningFrame::rebootSystem() {

  if (!RebootSystemWithMagicKey()) {
      qWarning() << "RebootSystemWithMagicKey() failed!";
  }

  if (!RebootSystem()) {
      qWarning() << "RebootSystem() failed!";
  }
}

void PartitionTableWarningFrame::keyPresseEvent(int keycode)
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



void PartitionTableWarningFrame::onCreateButtonClicked() {
    if (m_warningBox->getCurrentIndex() == 0) {
        rebootSystem();
    } else if (m_warningBox->getCurrentIndex() == 1) {
        m_partitionModel->createPartitionTable(m_devicePath);
    }

  emit this->finished();
}


}  // namespace installer
