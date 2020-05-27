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

#include "ui/frames/inner/system_info_avatar_frame.h"

#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QStringListModel>
#include <QVBoxLayout>

#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "ui/delegates/avatar_list_delegate.h"
#include "ui/frames/consts.h"
#include "ui/views/pointer_list_view.h"
#include "ui/widgets/avatar_button.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/title_label.h"

namespace installer {

namespace {

// Check whether |avatar| is valid.
bool IsValidAvatar(const QString& avatar) {
  if (avatar.isEmpty()) {
    return false;
  }
  return QFile::exists(avatar);
}

}  // namespace

class SystemInfoAvatarFramePrivate : public QObject{
    Q_OBJECT

public:
    SystemInfoAvatarFramePrivate(SystemInfoAvatarFrame* parent);
    ~SystemInfoAvatarFramePrivate();

private:
    void initUI();
    void initConnections();

    void onListViewPressed(const QModelIndex &index);

    Q_DECLARE_PUBLIC(SystemInfoAvatarFrame)
    SystemInfoAvatarFrame* q_ptr = nullptr;

    QListView* list_view_ = nullptr;
};

SystemInfoAvatarFrame::SystemInfoAvatarFrame(QWidget* parent)
  : QFrame(parent)
    , d_private(new SystemInfoAvatarFramePrivate(this))
{
    setObjectName("system_info_avatar_frame");
}

SystemInfoAvatarFrame::~SystemInfoAvatarFrame()
{
}

void SystemInfoAvatarFrame::readConf() {
  Q_D(SystemInfoAvatarFrame);
  // const QString avatar = d->current_avatar_button_->avatar();
  // emit this->avatarUpdated(avatar);
}

void SystemInfoAvatarFrame::writeConf() {
  Q_D(SystemInfoAvatarFrame);
  // const QString avatar = d->current_avatar_button_->avatar();
  // if (IsValidAvatar(avatar)) {
  //   WriteAvatar(avatar);
  // } else {
  //   qWarning() << "Invalid avatar: " << avatar;
  // }
}

SystemInfoAvatarFramePrivate::SystemInfoAvatarFramePrivate(SystemInfoAvatarFrame *parent)
    : q_ptr(parent)
{
    initUI();
    initConnections();
}

SystemInfoAvatarFramePrivate::~SystemInfoAvatarFramePrivate()
{
}

void SystemInfoAvatarFramePrivate::initUI() {
  const QStringList avatars = GetAvatars();
  list_view_ = new PointerListView();
  QStringListModel* list_model = new QStringListModel(avatars, list_view_);
  list_view_->setModel(list_model);
  AvatarListDelegate* list_delegate = new AvatarListDelegate(list_view_);
  list_view_->setItemDelegate(list_delegate);
  QSizePolicy list_policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // list_policy.setVerticalStretch(100);
  list_view_->setSizePolicy(list_policy);
  list_view_->setContentsMargins(0, 0, 0, 0);
  list_view_->setSpacing(8);
  list_view_->setAcceptDrops(false);
  list_view_->setWrapping(true);
  list_view_->setUniformItemSizes(true);
  list_view_->setFlow(QListView::LeftToRight);
  list_view_->setViewMode(QListView::IconMode);
  list_view_->setFrameShape(QListView::NoFrame);
  list_view_->setFixedWidth(600);
  list_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  list_view_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(list_view_, 0, Qt::AlignHCenter);

  Q_Q(SystemInfoAvatarFrame);
  q->setLayout(layout);
  q->setContentsMargins(0, 0, 0, 0);
}

void SystemInfoAvatarFramePrivate::initConnections() {
  // Return to previous page when chosen_avatar_button is clicked.
  Q_Q(SystemInfoAvatarFrame);
  // connect(current_avatar_button_, &QPushButton::clicked,
  //         q, &SystemInfoAvatarFrame::finished);
  connect(list_view_, &QListView::pressed,
          this, &SystemInfoAvatarFramePrivate::onListViewPressed);
}

void SystemInfoAvatarFramePrivate::onListViewPressed(const QModelIndex& index) {
  const QString avatar = index.model()->data(index).toString();
  Q_Q(SystemInfoAvatarFrame);

  if (IsValidAvatar(avatar)) {
    // current_avatar_button_->updateIcon(avatar);
    emit q->avatarUpdated(avatar);
  } else {
    qWarning() << "Invalid avatar:" << avatar;
  }
  emit q->finished();
}

}  // namespace installer

#include "system_info_avatar_frame.moc"
