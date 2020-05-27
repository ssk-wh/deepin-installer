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

#include "ui/frames/disk_space_insufficient_frame.h"

#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/partition_util.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/title_label.h"
#include "ui/delegates/main_window_util.h"

#include <QEvent>
#include <QHBoxLayout>

DCORE_USE_NAMESPACE

namespace installer {

namespace {

// Get content of comment label.
// Value of minimum disk space is changed based on size of physical memory.
QString GetCommentLabel() {
  int minimum = GetSettingsInt(kPartitionMinimumDiskSpaceRequired);
  if (minimum <= 0) {
    minimum = qMin(GetSettingsInt(kPartitionRootMiniSpace)
                   , GetSettingsInt(kPartitionFullDiskMiniSpace));
  }

  const int recommended = GetSettingsInt(kPartitionRecommendedDiskSpace);
  return QObject::tr("You need at least %1 GB disk space to install %2. "
                     "To get better performance, %3 GB or more is recommended")
      .arg(minimum)
      .arg(DSysInfo::productType() == DSysInfo::Deepin ? QObject::tr("Deepin") : QObject::tr("UOS"))
      .arg(recommended);
}

}  // namespace

DiskSpaceInsufficientFrame::DiskSpaceInsufficientFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
  : FrameInterface(frameProxyInterface, parent) {
  setObjectName("disk_space_insufficient_frame");

  initUI();
  initConnections();
}

void DiskSpaceInsufficientFrame::init()
{

}

void DiskSpaceInsufficientFrame::finished()
{

}

bool DiskSpaceInsufficientFrame::shouldDisplay() const
{
    return !GetSettingsBool(kSkipDiskSpaceInsufficientPage) &&
                IsDiskSpaceInsufficient();
}

void DiskSpaceInsufficientFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(tr("Insufficient Disk Space"));
    comment_label_->setText(GetCommentLabel());
    abort_button_->setText(tr("Exit"));
  } else {
    FrameInterface::changeEvent(event);
  }
}

void DiskSpaceInsufficientFrame::initConnections() {
  connect(abort_button_, &QPushButton::clicked, this, [=] {
          m_proxy->nextFrame();
  });
}

void DiskSpaceInsufficientFrame::initUI() {
  title_label_ = new TitleLabel(tr("Insufficient Disk Space"));
  comment_label_ = new CommentLabel(GetCommentLabel());
  QHBoxLayout* comment_layout = new QHBoxLayout();
  comment_layout->setContentsMargins(0, 0, 0, 0);
  comment_layout->addSpacing(0);
  comment_layout->addWidget(comment_label_);

  abort_button_ = new NavButton(tr("Exit"));

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(kMainLayoutSpacing);
  layout->addStretch();
  layout->addWidget(title_label_, 0, Qt::AlignCenter);
  layout->addLayout(comment_layout);
  layout->addStretch();
  layout->addWidget(abort_button_, 0, Qt::AlignCenter);
  setLayout(layout);
  setContentsMargins(0, 0, 0, 0);
}

}  // namespace installer
