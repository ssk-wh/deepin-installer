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

#include "ui/frames/timezone_frame.h"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QApplication>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QProcess>
#include <QScopedPointer>

#include "partman/os_prober.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"
#include "service/timezone_manager.h"
#include "ui/frames/consts.h"
#include "ui/widgets/comment_label.h"
#include "ui/widgets/nav_button.h"
#include "ui/widgets/timezone_map.h"
#include "ui/widgets/title_label.h"
#include "ui/frames/inner/systemdateframe.h"
#include "ui/frames/inner/select_time_zone_frame.h"
#include "ui/widgets/pointer_button.h"
#include "base/file_util.h"

namespace installer {

namespace {

const char kDefaultTimezone[] = "Asia/Shanghai";

}  // namespace

TimezoneFrame::TimezoneFrame(QWidget* parent)
    : QFrame(parent),
      timezone_(),
      alias_map_(GetTimezoneAliasMap()),
      timezone_manager_(new TimezoneManager(this)),
      timezone_source_(TimezoneSource::NotSet) {
  this->setObjectName("system_info_timezone_frame");

  this->initUI();
  this->initConnections();
}

void TimezoneFrame::readConf() {
  // Policy:
  //    * Read default timezone from settings.
  //    * Scan wifi spot.
  //    * Send http request to get geo ip.
  //    * Or wait for user to choose timezone on map.

  // Read timezone from settings.
  timezone_ = GetSettingsString(kTimezoneDefault);
  timezone_ = this->parseTimezoneAlias(timezone_);
  if (IsTimezoneInTab(timezone_)) {
    qDebug() << "timezone updated from settings";
    timezone_source_ = TimezoneSource::Conf;
  } else {
    const bool use_geoip = GetSettingsBool(kTimezoneUseGeoIp);
    const bool use_regdomain = GetSettingsBool(kTimezoneUseRegdomain);
    timezone_manager_->update(use_geoip, use_regdomain);

    // Use default timezone.
    timezone_ = kDefaultTimezone;
  }
  emit this->timezoneUpdated(timezone_);
}

void TimezoneFrame::updateTimezoneBasedOnLanguage(const QString& timezone) {
  // Check priority.
  if (timezone_source_ == TimezoneSource::NotSet ||
      timezone_source_ == TimezoneSource::Language) {
    if (IsTimezoneInTab(timezone)) {
      timezone_source_ = TimezoneSource::Language;
      timezone_ = timezone;
      emit this->timezoneUpdated(timezone_);
    }
  } else {
    qDebug() << "Ignores language default timezone:" << timezone;
  }
}

void TimezoneFrame::writeConf() {
  // Validate timezone before writing to settings file.
  if (!IsTimezoneInTab(timezone_)) {
    qWarning() << "Invalid timezone:" << timezone_;
    timezone_ = kDefaultTimezone;
  }

  WriteTimezone(timezone_);
  WriteIsLocalTime(true);
  WriteIsLocalTimeForce(true);

  QScopedPointer<QProcess> process(new QProcess);
  process->start("timedatectl", {"set-timezone", timezone_});
  process->waitForFinished();

  process->start("timedatectl", {"set-ntp", "false"});
  process->waitForFinished();

  process->start("timedatectl", {"set-time", m_systemDateFrame->timedate()});
  process->waitForFinished();
}

void TimezoneFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(tr("Select Timezone"));

    if(m_mapOrListStackedLayout->currentWidget() == timezone_map_){
        comment_label_->setText(tr("Click your zone on the map"));
    }
    else {
        comment_label_->setText(tr("Select your timezone from the list"));
    }

    next_button_->setText(tr("Next"));
    m_timezoneMapButton->setText(tr("Map"));
    m_timezoneListButton->setText(tr("List"));
    m_setTimePushButton->setText(tr("Time settings"));

    // Also update timezone.
    if (!timezone_.isEmpty()) {
      emit this->timezoneUpdated(timezone_);
    }
  } else {
    QFrame::changeEvent(event);
  }
}

void TimezoneFrame::showEvent(QShowEvent* event) {
  QFrame::showEvent(event);

  qApp->installEventFilter(this);

  // NOTE(xushaohua): Add a delay to wait for paint event of timezone map.
  QTimer::singleShot(0, [&]() {
      if(m_stackedLayout->currentWidget() == m_timezonePage){
          if(m_mapOrListStackedLayout->currentWidget() == timezone_map_){
              timezone_map_->showMark();
          }
          else {
              timezone_map_->hideMark();
          }
          m_setTimePushButton->show();
      }
      else {
          timezone_map_->hideMark();
          m_setTimePushButton->hide();
      }
  });
}

void TimezoneFrame::hideEvent(QHideEvent *event)
{
    qApp->removeEventFilter(this);

    QWidget::hideEvent(event);
}

bool TimezoneFrame::eventFilter(QObject *watched, QEvent *event)
{
    if((event->type() == QEvent::KeyPress)){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Space){
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void TimezoneFrame::initConnections() {
  connect(next_button_, &QPushButton::clicked,
          this, &TimezoneFrame::onNextButtonClicked);

  connect(timezone_manager_, &TimezoneManager::timezoneUpdated,
          this, &TimezoneFrame::onTimezoneManagerUpdated);
  connect(timezone_map_, &TimezoneMap::timezoneUpdated,
          this, &TimezoneFrame::onTimezoneMapUpdated);

  // Remark timezone on map when it is updated.
  connect(this, &TimezoneFrame::timezoneUpdated,
          timezone_map_, &TimezoneMap::setTimezone);
  connect(this, &TimezoneFrame::timezoneUpdated,
          m_selectTimeZoneFrame, &SelectTimeZoneFrame::onUpdateTimezoneList);

  connect(this, &TimezoneFrame::timezoneSet,
          timezone_map_, &TimezoneMap::setTimezoneData);

  connect(m_mapListButtonGroup, static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked)
          , this, &TimezoneFrame::onMapListButtonGroupToggled);

  connect(m_systemDateFrame, &SystemDateFrame::finished, this, [&] {
      this->writeConf();
      emit this->finished();
  });
  connect(m_systemDateFrame, &SystemDateFrame::cancel, this, [=] {
      m_stackedLayout->setCurrentWidget(m_timezonePage);
      if(m_mapOrListStackedLayout->currentWidget() == timezone_map_){
          timezone_map_->showMark();
      }
      else {
          timezone_map_->hideMark();
      }
      m_setTimePushButton->show();
  });

  connect(m_selectTimeZoneFrame, &SelectTimeZoneFrame::timezoneUpdated
          , this, &TimezoneFrame::onSelectTimezoneUpdated);

  connect(m_setTimePushButton, &QPushButton::clicked, this
          , &TimezoneFrame::onSetTimePushButtonClicked);
}

void TimezoneFrame::initUI() {
  title_label_ = new TitleLabel(tr("Select Timezone"));
  comment_label_ = new CommentLabel(tr("Click your zone on the map"));
  timezone_map_ = new TimezoneMap(this);
  next_button_ = new NavButton(tr("Next"));

  m_mapListButtonGroup = new QButtonGroup;
  m_timezoneMapButton = new PointerButton(tr("Map"));
  m_timezoneMapButton->setObjectName("timezoneMapButton");
  m_timezoneMapButton->setCheckable(true);
  m_timezoneMapButton->setFlat(true);
  m_timezoneMapButton->setMinimumWidth(60);
  m_timezoneMapButton->setMaximumHeight(36);
  m_timezoneListButton = new PointerButton(tr("List"));
  m_timezoneListButton->setObjectName("timezoneListButton");
  m_timezoneListButton->setCheckable(true);
  m_timezoneListButton->setFlat(true);
  m_timezoneListButton->setMinimumWidth(60);
  m_timezoneListButton->setMaximumHeight(36);

  m_mapListButtonGroup->addButton(m_timezoneMapButton);
  m_mapListButtonGroup->addButton(m_timezoneListButton);
  m_timezoneMapButton->setChecked(true);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(m_timezoneMapButton, 0, Qt::AlignCenter);
  buttonLayout->addWidget(m_timezoneListButton, 0, Qt::AlignCenter);
  buttonLayout->addStretch();

  m_upLayout = new QVBoxLayout();
  m_upLayout->setContentsMargins(0, 0, 0, 0);
  m_upLayout->setSpacing(kMainLayoutSpacing);
  m_upLayout->addSpacing(kMainLayoutSpacing);
  m_upLayout->addWidget(title_label_, 0, Qt::AlignCenter);
  m_upLayout->addWidget(comment_label_, 0, Qt::AlignCenter);
  m_upLayout->addLayout(buttonLayout);

  m_mapOrListStackedLayout = new QStackedLayout;
  m_selectTimeZoneFrame = new SelectTimeZoneFrame;
  m_mapOrListStackedLayout->addWidget(timezone_map_);
  m_mapOrListStackedLayout->addWidget(m_selectTimeZoneFrame);
  m_mapOrListStackedLayout->setAlignment(Qt::AlignCenter);

  QHBoxLayout* hLayout = new QHBoxLayout;
  hLayout->setMargin(0);
  hLayout->setSpacing(0);
  hLayout->addLayout(m_mapOrListStackedLayout);
  m_upLayout->addLayout(hLayout);

  m_upLayout->addWidget(next_button_, 0, Qt::AlignCenter | Qt::AlignBottom);

  m_timezonePage = new QWidget;
  m_timezonePage->setLayout(m_upLayout);

  m_stackedLayout = new QStackedLayout;
  m_stackedLayout->addWidget(m_timezonePage);

  m_setTimePushButton = new PointerButton();
  m_setTimePushButton->setObjectName("setTimePushButton");
  m_setTimePushButton->setFlat(true);
  m_setTimePushButton->setFixedHeight(23);
  m_setTimePushButton->setText(tr("Time settings"));
  m_setTimePushButton->setNormalPic(":/images/manual_normal.svg");
  m_setTimePushButton->setHoverPic(":/images/manual_hover.svg");
  m_setTimePushButton->setPressPic(":/images/manual_press.svg");

  QSizePolicy spaceRetain = m_setTimePushButton->sizePolicy();
  spaceRetain.setRetainSizeWhenHidden(true);
  m_setTimePushButton->setSizePolicy(spaceRetain);

  m_bottomLayout = new QHBoxLayout();
  m_bottomLayout->setContentsMargins(30, 0, 0, 0);
  m_bottomLayout->setSpacing(30);
  m_bottomLayout->addWidget(m_setTimePushButton);
  m_bottomLayout->addStretch();

  m_setTimePushButton->hide();
  if (!GetSettingsBool(kSkipAutoSyncTimePage)) {
    m_setTimePushButton->show();

    m_systemDateFrame = new SystemDateFrame;
    m_stackedLayout->addWidget(m_systemDateFrame);
  }

  QVBoxLayout* mainLayout = new QVBoxLayout();
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addLayout(m_stackedLayout);
  mainLayout->addLayout(m_bottomLayout);

  setLayout(mainLayout);
  setContentsMargins(0, 0, 0, 0);
  setStyleSheet(ReadFile(":/styles/timezone_frame.css"));
}

QString TimezoneFrame::parseTimezoneAlias(const QString& timezone) {
  // If |timezone| not in alias map, returns itself.
  return alias_map_.value(timezone, timezone);
}

void TimezoneFrame::onNextButtonClicked() {
  if (IsTimezoneInTab(timezone_)) {
    this->writeConf();
    emit this->timezoneUpdated(timezone_);
    // Emit finished() signal only if a valid timezone is specified.
    emit this->finished();
  } else {
    qWarning() << "Invalid timezone:" << timezone_;
  }
}

void TimezoneFrame::onTimezoneManagerUpdated(const QString& timezone) {
  // Check priority.
  if (timezone_source_ == TimezoneSource::NotSet ||
      timezone_source_ == TimezoneSource::Language ||
      timezone_source_ == TimezoneSource::Scan) {
    // Update timezone only if it is not set.
    timezone_source_ = TimezoneSource::Scan;
    timezone_ = this->parseTimezoneAlias(timezone);
    emit this->timezoneUpdated(timezone_);
  } else {
    qDebug() << "Ignore timezone value from timezone-manager:" << timezone;
  }
}

void TimezoneFrame::onTimezoneMapUpdated(const QString& timezone) {
  timezone_source_ = TimezoneSource::User;
  // No need to convert timezone alias.
  timezone_ = timezone;
  m_selectTimeZoneFrame->onUpdateTimezoneList(timezone);
}

void TimezoneFrame::onMapListButtonGroupToggled(QAbstractButton *button)
{
    if (button == m_timezoneMapButton){
        comment_label_->setText(tr("Click your zone on the map"));
        m_mapOrListStackedLayout->setCurrentWidget(timezone_map_);
        timezone_map_->showMark();
    }
    else{
        comment_label_->setText(tr("Select your timezone from the list"));
        m_mapOrListStackedLayout->setCurrentWidget(m_selectTimeZoneFrame);
        timezone_map_->hideMark();
    }
}

void TimezoneFrame::onSelectTimezoneUpdated(const QString &timezone)
{
    timezone_source_ = TimezoneSource::User;
    timezone_ = timezone;
    emit timezoneSet(timezone);
}

void TimezoneFrame::onSetTimePushButtonClicked()
{
    m_stackedLayout->setCurrentWidget(m_systemDateFrame);
    timezone_map_->hideMark();
    m_setTimePushButton->hide();
}

}  // namespace installer
