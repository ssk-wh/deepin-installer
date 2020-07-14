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
  QDir installer_dir(QFile::exists(kSlideFolder) ? kSlideFolder : SOURCE_DIR "/resources/slide");
  Q_ASSERT(installer_dir.exists());
  QDir oem_slide_dir(GetOemDir().absoluteFilePath("slide"));

  QString os_type_slide = GetSettingsString(kInstallProgressOsTypeSlide);
  QDir os_type_dir(RESOURCES_DIR + QString(QDir::separator()) + os_type_slide);
  QFileInfo os_type_file(os_type_dir.absoluteFilePath(locale));

  qInfo() << "os_type_slide = " << os_type_slide;
  qInfo() << "os_type_file = " << os_type_file.absoluteFilePath();
  if (!os_type_slide.isEmpty() && os_type_file.isDir() && os_type_file.exists())
  {
      return os_type_file.absoluteFilePath();
  }

  // Check existence of folders one by one.
  QFileInfo file_info(oem_slide_dir.absoluteFilePath(locale));
  if (file_info.isDir() && file_info.exists()) {
    return file_info.absoluteFilePath();
  }

  file_info.setFile(installer_dir.absoluteFilePath(locale));
  if (file_info.isDir() && file_info.exists()) {
    return file_info.absoluteFilePath();
  }

  file_info.setFile(oem_slide_dir.absoluteFilePath(kDefaultSlide));
  if (file_info.isDir() && file_info.exists()) {
    return file_info.absoluteFilePath();
  }

  return installer_dir.absoluteFilePath(kDefaultSlide);
}

QStringList GetSlideFiles(const QString& locale) {
  QStringList slide_files;

  QDir slide_dir(GetSlideDir(locale));
  Q_ASSERT(slide_dir.exists());

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
