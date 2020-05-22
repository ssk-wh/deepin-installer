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

#include "ui/frames_cli/inner/prepare_install_frame.h"



#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/frames/consts.h"
#include "ui/delegates/advanced_partition_delegate.h"
#include "ui/delegates/partition_util.h"
#include "ui/models_cli/partition_type_model.h"

namespace installer {

namespace {
// Minimum size of new partition is 100 Mib.
const qint64 kMinimumPartitionSizeCli = 100 * kMebiByte;
}  // namespace

PrepareInstallFrame::PrepareInstallFrame(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, AdvancedPartitionDelegate* delegate)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      delegate_(delegate),
      partition_()
      {
    this->setObjectName("PrepareInstallFrame");
    initUI();
    initConnections();
}

void PrepareInstallFrame::show()
{
    //if(!m_isshow){
        NCursesWindowBase::show();
        m_isshow = true;
    //}
}

void PrepareInstallFrame::hide()
{
    NCursesWindowBase::hide();
    m_isshow = true;
}


void PrepareInstallFrame::initConnections() {

  connect(cancel_button_, &NcursesButton::clicked,
          this, &PrepareInstallFrame::onBackButtonClicked);
  connect(create_button_, &NcursesButton::clicked,
          this, &PrepareInstallFrame::onCreateButtonClicked);
}

void PrepareInstallFrame::initUI() {

  m_titleLabel_ = new NcursesLabel(this, 1, 1, begy(), begx());
  m_titleLabel_->setFocusEnabled(false);
  m_titleLabel_->setText(tr("Ready to Install"));

  m_commentLabel = new NcursesLabel(this, 1, 1, begy() , begx());
  m_commentLabel->setFocusEnabled(false);
  m_commentLabel->setText(tr("Make a backup of your important data and then continue"));

  operations_box_ = new NcursesListView(this, height() - 10, 80, begy(), begx());
  QStringList opt(delegate_->getOptDescriptions());
  operations_box_->setList(opt);
  operations_box_->setFocus(true);

  cancel_button_ = new NcursesButton(this, tr("Back"), 2, 8, begy() + height() - 10 + 3, begx());
  create_button_ = new NcursesButton(this, tr("Continue"), 2, 8, begy() + height() - 10 + 3, begx() + 12);

  this->setFocus(true);

  m_showChild.push_back(operations_box_);
  m_showChild.push_back(cancel_button_);
  m_showChild.push_back(create_button_);

}

void PrepareInstallFrame::updateTs()
{

    cancel_button_->setText(tr("Back"));
    create_button_->setText(tr("Continue"));
    layout();
}

void PrepareInstallFrame::layout()
{
    m_titleLabel_->adjustSizeByContext();
    m_titleLabel_->mvwin(begy(), begx() + (width() - m_titleLabel_->width()) / 2);

    m_commentLabel->adjustSizeByContext();
    m_commentLabel->mvwin(begy() + 2, begx() +40);

    operations_box_->adjustSizeByContext();
    operations_box_->mvwin(begy() + 3, begx() + 40);

    cancel_button_->adjustSizeByContext();
    cancel_button_->mvwin(begy() + height() -3 , begx() + (width() - m_titleLabel_->width()) / 2 - 20);

    create_button_->adjustSizeByContext();
    create_button_->mvwin(begy() + height() -3 , begx() + (width() - m_titleLabel_->width()) / 2 + 20);

    NCursesWindowBase::show();
}

void PrepareInstallFrame::onKeyPress(int keycode)
{
    switch (keycode) {
        case KEY_TAB: break;
        case KEY_RIGHT:
        case KEY_LEFT:
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
                if (keycode == KEY_LEFT) {
                    if (index == 0) {
                        offset = size -1;
                    } else {
                        offset = -1;
                    }
                }

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

void PrepareInstallFrame::keyPresseEvent(int keycode)
{
    if (keycode == KEY_TAB) return ;

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



void PrepareInstallFrame::onCreateButtonClicked()
{
     emit this->finished(true);
}

void PrepareInstallFrame::onBackButtonClicked()
{
     emit this->finished(false);
}


}  // namespace installer