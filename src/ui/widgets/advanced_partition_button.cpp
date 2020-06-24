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

#include "ui/widgets/advanced_partition_button.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QPainter>
#include <QApplication>
#include <QPainterPath>

#include "base/file_util.h"
#include "ui/delegates/partition_util.h"
#include "ui/utils/widget_util.h"
#include "ui/widgets/rounded_progress_bar.h"

namespace installer {

namespace {
const int kMainWindowWidth = 555;
const int kMainWindowHeight = 50;

const int kBtnSize = 25;
const int kItemSpace = 4;

const int kOsIconLeftMargin = 1;
const int kDevicePathLeftMargin = 31;
const int kDiskSizeLeftMarin = 141;
const int kDiskPercentLeftMarin = 218;
const int kDiskPercentHeight = 6;
const int kMountPointLeftMargin = 295;
const int kTipLeftMargin = 375;

const int kFileSystemLeftMargin = 452;
const int kControlButtonLeftMargin = 520;

const int kItemRightMargin = 3;

const int kPartitionInfoFontSize = 9; // 9pt
}  // namespace

AdvancedPartitionButton::AdvancedPartitionButton(const Partition::Ptr partition,
                                                 QWidget* parent)
    : PointerButton(parent),
      partition_(partition),
      editable_(false) {
    setObjectName("advanced_partition_button");

    initUI();
    initConnections();
}

const Partition::Ptr AdvancedPartitionButton::partition() const {
    return partition_;
}

void AdvancedPartitionButton::setEditable(bool editable) {
    editable_ = editable;
    updateStatus();
}

void AdvancedPartitionButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::HighQualityAntialiasing);
    painter.setPen(Qt::NoPen);

    QRect backgroudRect(rect().x() + kItemSpace, rect().y() + kItemSpace
                        , rect().width() - 2 * kItemSpace, rect().height() - kItemSpace);
    QPainterPath path;
    path.addRoundedRect(backgroudRect, 5, 5);
    painter.setClipPath(path);

    // Draw background.
    if (getStatus() == PointerButton::ButtonStatus::Normal) {
        // TODO(chenxiong): use dtk color
        painter.fillRect(backgroudRect, QColor(245,245,245));
    }
    else if (getStatus() == PointerButton::ButtonStatus::Hover) {
        // TODO(chenxiong): use dtk color
        painter.fillRect(backgroudRect, QColor(206,206,206));
    }
    else {
        // TODO(chenxiong): use dtk color
        painter.fillRect(backgroudRect, QColor(245,245,245));
    }

    // Draw OS icon.
    const QPixmap os_icon = installer::renderPixmap(GetOsTypeIcon(partition_->os));
    const qreal ratio = qApp->devicePixelRatio();
    const int os_height = static_cast<int>(os_icon.height()/ ratio) > backgroudRect.height() ?
                backgroudRect.height():static_cast<int>(os_icon.height()/ ratio);
    const int os_width = static_cast<int>(os_icon.width()*os_height/os_icon.height());
    const int x = backgroudRect.x() + kOsIconLeftMargin;
    const int y = backgroudRect.y() + static_cast<int>((backgroudRect.height() - os_height) / 2);
    const QRect os_rect(x, y, os_width, os_height);
    painter.drawPixmap(os_rect, os_icon);

    // Draw partition label name and partition path.
    QString text = GetPartitionLabel(partition_);
    if (partition_->type != PartitionType::Unallocated) {
        text.append(QString("(%1)").arg(GetPartitionName(partition_->path)));
    }
    const QColor text_color(Qt::black);
    painter.setPen(QPen(text_color));

    QFont font;
    font.setPointSize(kPartitionInfoFontSize);
    painter.setFont(font);

    int text_x = std::max(backgroudRect.x() + kDevicePathLeftMargin, os_rect.right() + kItemRightMargin);
    QRect text_rect(text_x, backgroudRect.y(), kDiskSizeLeftMarin - text_x, backgroudRect.height());
    text = painter.fontMetrics().elidedText(text, Qt::TextElideMode::ElideRight, text_rect.width());
    painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);

    // Draw disk size.
    text = GetPartitionUsage(partition_);
    text_rect = QRect(backgroudRect.x() + kDiskSizeLeftMarin, backgroudRect.y(),
           kDiskPercentLeftMarin - (backgroudRect.x() + kDiskSizeLeftMarin),
           backgroudRect.height());
    text = painter.fontMetrics().elidedText(text, Qt::TextElideMode::ElideRight, text_rect.width());
    painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);

    //Draw disk percent.
    const QColor full_color(Qt::gray);
    const QRect full_rect(backgroudRect.x() + kDiskPercentLeftMarin,
          static_cast<int>(backgroudRect.y()+(backgroudRect.height() - kDiskPercentHeight)/2),
          static_cast<int>(kMountPointLeftMargin - (backgroudRect.x() + kDiskPercentLeftMarin)),
          kDiskPercentHeight);
    QPainterPath full_path;
    full_path.addRoundedRect(full_rect, full_rect.height()/2, full_rect.height()/2);
    painter.fillPath(full_path,full_color);

    qreal disk_percent = GetPartitionUsageValue(partition_);
    if (disk_percent < 0) {
        disk_percent = 0.0;
    }
    const QColor percent_color(Qt::blue);
    const QRect percent_rect(backgroudRect.x() + kDiskPercentLeftMarin,
          static_cast<int>(backgroudRect.y() + (backgroudRect.height() - kDiskPercentHeight)/2),
          static_cast<int>((kMountPointLeftMargin - (backgroudRect.x() + kDiskPercentLeftMarin))*disk_percent),
          kDiskPercentHeight);
    QPainterPath percent_path;
    percent_path.addRoundedRect(percent_rect, percent_rect.height()/2, percent_rect.height()/2);
    painter.fillPath(percent_path, percent_color);

    // Draw mount point.
    text = partition_->mount_point;
    text_rect = QRect(backgroudRect.x() + kMountPointLeftMargin, backgroudRect.y(),
           kTipLeftMargin - (backgroudRect.x() + kMountPointLeftMargin),
           backgroudRect.height());
    text = painter.fontMetrics().elidedText(text, Qt::TextElideMode::ElideRight, text_rect.width());
    painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);

    // Draw tip.
    if (partition_->mount_point == kMountPointRoot) {
        text = ::QObject::tr("Install here");
    } else if (partition_->status == PartitionStatus::Format ||
               partition_->status == PartitionStatus::New) {
        text = ::QObject::tr("To be formatted");
    }
    else {
        text.clear();
    }
    text_rect = QRect(backgroudRect.x() + kTipLeftMargin, backgroudRect.y(),
           kFileSystemLeftMargin - (backgroudRect.x() + kTipLeftMargin),
           backgroudRect.height());
    text = painter.fontMetrics().elidedText(text, Qt::TextElideMode::ElideRight, text_rect.width());
    QPen penBak = painter.pen();
    painter.setPen(QColor("#ff8000"));
    painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);
    painter.setPen(penBak);

    // Draw filesystem name.
    text.clear();
    if (partition_->type == PartitionType::Normal ||
        partition_->type == PartitionType::Logical) {
        text = GetFsTypeName(partition_->fs);
    }
    text_rect = QRect(backgroudRect.x() + kFileSystemLeftMargin, backgroudRect.y(),
           kControlButtonLeftMargin - (backgroudRect.x() + kFileSystemLeftMargin),
           backgroudRect.height());
    text = painter.fontMetrics().elidedText(text, Qt::TextElideMode::ElideRight, text_rect.width());

    font.setPointSize(kPartitionInfoFontSize);
    painter.setFont(font);
    painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);

    m_controlButtonPos = QPoint(backgroudRect.x() + kControlButtonLeftMargin
                                , backgroudRect.y() + (backgroudRect.height() - kBtnSize) / 2);

    updateStatus();
    painter.end();
}

void AdvancedPartitionButton::initConnections() {
  connect(control_button_, &QPushButton::clicked,
          this, &AdvancedPartitionButton::onControlButtonClicked, Qt::QueuedConnection);
  connect(this, &QPushButton::toggled,
          this, &AdvancedPartitionButton::onToggled);
}

void AdvancedPartitionButton::initUI() {
  control_button_ = new PointerButton(this);
  control_button_->setObjectName("control_button");
  control_button_->setFlat(true);
  control_button_->setFixedSize(kBtnSize, kBtnSize);
  control_button_->setFocusPolicy(Qt::NoFocus);

  setContentsMargins(0, 0, 0, 0);
  setFixedWidth(kMainWindowWidth);
  setFixedHeight(kMainWindowHeight);
  setCheckable(true);
  setChecked(false);
  setFlat(true);
}

void AdvancedPartitionButton::updateStatus() {
  control_status_ = ControlStatus::Hide;

  if (editable_) {
    if (partition_->type == PartitionType::Normal ||
        partition_->type == PartitionType::Logical) {
      control_status_ = ControlStatus::Delete;
      control_button_->setNormalPic(":/images/partition_delete_normal.svg");
      control_button_->setHoverPic(":/images/partition_delete_hover.svg");
      control_button_->setPressPic(":/images/partition_delete_press.svg");
    }
  } else if (this->isChecked()) {
    if (partition_->type == PartitionType::Normal ||
        partition_->type == PartitionType::Logical) {
      control_status_ = ControlStatus::Edit;
      control_button_->setNormalPic(":/images/edit_normal.svg");
      control_button_->setHoverPic(":/images/edit_hover.svg");
      control_button_->setPressPic(":/images/edit_press.svg");
    } else if (partition_->type == PartitionType::Unallocated) {
      control_status_ = ControlStatus::New;
      control_button_->setNormalPic(":/images/partition_new_normal.svg");
      control_button_->setHoverPic(":/images/partition_new_hover.svg");
      control_button_->setPressPic(":/images/partition_new_press.svg");
    }
  } else {
    control_button_->setIcon(QIcon());
  }

  control_button_->setVisible(control_status_ != ControlStatus::Hide);
  control_button_->style()->unpolish(control_button_);
  control_button_->style()->polish(control_button_);
  control_button_->update();

  if (control_status_ != ControlStatus::Hide) {
      control_button_->move(m_controlButtonPos);
      control_button_->raise();
  }
}

void AdvancedPartitionButton::onControlButtonClicked() {
  switch (control_status_) {
    case ControlStatus::Delete: {
      emit this->deletePartitionTriggered(partition_);
      break;
    }
    case ControlStatus::Edit: {
      emit this->editPartitionTriggered(partition_);
      break;
    }
    case ControlStatus::New: {
      emit this->newPartitionTriggered(partition_);
      break;
    }
    default: {
      // Never reach here.
      break;
    }
  }
}

void AdvancedPartitionButton::onToggled() {
  this->updateStatus();
}

}  // namespace installer
