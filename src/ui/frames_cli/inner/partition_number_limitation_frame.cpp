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

#include "ui/frames_cli/inner/partition_number_limitation_frame.h"



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


PartitionNumberLimitationFrame::PartitionNumberLimitationFrame(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, PartitionModel* model)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      m_partitionModel(model) {
  this->setObjectName("partition_number_limitation_frame");
}

void PartitionNumberLimitationFrame::inits() {
  this->initUI();
  this->initConnections();
}

void PartitionNumberLimitationFrame::show()
{
     FrameInterfacePrivate::show();
     m_isshow = true;
}

void PartitionNumberLimitationFrame::hide()
{
    NCursesWindowBase::hide();
    m_isshow = true;
}


void PartitionNumberLimitationFrame::initConnections() {
  connect(cancel_button_, &NcursesButton::clicked,
          this, &PartitionNumberLimitationFrame::finished);
}

void PartitionNumberLimitationFrame::initUI() {
  this->drawShadow(true);
  this->box();

  printTitle(::QObject::tr("Failed to Create New Partition"), width());

  m_commentLab = new NcursesLabel(this, 1, 40, begy(), begx());
  m_commentLab->setFocusEnabled(false);
  m_commentLab->setText(::QObject::tr("You should delete a primary partition before creating a new one, as there can only be four primary partitions on an MBR disk"));

  m_commentLab1 = new NcursesLabel(this, height() - 10, 30, begy(), begx());
  m_commentLab1->setFocusEnabled(false);
  m_commentLab1->setText(::QObject::tr("You should delete a primary partition before creating a logical one, or move the existing logical partition to the end of the disk"));

  QString strCancel = ::QObject::tr("Back");
  int buttonHeight = 3;
  int buttonWidth = strCancel.length() + 4;

  cancel_button_ = new NcursesButton(this, strCancel, 3, 14, begy() + height() - 5, begx() + (width() - buttonWidth) / 2 );

  cancel_button_->drawShadow(true);

  cancel_button_->box();

  this->setFocus(true);

}

void PartitionNumberLimitationFrame::updateTs()
{
    box(ACS_VLINE,ACS_HLINE);
    printTitle(::QObject::tr("Failed to Create New Partition"), width());
    cancel_button_->setText(::QObject::tr("Back"));
    layout();
}

void PartitionNumberLimitationFrame::layout()
{
    m_commentLab->adjustSizeByContext();
    m_commentLab->mvwin(begy() + 1, begx() + 1);
    m_commentLab1->adjustSizeByContext();
    m_commentLab1->mvwin(begy() + m_commentLab->height() + 1, begx() + 1);

    NCursesWindowBase::show();
}

void PartitionNumberLimitationFrame::onKeyPress(int keycode)
{
    switch (keycode) {
    case KEY_TAB:
            switchChildWindowsFoucs();
        break;
    }

    qDebug()<< keycode;
}


void PartitionNumberLimitationFrame::keyPresseEvent(int keycode)
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
            emit keyEventTrigerSignal(keycode);
            FrameInterfacePrivate::keyEventTriger(keycode);
        }
   }
}

void PartitionNumberLimitationFrame::backHandle()
{
    Q_EMIT doBackBtnClickedSignal();
}


}  // namespace installer
