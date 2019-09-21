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

#ifndef INSTALLER_SYSINFO_TIMEZONE_H
#define INSTALLER_SYSINFO_TIMEZONE_H

#include <QList>
#include <QHash>
#include <QMap>
#include <QPair>
#include <QStringList>

namespace installer {

struct ZoneInfo {
 public:
  QString country;
  QString timezone;

  // Coordinates of zone.
  double latitude;
  double longitude;

  // Distance to clicked point for comparison.
  double distance;
};
bool ZoneInfoDistanceComp(const ZoneInfo& a, const ZoneInfo& b);
QDebug& operator<<(QDebug& debug, const ZoneInfo& info);
typedef QList<ZoneInfo> ZoneInfoList;

// Read available timezone info in zone.tab file.
ZoneInfoList GetZoneInfoList();

// Read available Continent vs timeZone info in zone.tab file.
typedef QList<QPair<QString, QStringList>> ContinentZoneInfoList;
ContinentZoneInfoList GetContinentZoneInfo();

// Find ZoneInfo based on |country| or |timezone|.
// Returns -1 if not found.
int GetZoneInfoByCountry(const ZoneInfoList& list, const QString& country);
int GetZoneInfoByZone(const ZoneInfoList& list, const QString& timezone);

// Read current timezone in /etc/timezone file.
QString GetCurrentTimezone();

// Returns name of timezone, excluding continent name.
QString GetTimezoneName(const QString& timezone);

// Returns local name of <continent, timezone>, excluding continent name.
// |locale| is desired locale name.
QPair<QString, QString> GetLocalTimezoneName(const QString& timezone, const QString& locale);

// A map between old name of timezone and current name.
// e.g. Asia/Chongqing -> Asia/Shanghai
typedef QHash<QString, QString> TimezoneAliasMap;
TimezoneAliasMap GetTimezoneAliasMap();

// Returns true if |timezone| is in /usr/share/zoneinfo/zone.tab
bool IsTimezoneInTab(const QString& timezone);

// Validate |timezone|.
// Returns true if timezone is placed in /usr/share/zoneinfo.
bool IsValidTimezone(const QString& timezone);

struct TimezoneOffset {
  QString name;  // Offset name, like CST.
  long seconds;  // Offset seconds.
};

// Get |timezone| GMT offset.
TimezoneOffset GetTimezoneOffset(const QString& timezone);

}  // namespace installer

#endif  // INSTALLER_SYSINFO_TIMEZONE_H
