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

#include "ui/delegates/install_slide_frame_util.h"

#include <QDebug>

#include "base/file_util.h"
#include "service/settings_manager.h"
#include "service/settings_name.h"

namespace installer {

namespace {

const char kDefaultSlide[] = "default";

const char kSlideFolder[] = RESOURCES_DIR "/slide";

}  // namespace

QString GetSlideDir(const QString& locale) {
    QString s_dir;

    // 优先级从高到低
    // oem语言目录
    QDir oem_slide_dir(GetOemDir().absoluteFilePath("slide") + QDir::separator() + locale);
    qDebug() << "oem_slide_dir = " << oem_slide_dir.path();
    if (oem_slide_dir.exists()) {
        return oem_slide_dir.path();
    }

    // oem默认目录
    QDir oem_default_slide_dir(GetOemDir().absoluteFilePath("slide") + QDir::separator()  + QString(kDefaultSlide));
    qDebug() << "oem_default_slide_dir = " << oem_default_slide_dir.path();
    if (oem_default_slide_dir.exists()) {
        return oem_default_slide_dir.path();
    }

    // 平台语言目录
    QDir os_type_slide_dir(QString(kSlideFolder) + QDir::separator() + GetOSType() + QDir::separator() + locale);
    qDebug() << "os_type_slide_dir = " << os_type_slide_dir.path();
    if (os_type_slide_dir.exists()) {
        return os_type_slide_dir.path();
    }

    // 平台默认目录
    QDir os_type_default_slide_dir(QString(kSlideFolder) + QDir::separator() + GetOSType() + QDir::separator() + QString(kDefaultSlide));
    qDebug() << "os_type_default_slide_dir = " << os_type_slide_dir.path();
    if (os_type_default_slide_dir.exists()) {
        return os_type_default_slide_dir.path();
    }

    // 语言相关
    QDir slide_dir(QString(kSlideFolder) + QDir::separator() + locale);
    qDebug() << "slide_dir = " << slide_dir.path();
    if (slide_dir.exists()) {
        return slide_dir.path();

    }

    // 默认目录
    QDir default_slide_dir(QString(kSlideFolder) + QDir::separator() + QString(kDefaultSlide));
    qDebug() << "default_slide_dir = " << slide_dir.path();
    if (default_slide_dir.exists()) {
        return default_slide_dir.path();

    }

    return QString();
}

QStringList GetSlideFiles(const QString& locale) {
  QStringList slide_files;

  QDir slide_dir(GetSlideDir(locale));
  qDebug() << "slide_dir = " << slide_dir;

  // List all png files in slide folder.
  QString filepath;
  for (const QString& filename : slide_dir.entryList(QDir::Files)) {
    filepath = slide_dir.absoluteFilePath(filename);
    if (QFile::exists(filepath)) {
      slide_files.append(filepath);
    }
  }

  return slide_files;
}

}  // namespace installer
