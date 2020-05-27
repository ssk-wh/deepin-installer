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
#include "ui/widgets/timezone_map.h"
#include "ui/widgets/title_label.h"
#include "ui/frames/inner/systemdateframe.h"
#include "ui/frames/inner/select_time_zone_frame.h"
#include "ui/widgets/pointer_button.h"
#include "base/file_util.h"
#include "ui/interfaces/frameinterfaceprivate.h"

DWIDGET_USE_NAMESPACE

namespace installer {

// Priority of timezone: User > Conf > Scan
enum class TimezoneSource {
  NotSet,  // Timezone not set.
  User,  // Timezone is setup by user.
  Conf,  // Timezone is read from conf file
  Scan,  // Timezone is updated based on geoip or regdomain
  Language,  // Timezone is setup based on current selected language.
};

namespace {

const char kDefaultTimezone[] = "Asia/Shanghai";

}  // namespace

class TimezoneFramePrivate : public FrameInterfacePrivate
{
    Q_OBJECT
public:
  explicit TimezoneFramePrivate(FrameInterface* parent)
      : FrameInterfacePrivate(parent)
      , q_ptr(qobject_cast<TimezoneFrame* >(parent))
      , timezone_()
      , alias_map_(GetTimezoneAliasMap())
      , timezone_manager_(new TimezoneManager(this))
      , timezone_source_(TimezoneSource::NotSet)
  {}

  TimezoneFrame* q_ptr;

  void initConnections();
  void initUI();
  bool validate() const;

  // Convert timezone alias to its original name.
  QString parseTimezoneAlias(const QString& timezone);

  QString timezone_;
  TimezoneAliasMap alias_map_;

  TimezoneManager* timezone_manager_ = nullptr;

  TitleLabel* title_label_ = nullptr;
  CommentLabel* comment_label_ = nullptr;
  DButtonBoxButton* m_timezoneMapButton = nullptr;
  DButtonBoxButton* m_timezoneListButton = nullptr;
  DButtonBox* m_mapListButtonGroup = nullptr;
  TimezoneMap* timezone_map_ = nullptr;
  SystemDateFrame* m_systemDateFrame = nullptr;
  SelectTimeZoneFrame* m_selectTimeZoneFrame = nullptr;
  QStackedLayout* m_mapOrListStackedLayout = nullptr;
  QVBoxLayout* m_upLayout = nullptr;
  QHBoxLayout* m_bottomLayout = nullptr;
  QList<DButtonBoxButton *> m_buttonList;

  QWidget* m_timezonePage = nullptr;

  TimezoneSource timezone_source_;

  // Update timezone after receiving signals from timezone manager.
  void onTimezoneManagerUpdated(const QString& timezone);

  // Update timezone after a new one has been chosen by user.
  void onTimezoneMapUpdated(const QString& timezone);

  void onMapListButtonGroupToggled(QAbstractButton* button);

  void onSelectTimezoneUpdated(const QString& timezone);

  void onSetTimePushButtonClicked();
  void updateTs();
};

void TimezoneFramePrivate::updateTs()
{
    title_label_->setText(tr("Select Timezone"));
    nextButton->setText(tr("Next"));
    m_timezoneMapButton->setText(tr("Map"));
    m_timezoneListButton->setText(tr("List"));
}

TimezoneFrame::TimezoneFrame(FrameProxyInterface* frameProxyInterface, QWidget* parent)
    : FrameInterface(frameProxyInterface, parent)
    , m_private(new TimezoneFramePrivate(this))
{
  setObjectName("system_info_timezone_frame");

  m_private->initUI();
  m_private->initConnections();
}

TimezoneFrame::~TimezoneFrame()
{

}

bool TimezoneFrame::shouldDisplay() const
{
    return !(GetSettingsBool(kSystemInfoSetupAfterReboot) || GetSettingsBool(kSkipTimezonePage));
}

QString TimezoneFrame::returnFrameName() const
{
    return "Select Timezone";
}

void TimezoneFrame::init() {
  // Policy:
  //    * Read default timezone from settings.
  //    * Scan wifi spot.
  //    * Send http request to get geo ip.
  //    * Or wait for user to choose timezone on map.

  // Read timezone from settings.
  m_private->timezone_ = GetSettingsString(kTimezoneDefault);
  m_private->timezone_ = m_private->parseTimezoneAlias(m_private->timezone_);
  if (IsTimezoneInTab(m_private->timezone_)) {
    qDebug() << "timezone updated from settings";
    m_private->timezone_source_ = TimezoneSource::Conf;
  } else {
    const bool use_geoip = GetSettingsBool(kTimezoneUseGeoIp);
    const bool use_regdomain = GetSettingsBool(kTimezoneUseRegdomain);
    m_private->timezone_manager_->update(use_geoip, use_regdomain);

    // Use default timezone.
    m_private->timezone_ = kDefaultTimezone;
  }
  emit timezoneUpdated(m_private->timezone_);
}

void TimezoneFrame::updateTimezoneBasedOnLanguage(const QString& timezone) {
  // Check priority.
  if (m_private->timezone_source_ == TimezoneSource::NotSet ||
      m_private->timezone_source_ == TimezoneSource::Language) {
    if (IsTimezoneInTab(timezone)) {
      m_private->timezone_source_ = TimezoneSource::Language;
      m_private->timezone_ = timezone;
      emit timezoneUpdated(m_private->timezone_);
    }
  } else {
    qDebug() << "Ignores language default timezone:" << timezone;
  }
}

void TimezoneFrame::finished() {
  // Validate timezone before writing to settings file.
  if (!IsTimezoneInTab(m_private->timezone_)) {
    qWarning() << "Invalid timezone:" << m_private->timezone_;
    m_private->timezone_ = kDefaultTimezone;
  }

  WriteTimezone(timezone_);
  WriteIsLocalTime(true);
  WriteIsLocalTimeForce(true);

  QScopedPointer<QProcess> process(new QProcess);

  const QString localRtc = [=]() -> QString {
    const bool forceUse = GetSettingsBool(kTimezoneUseLocalTime);
    const bool DI_IS_LOCALE_TIME = GetSettingsBool("DI_IS_LOCAL_TIME");

    return forceUse || DI_IS_LOCALE_TIME ? "true" : "false";
  }();

  process->start("timedatectl", {"set-local-rtc", localRtc});
  process->waitForFinished();

  process->start("timedatectl", {"set-timezone", timezone_});
  process->waitForFinished();

  process->start("timedatectl", {"set-ntp", "false"});
  process->waitForFinished();

  process->start("timedatectl", {"set-time", m_systemDateFrame->timedate()});
  process->waitForFinished();
}

void TimezoneFrame::changeEvent(QEvent* event) {
  if (event->type() == QEvent::LanguageChange) {
    m_private->updateTs();

    if(m_private->m_mapOrListStackedLayout->currentWidget() == m_private->timezone_map_){
        m_private->comment_label_->setText(tr("Click your zone on the map"));
    }
    else {
        m_private->comment_label_->setText(tr("Select your timezone from the list"));
    }

    // Also update timezone.
    if (!m_private->timezone_.isEmpty()) {
      emit timezoneUpdated(m_private->timezone_);
    }
  } else {
    FrameInterface::changeEvent(event);
  }
}

void TimezoneFrame::showEvent(QShowEvent* event) {
  FrameInterface::showEvent(event);

  qApp->installEventFilter(this);

  // NOTE(xushaohua): Add a delay to wait for paint event of timezone map.
  QTimer::singleShot(0, [&]() {
      if(m_private->m_mapOrListStackedLayout->currentWidget() == m_private->timezone_map_){
          m_private->timezone_map_->showMark();
      }
      else {
          m_private->timezone_map_->hideMark();
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

void TimezoneFramePrivate::initConnections() {
  connect(timezone_manager_, &TimezoneManager::timezoneUpdated,
          this, &TimezoneFramePrivate::onTimezoneManagerUpdated);
  connect(timezone_map_, &TimezoneMap::timezoneUpdated,
          this, &TimezoneFramePrivate::onTimezoneMapUpdated);

  // Remark timezone on map when it is updated.
  connect(q_ptr, &TimezoneFrame::timezoneUpdated,
          timezone_map_, &TimezoneMap::setTimezone);
  connect(q_ptr, &TimezoneFrame::timezoneUpdated,
          m_selectTimeZoneFrame, &SelectTimeZoneFrame::onUpdateTimezoneList);

  connect(q_ptr, &TimezoneFrame::timezoneSet,
          timezone_map_, &TimezoneMap::setTimezoneData);

  connect(m_mapListButtonGroup, static_cast<void (DButtonBox::*)(QAbstractButton*)>(&DButtonBox::buttonClicked)
          , this, &TimezoneFramePrivate::onMapListButtonGroupToggled);

  connect(m_selectTimeZoneFrame, &SelectTimeZoneFrame::timezoneUpdated
          , this, &TimezoneFramePrivate::onSelectTimezoneUpdated);
}

void TimezoneFramePrivate::initUI() {
  title_label_ = new TitleLabel("");
  comment_label_ = new CommentLabel(tr("Click your zone on the map"));
  timezone_map_ = new TimezoneMap(q_ptr);

  m_mapListButtonGroup = new DButtonBox;
  m_timezoneMapButton = new DButtonBoxButton("");
  m_timezoneMapButton->setObjectName("timezoneMapButton");
  m_timezoneMapButton->setCheckable(true);
  m_timezoneMapButton->setMinimumWidth(60);
  m_timezoneMapButton->setMaximumHeight(36);
  m_timezoneListButton = new DButtonBoxButton("");
  m_timezoneListButton->setObjectName("timezoneListButton");
  m_timezoneListButton->setCheckable(true);
  m_timezoneListButton->setMinimumWidth(60);
  m_timezoneListButton->setMaximumHeight(36);

  m_buttonList.append(m_timezoneMapButton);
  m_buttonList.append(m_timezoneListButton);
  m_mapListButtonGroup->setButtonList(m_buttonList, true);
  m_timezoneMapButton->setChecked(true);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(m_mapListButtonGroup, 0, Qt::AlignCenter);
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

  if (!GetSettingsBool(kSkipAutoSyncTimePage)) {
    m_systemDateFrame = new SystemDateFrame;
    m_upLayout->addWidget(m_systemDateFrame, 0, Qt::AlignHCenter);
  }

  m_timezonePage = new QWidget;
  m_timezonePage->setContentsMargins(0, 0, 0, 0);
  m_timezonePage->setLayout(m_upLayout);

  centerLayout->addWidget(m_timezonePage);

  q_ptr->setContentsMargins(0, 0, 0, 0);

  updateTs();
}

bool TimezoneFramePrivate::validate() const
{
    return m_systemDateFrame->validateTimeDate();
}

QString TimezoneFramePrivate::parseTimezoneAlias(const QString& timezone) {
  // If |timezone| not in alias map, returns itself.
  return alias_map_.value(timezone, timezone);
}

void TimezoneFramePrivate::onTimezoneManagerUpdated(const QString& timezone) {
  // Check priority.
  if (timezone_source_ == TimezoneSource::NotSet ||
      timezone_source_ == TimezoneSource::Language ||
      timezone_source_ == TimezoneSource::Scan) {
    // Update timezone only if it is not set.
    timezone_source_ = TimezoneSource::Scan;
    timezone_ = parseTimezoneAlias(timezone);
    emit q_ptr->timezoneUpdated(timezone_);
  } else {
    qDebug() << "Ignore timezone value from timezone-manager:" << timezone;
  }
}

void TimezoneFramePrivate::onTimezoneMapUpdated(const QString& timezone) {
  timezone_source_ = TimezoneSource::User;
  // No need to convert timezone alias.
  timezone_ = timezone;
  m_selectTimeZoneFrame->onUpdateTimezoneList(timezone);
}

void TimezoneFramePrivate::onMapListButtonGroupToggled(QAbstractButton *button)
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

void TimezoneFramePrivate::onSelectTimezoneUpdated(const QString &timezone)
{
    timezone_source_ = TimezoneSource::User;
    timezone_ = timezone;
    emit q_ptr->timezoneSet(timezone);
}

}  // namespace installer

#include "timezone_frame.moc"
