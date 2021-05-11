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

#include "sysinfo/timezone.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE  /* For tm_gmtoff and tm_zone */
#endif
#include <cmath>
#include <libintl.h>
#include <locale.h>
#include <time.h>
#include <QDebug>

#include "base/consts.h"
#include "base/file_util.h"

namespace installer {

namespace {

// Absolute path to zone.tab file.
const char kZoneTabFile[] = "/usr/share/zoneinfo/zone.tab";

// Absolute path to backward timezone file.
const char kTimezoneAliasFile[] = RESOURCES_DIR "/timezone_alias";

// Domain name for timezones.
const char kTimezoneDomain[] = "deepin-installer-timezones";

// continent: Antarctica
const char kAntarctica[] = "Antarctica";

// Parse latitude and longitude of the zone's principal location.
// See https://en.wikipedia.org/wiki/List_of_tz_database_time_zones.
// |pos| is in ISO 6709 sign-degrees-minutes-seconds format,
// either +-DDMM+-DDDMM or +-DDMMSS+-DDDMMSS.
// |digits| 2 for latitude, 3 for longitude.
double ConvertPos(const QString& pos, int digits) {
  if (pos.length() < 4 || digits > 9) {
    return 0.0;
  }

  const QString integer = pos.left(digits + 1);
  const QString fraction = pos.mid(digits + 1);
  const double t1 = integer.toDouble();
  const double t2 = fraction.toDouble();
  if (t1 > 0.0) {
    return t1 + t2 / pow(10.0, fraction.length());
  } else {
    return t1 - t2 / pow(10.0, fraction.length());
  }
}

}  // namespace

bool ZoneInfoDistanceComp(const ZoneInfo& a, const ZoneInfo& b) {
  return a.distance < b.distance;
}

QDebug& operator<<(QDebug& debug, const ZoneInfo& info) {
  debug << "ZoneInfo {"
        << "cc:" << info.country
        << "tz:" << info.timezone
        << "lat:" << info.latitude
        << "lng:" << info.longitude
        << "}";
  return debug;
}

ZoneInfoList GetZoneInfoList() {
  ZoneInfoList list;
  const QString content(ReadFile(kZoneTabFile));
  for (const QString& line : content.split('\n')) {
    if (!line.startsWith('#')) {
      const QStringList parts(line.split('\t'));
      // Parse latitude and longitude.
      if (parts.length() >= 3) {
        if (parts.at(2).contains(kAntarctica)){
            continue;
        }

        const QString coordinates = parts.at(1);
        int index = coordinates.indexOf('+', 3);
        if (index == -1) {
          index = coordinates.indexOf('-', 3);
        }
        Q_ASSERT(index > -1);
        const double latitude = ConvertPos(coordinates.left(index), 2);
        const double longitude = ConvertPos(coordinates.mid(index), 3);
        const ZoneInfo zone_info = {parts.at(0), parts.at(2),
                                    latitude, longitude, 0.0};
        list.append(zone_info);
      }
    }
  }
  return list;
}

int GetZoneInfoByCountry(const ZoneInfoList& list,
                         const QString& country) {
  int index = -1;
  for (const ZoneInfo& info : list) {
    index ++;
    if (info.country == country) {
      return index;
    }
  }
  return -1;
}

int GetZoneInfoByZone(const ZoneInfoList& list, const QString& timezone) {
  int index = -1;
  for (const ZoneInfo& info : list) {
    index ++;
    if (info.timezone == timezone) {
      return index;
    }
  }
  return -1;
}

QString GetCurrentTimezone() {
  const QString content(ReadFile("/etc/timezone"));
  return content.trimmed();
}

QString GetTimezoneName(const QString& timezone) {
  const int index = timezone.lastIndexOf('/');
  return (index > -1) ? timezone.mid(index + 1) : timezone;
}

QPair<QString, QString> GetLocalTimezoneName(const QString& timezone, const QString& locale) {
    QString local_name;

    if(locale == "en_US"){
        local_name = timezone;
    }
    else {
        QString localBak = setlocale(LC_ALL, nullptr);

        // Set locale first.
        QString locale_type = (locale + ".utf8");
        char * p_rest = setlocale(LC_ALL, locale_type.toLocal8Bit().constData());
        if (p_rest == nullptr) {
            qCritical() << "setlocale failed. locale:" << locale_type;
        }

        QString envBak = getenv("LANGUAGE");

        char *str = const_cast<char*>(QString("LANGUAGE=" + locale).toStdString().c_str());
        putenv(str);
        bindtextdomain(kTimezoneDomain, "/usr/share/locale/");

        textdomain(kTimezoneDomain);

        local_name =
            dgettext(kTimezoneDomain, timezone.toLocal8Bit().constData());

        // Reset locale.
        if (!localBak.isEmpty()) {
            setlocale(LC_ALL, localBak.toLocal8Bit().constData());
        }

        // Reset env
        if (!envBak.isEmpty()) {
            putenv(QString("LANGUAGE=" + envBak).toLocal8Bit().data());
        }
    }

    int index = local_name.indexOf('/');
    if (index < 0){
        // Some translations of locale name contains non-standard char.
        index = local_name.indexOf(0x2215); // ∕ = 0x2215(unicode)
    }

    if (index > 0){
        return QPair<QString, QString>(local_name.left(index), local_name.mid(index + 1));
    }
    else{
        qWarning() << "invalid local timezone:" << local_name;
        return QPair<QString, QString>("", local_name);
    }
}

TimezoneAliasMap GetTimezoneAliasMap() {
  TimezoneAliasMap map;

  const QString content = ReadFile(QFile::exists(kTimezoneAliasFile) ? kTimezoneAliasFile : SOURCE_DIR "/resources/timezone_alias");
  for (const QString& line : content.split('\n')) {
    if (!line.isEmpty()) {
      const QStringList parts = line.split(':');
      Q_ASSERT(parts.length() == 2);
      if (parts.length() == 2) {
        map.insert(parts.at(0), parts.at(1));
      }
    }
  }

  return map;
}

bool IsTimezoneInTab(const QString& timezone) {
  const ZoneInfoList info_list = GetZoneInfoList();
  const int index = GetZoneInfoByZone(info_list, timezone);
  return (index > -1);
}

bool IsValidTimezone(const QString& timezone) {
  // Ignores empty timezone.
  if (timezone.isEmpty()) {
    return false;
  }

  // If |filepath| is a file or a symbolic link to file, it is a valid timezone.
  const QString filepath(QString("/usr/share/zoneinfo/") + timezone);
  return QFile::exists(filepath);
}

TimezoneOffset GetTimezoneOffset(const QString& timezone) {
  const char* kTzEnv = "TZ";
  const char* old_tz = getenv(kTzEnv);
  setenv(kTzEnv, timezone.toStdString().c_str(), 1);
  struct tm tm;
  const time_t curr_time = time(NULL);

  // Call tzset() before localtime_r(). Set tzset(3).
  tzset();
  (void) localtime_r(&curr_time, &tm);

  // Reset timezone.
  if (old_tz) {
    setenv(kTzEnv, old_tz, 1);
  } else {
    unsetenv(kTzEnv);
  }

  const TimezoneOffset offset = {tm.tm_zone, tm.tm_gmtoff};
  return offset;
}

ContinentZoneInfoList GetContinentZoneInfo()
{
    QMap<QString, QStringList> infoMap;
    const QString& content(ReadFile(kZoneTabFile));

    QStringList strList = content.split('\n');
    for (const QString& line : strList) {
        if (!line.startsWith('#')) {
            const QStringList parts(line.split('\t'));
            if(parts.length() < 3){
                continue;
            }

            // Parse Continent/timeZone.
            const QString& continentTimeZone = parts.at(2);
            // Miller cylindrical map projection does not support Antarctica timezone.
            if (continentTimeZone.contains(kAntarctica)){
                continue;
            }

            int index = continentTimeZone.indexOf('/');
            if(index < 0){
                qWarning() << "invalid Continent/timeZone: " << continentTimeZone;
                continue;
            }

            const QString& continent = continentTimeZone.left(index);
            const QString& timeZone = continentTimeZone.mid(index + 1);
            infoMap[continent] << timeZone;
        }
    }

    ContinentZoneInfoList infoList;
    for (auto it = infoMap.begin(); it != infoMap.end(); ++it) {
        infoList << QPair<QString, QStringList>(it.key(), it.value());
    }
    return infoList;
}

}  // namespace installer
