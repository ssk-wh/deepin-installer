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
//const qint64 kMinimumPartitionSizeCli = 100 * kMebiByte;
}  // namespace

/*PrepareInstallFrame::PrepareInstallFrame(NCursesWindowBase* parent, int lines, int cols, int beginY, int beginX, AdvancedPartitionDelegate* delegate)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      delegate_(delegate),
      partition_()
      {
    this->setObjectName("PrepareInstallFrame");
    initUI();
    initConnections();
}*/

PrepareInstallFrame::PrepareInstallFrame(NCursesWindowBase *parent, int lines, int cols, int beginY, int beginX, QStringList optdescriptions)
    : FrameInterfacePrivate(parent, lines, cols, beginY, beginX),
      m_optDescriptions(optdescriptions)
{
    this->setObjectName("PrepareInstallFrame");
    initUI();
    initConnections();
}

void PrepareInstallFrame::show()
{
    if(!m_isshow){
        FrameInterfacePrivate::show();
        m_isshow = true;
        create_button_->setFocus(true);
    }
}

void PrepareInstallFrame::hide()
{
    NCursesWindowBase::hide();
    m_isshow = false;
}


void PrepareInstallFrame::initConnections() {

  connect(cancel_button_, &NcursesButton::clicked,
          this, &PrepareInstallFrame::onBackButtonClicked);
  connect(create_button_, &NcursesButton::clicked,
          this, &PrepareInstallFrame::onCreateButtonClicked);
}

void PrepareInstallFrame::initUI() {

    setBackground(NcursesUtil::getInstance()->dialog_attr());
    this->drawShadow(true);
    this->box();

  m_titleLabel_ = new NcursesLabel(this, 1, 1, begy(), begx());
  m_titleLabel_->setFocusEnabled(false);
  m_titleLabel_->setText(::QObject::tr("Ready to Install"));

  m_commentLabel = new NcursesLabel(this, 1, 1, begy() , begx());
  m_commentLabel->setFocusEnabled(false);
  m_commentLabel->setText(::QObject::tr("Make a backup of your important data and then continue"));

  //QStringList opt(delegate_->getOptDescriptions());

  QString strBack = ::QObject::tr("Back");
  QString strContinue = ::QObject::tr("Continue");

  cancel_button_ = new NcursesButton(this, strBack, 3, 14, begy() + height() - 5, begx() + 5);

  operations_box_ = new NcursesTextBrower(this, height() - 10, width() - 4, begy() + 4, begx() + 2);
  foreach (QString testoptions, m_optDescriptions) {
      operations_box_->appendItemText(testoptions);
  }
  operations_box_->setFocusEnabled(false);

  create_button_ = new NcursesButton(this, strContinue, 3, 14, begy() + height() - 5, begx() + width() - 20);

  cancel_button_->drawShadow(true);
  create_button_->drawShadow(true);
  cancel_button_->box();
  create_button_->box();

}

void PrepareInstallFrame::updateTs()
{
    cancel_button_->setText(::QObject::tr("Back"));
    create_button_->setText(::QObject::tr("Continue"));
    layout();
}

void PrepareInstallFrame::layout()
{
    m_titleLabel_->adjustSizeByContext();
    m_titleLabel_->mvwin(begy(), begx() + (width() - m_titleLabel_->width()) / 2);

    m_commentLabel->adjustSizeByContext();
    m_commentLabel->mvwin(begy() + 2, begx() + (width() - m_commentLabel->width()) / 2);

    //operations_box_->adjustSizeByContext();
    //operations_box_->mvwin(begy() + 3, begx() + (width() - operations_box_->width()) / 2);

    //cancel_button_->adjustSizeByContext();
    //cancel_button_->mvwin(begy() + height() -3 , begx() + (width() - m_titleLabel_->width()) / 2 - 20);

    //create_button_->adjustSizeByContext();
    //create_button_->mvwin(begy() + height() -3 , begx() + (width() - m_titleLabel_->width()) / 2 + 20);

    //NCursesWindowBase::show();
}

void PrepareInstallFrame::onKeyPress(int keycode)
{
    switch (keycode) {
    case KEY_TAB:
        switchChildWindowsFoucs();
        break;
    }

    qDebug()<< keycode;
}

void PrepareInstallFrame::keyPresseEvent(int keycode)
{
    if (!m_isshow) {
        if (m_currentchoicetype != -1) {
            emit keyEventTrigerSignal(keycode);
        }
        return;
    } else {
        if (m_currentchoicetype != -1) {
            emit keyEventTrigerSignal(keycode);
        } else {
            emit keyEventTrigerSignal(keycode);
            FrameInterfacePrivate::keyEventTriger(keycode);
        }
    }
}

void PrepareInstallFrame::backHandle()
{
    Q_EMIT emit this->finished(false);
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
