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

#include "ui/widgets/timezone_map.h"

#include <QDebug>
#include <QItemSelectionModel>
#include <QLabel>
#include <QListView>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include "service/screen_adaptation_manager.h"
#include "base/file_util.h"
#include "../utils/widget_util.h"
#include "service/settings_manager.h"
#include "ui/delegates/timezone_map_util.h"
#include "ui/widgets/popup_menu.h"
#include "ui/widgets/tooltip_pin.h"

namespace installer {

namespace {

static const int kZonePinHeight = 30;
static const int kZonePinMinimumWidth = 60;

static const double kDistanceThreshold = 50.0;
static const QString kDotFile = ":/images/position.svg";
static const QString kTimezoneMapFile = ":/images/map.svg";

// At absolute position of |zone| on a map with size (map_width, map_height).
QPoint ZoneInfoToPosition(const ZoneInfo& zone, int map_width, int map_height) {
  // TODO(xushaohua): Call round().
  const int x = int(ConvertLongitudeToX(zone.longitude) * map_width);
  const int y = int(ConvertLatitudeToY(zone.latitude) * map_height);
  return QPoint(x, y);
}

}  // namespace

TimezoneMap::TimezoneMap(QWidget* parent)
    : QFrame(parent),
      current_zone_(),
      total_zones_(GetZoneInfoList()),
      nearest_zones_(),
      m_mapLabelSize()
{
  this->setObjectName("timezone_map");

  this->initUI();
  this->initConnections();
}

TimezoneMap::~TimezoneMap() {
}

const QString TimezoneMap::getTimezone() const {
  return current_zone_.timezone;
}

void TimezoneMap::setTimezone(const QString& timezone) {
  setTimezoneData(timezone);
  QTimer::singleShot(0, this, &TimezoneMap::updateMap);
}

void TimezoneMap::setTimezoneData(const QString& timezone) {
  const int index = GetZoneInfoByZone(total_zones_, timezone);
  if (index > -1 && index < total_zones_.size()) {
    current_zone_ = total_zones_.at(index);
    nearest_zones_.clear();
    nearest_zones_.append(current_zone_);
  } else {
    qWarning() << "Timezone not found:" << timezone;
  }
}

void TimezoneMap::hideMark()
{
    popup_window_->hide();
    zone_pin_->hide();
    dot_->hide();
}

void TimezoneMap::showMark()
{
    updateMap();
}

//void TimezoneMap::resizeEvent(QResizeEvent* event) {
//  QTimer::singleShot(0, this, &TimezoneMap::updateMap);

//  QWidget::resizeEvent(event);
//}

bool TimezoneMap::eventFilter(QObject* watched, QEvent* event) {
    if (watched == map_label_) {
        if (event->type() == QMouseEvent::MouseButtonPress) {
            QMouseEvent* e = reinterpret_cast<QMouseEvent*>(event);
            // Get nearest zones around mouse.
            nearest_zones_ =
                GetNearestZones(total_zones_, kDistanceThreshold, e->x(), e->y(),
                                map_label_->width(), map_label_->height());
            qDebug() << nearest_zones_;
            if (nearest_zones_.length() == 1) {
                current_zone_ = nearest_zones_.first();
                remark();
                emit timezoneUpdated(current_zone_.timezone);
            }
            else {
                popupZoneWindow(e->pos());
            }

            m_mapLabelSize = map_label_->size();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void TimezoneMap::initConnections() {
  // Hide dot when popup-zones window is hidden.
  connect(popup_window_, &PopupMenu::onHide,
          dot_, &QLabel::hide);

  // Hide popup_window_ and mark new timezone on map.
  connect(popup_window_, &PopupMenu::menuActivated,
          this, &TimezoneMap::onPopupWindowActivated);
}

void TimezoneMap::initUI() {
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  layout->setSpacing(0);

  map_label_ = new QLabel(this);
  map_label_->installEventFilter(this);

  Q_ASSERT(this->parentWidget());
  // Set parent widget of dot_ to TimezoneFrame.
  dot_ = new QLabel(this);
  QPixmap dot_pixmap;
  if (ScreenAdaptationManager::instance()->is4KScreen()) {
    dot_pixmap = installer::renderPixmap(kDotFile);
  }
  else {
    dot_pixmap = ScreenAdaptationManager::instance()->adapterPixmap(kDotFile);
  }

  Q_ASSERT(!dot_pixmap.isNull());
  dot_->setPixmap(dot_pixmap);
  dot_->setFixedSize(dot_pixmap.size() / devicePixelRatioF());
  dot_->hide();

  // Set parent widget of zone_pin_ to TimezoneFrame.
  zone_pin_ = new TooltipPin(this);
  zone_pin_->setFixedHeight(kZonePinHeight);
  zone_pin_->setMinimumWidth(kZonePinMinimumWidth);
  // Allow mouse event to pass through.
  zone_pin_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  zone_pin_->hide();

  popup_window_ = new PopupMenu(this);
  popup_window_->hide();

  layout->addWidget(map_label_, 0, Qt::AlignCenter);
  setLayout(layout);

  updateMap();
}

void TimezoneMap::popupZoneWindow(const QPoint& pos) {
  // Hide all marks first.
  dot_->hide();
  zone_pin_->hide();
  popup_window_->hide();

  // Popup zone list window.
  QStringList zone_names;
  const QString locale = ReadLocale();
  for (const ZoneInfo& zone : nearest_zones_) {
    zone_names.append(GetLocalTimezoneName(zone.timezone, locale).second);
  }

  // Show popup window above dot
  popup_window_->setStringList(zone_names);

  const int half_width = dot_->width() / 2;
  const int half_height = dot_->height() / 2;
  // Position relative to parent.
  const QPoint parent_pos(map_label_->mapToParent(pos));

  // Add 8px margin.
  const QPoint popup_pos(parent_pos.x(), parent_pos.y() - half_height - 8);
  popup_window_->popup(popup_pos);
  popup_window_->raise();
  popup_window_->show();

  const QPoint dot_pos(parent_pos.x() - half_width,
                       parent_pos.y() - half_height);
  dot_->move(dot_pos);
  dot_->show();
  dot_->raise();
}

void TimezoneMap::remark() {
    if (!isVisible()) {
        return;
    }

  // Hide all marks first.
  dot_->hide();
  zone_pin_->hide();
  popup_window_->hide();

  const int map_width = map_label_->width();
  const int map_height = map_label_->height();

  Q_ASSERT(!nearest_zones_.isEmpty());
  if (!nearest_zones_.isEmpty()) {
    const QString locale = ReadLocale();
    zone_pin_->setText(GetLocalTimezoneName(current_zone_.timezone, locale).second);
    // Adjust size of pin to fit its content.
    zone_pin_->adjustSize();

    // Show zone pin at current marked zone.
    const QPoint zone_pos = ZoneInfoToPosition(current_zone_, map_width,
                                               map_height);

    const int half_width = dot_->width() / 2;
    const int half_height = dot_->height() / 2;
    const QPoint parent_pos(map_label_->mapToParent(zone_pos));

    // Add 2px margin.
    const QPoint zone_pin_pos(parent_pos.x(), parent_pos.y() - half_height - 2);
    zone_pin_->popup(zone_pin_pos);
    zone_pin_->raise();

    const QPoint dot_pos(parent_pos.x() - half_width,
                         parent_pos.y() - half_height);
    dot_->move(dot_pos);
    dot_->raise();
    dot_->show();
    dot_->raise();
  }
}

void TimezoneMap::updateMap() {
    QPixmap mapPixmap;

    if (ScreenAdaptationManager::instance()->is4KScreen()) {
      mapPixmap = ScreenAdaptationManager::instance()->adapterPixmap(kTimezoneMapFile);
    }
    else {
      mapPixmap = installer::renderPixmap(kTimezoneMapFile);
    }

    mapPixmap = mapPixmap.scaled(QSize(size().width() - 100, size().height()), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    map_label_->setPixmap(mapPixmap);

    QTimer::singleShot(0, this, &TimezoneMap::remark);
}

void TimezoneMap::showHistoryTimeZone()
{
    this->setTimezoneData(current_zone_.timezone);
}

void TimezoneMap::onPopupWindowActivated(int index) {
  // Hide popup window and dot first.
  popup_window_->hide();
  dot_->hide();
  dot_->raise();

  // Update current selected timezone and mark it on map.
  Q_ASSERT(index < nearest_zones_.length());
  if (index < nearest_zones_.length()) {
    current_zone_ = nearest_zones_.at(index);
    this->remark();
    emit this->timezoneUpdated(current_zone_.timezone);
  }
}

}  // namespace installer
