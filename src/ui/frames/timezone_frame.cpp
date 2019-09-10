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
#include <QHBoxLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QStackedLayout>

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
}

void TimezoneFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    title_label_->setText(tr("Select Time Zone"));
    comment_label_->setText(tr("Mark your zone on the map"));
    next_button_->setText(tr("Next"));

    // Also update timezone.
    if (!timezone_.isEmpty()) {
      timezone_map_->setTimezone(timezone_);
      emit this->timezoneUpdated(timezone_);
    }
  } else {
    QFrame::changeEvent(event);
  }
}

void TimezoneFrame::showEvent(QShowEvent* event) {
  QFrame::showEvent(event);

  next_button_->setFocus();

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

  connect(this, &TimezoneFrame::timezoneSet,
          timezone_map_, &TimezoneMap::setTimezoneData);

  connect(m_listSelectedCheckBox, &QCheckBox::clicked, this
          , &TimezoneFrame::onListSelectedCheckBoxClicked);
  connect(m_systemDateFrame, &SystemDateFrame::finished, this, &TimezoneFrame::finished);
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
  title_label_ = new TitleLabel(tr("Select Time Zone"));
  comment_label_ = new CommentLabel(tr("Mark your zone on the map"));
  timezone_map_ = new TimezoneMap(this);
  next_button_ = new NavButton(tr("Next"));
  m_listSelectedCheckBox = new QCheckBox;
  m_listSelectedCheckBox->setText(tr("Set your date and time manually"));
  m_listSelectedCheckBox->setObjectName("listSelectedCheckBox");

  m_listSelectedCheckBox->setChecked(false);

  m_upLayout = new QVBoxLayout();
  m_upLayout->setContentsMargins(0, 0, 0, 0);
  m_upLayout->setSpacing(kMainLayoutSpacing);
  m_upLayout->addStretch();
  m_upLayout->addWidget(title_label_, 0, Qt::AlignCenter);
  m_upLayout->addWidget(comment_label_, 0, Qt::AlignCenter);
  m_upLayout->addWidget(m_listSelectedCheckBox, 0, Qt::AlignCenter);
  m_upLayout->addStretch();

  m_mapOrListStackedLayout = new QStackedLayout;
  m_selectTimeZoneFrame = new SelectTimeZoneFrame;
  m_mapOrListStackedLayout->addWidget(timezone_map_);
  m_mapOrListStackedLayout->addWidget(m_selectTimeZoneFrame);

  QHBoxLayout* hLayout = new QHBoxLayout;
  hLayout->setMargin(0);
  hLayout->setSpacing(0);
  hLayout->addStretch();
  hLayout->addLayout(m_mapOrListStackedLayout);
  hLayout->addStretch();
  m_upLayout->addLayout(hLayout);

  m_upLayout->addStretch();
  m_upLayout->addWidget(next_button_, 0, Qt::AlignCenter);

  m_timezonePage = new QWidget;
  m_timezonePage->setLayout(m_upLayout);

  m_stackedLayout = new QStackedLayout;
  m_stackedLayout->addWidget(m_timezonePage);

  m_listSelectedCheckBox->hide();
  if (!GetSettingsBool(kSkipAutoSyncTimePage)) {
    m_listSelectedCheckBox->show();

    m_systemDateFrame = new SystemDateFrame;
    m_stackedLayout->addWidget(m_systemDateFrame);
  }

  m_setTimePushButton = new PointerButton();
  m_setTimePushButton->setObjectName("setTimePushButton");
  m_setTimePushButton->setFlat(true);
  m_setTimePushButton->setFixedHeight(23);
  m_setTimePushButton->setText(tr("Time setting"));

  m_bottomLayout = new QHBoxLayout();
  m_bottomLayout->setContentsMargins(30, 0, 0, 0);
  m_bottomLayout->setSpacing(30);
  m_bottomLayout->addWidget(m_setTimePushButton);
  m_bottomLayout->addStretch();

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
  emit this->timezoneUpdated(timezone_);
}

void TimezoneFrame::onListSelectedCheckBoxClicked(bool checked)
{
    if(checked){
        m_mapOrListStackedLayout->setCurrentWidget(m_selectTimeZoneFrame);
        timezone_map_->hideMark();
    }
    else {
        m_mapOrListStackedLayout->setCurrentWidget(timezone_map_);
        timezone_map_->showMark();
    }
}

void TimezoneFrame::onSelectTimezoneUpdated(const QString &timezone)
{
    timezone_source_ = TimezoneSource::User;
    timezone_ = this->parseTimezoneAlias(timezone);
    emit timezoneSet(timezone_);
}

void TimezoneFrame::onSetTimePushButtonClicked()
{
    m_stackedLayout->setCurrentWidget(m_systemDateFrame);
    timezone_map_->hideMark();
    m_setTimePushButton->hide();
}

}  // namespace installer
