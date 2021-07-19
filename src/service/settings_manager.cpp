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

#include "service/settings_manager.h"

#include <fcntl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QHash>
#include <QSettings>
#include <random>
#include <DSysInfo>
#include <QDBusInterface>
#include <QRegExp>

#include "base/consts.h"
#include "service/settings_name.h"
#include "partman/structs.h"
#include "base/command.h"

DCORE_USE_NAMESPACE

namespace installer {

namespace {

// Absolute path to oem dir.

const char kLocaleKey[] = "DI_LOCALE";
const char kComponentUninstallPackages[] = "DI_COMPONENT_UNINSTALL";

// Absolute path to installer config file.
#ifdef QT_DEBUG
const char kInstallerConfigFile[] = "/tmp/deepin-installer.conf";
#else
const char kInstallerConfigFile[] = "/etc/deepin-installer.conf";
#endif // QT_DEBUG

// Absolute path to default installer settings
const char kDefaultSettingsFile[] = RESOURCES_DIR "/default_settings.ini";
const char kDefaultWallpaperFile[] = RESOURCES_DIR "/default_wallpaper.jpg";
// File name of installer wallpaper.
const char kOemWallpaperFilename[] = "installer-background.jpg";

const char kComponentDefaultFile[] = "/packages_default.json";
const char kComponentExtraFile[] = "/packages_choice.json";
const char kComponentSortFile[] = "/packages_sort.json";
const char kLanguageSortFile[] = "/language_sort.json";

// File name of auto partition script.
const char kAutoPartFile[] = "auto_part.sh";

// Absolute path to oem folder.
const char kDebugOemDir[] = "/tmp/oem";
const char kUbuntuOemDir[] = "/cdrom/oem";
const char kDeepinOemDir[] = "/lib/live/mount/medium/oem";
const char kDeepinOemCandidateDir[] = "/usr/lib/live/mount/medium/oem";
const char kDeepinOemCandidateDir2[] = "/run/live/medium/oem";

// Filename of oem settings
const char kOemSettingsFilename[] = "settings.ini";

void AppendToConfigFile(const QString& key, const QVariant& value) {
  QSettings settings(kInstallerConfigFile, QSettings::IniFormat);
  settings.setValue(key, value);
}

QStringList ListAvatarFiles(const QString& dir_name) {
  QStringList result;
  QDir dir(dir_name);
  if (!dir.exists()) {
    return result;
  }

  const QStringList name_filters = { "*.png", "*.jpg" };
  const QFileInfoList info_list =
      dir.entryInfoList(name_filters, QDir::NoDotAndDotDot | QDir::Files);
  for (const QFileInfo& info : info_list) {
    // Ignores "default.png" and "guest.png".
    if (info.size() > 0 &&
        (!info.fileName().startsWith("default")) &&
        (!info.fileName().startsWith("guest"))) {
      result.append(info.absoluteFilePath());
    }
  }

  return result;
}

}  // namespace

QDir GetOemDir() {
    QString g_oem_dir;
    if (QDir(kDebugOemDir).exists()) {
        g_oem_dir = kDebugOemDir;
    } else if (QDir(kUbuntuOemDir).exists()) {
        g_oem_dir = kUbuntuOemDir;
    } else if (QDir(kDeepinOemDir).exists()) {
        g_oem_dir = kDeepinOemDir;
    } else if (QDir(kDeepinOemCandidateDir).exists()) {
        g_oem_dir = kDeepinOemCandidateDir;
    } else {
        g_oem_dir = kDeepinOemCandidateDir2;
    }

    qDebug() << "OEM_DIR=" << g_oem_dir;
    return QDir(g_oem_dir);
}

QString GetOemLicenseDir() {
    QString oem_license_dir = GetOemDir().path() + "/licenses";
    return  QFileInfo(oem_license_dir).exists() ? oem_license_dir : RESOURCES_DIR"/licenses";
}

QString GetCurrentPlatform() {
    QMap<QString, QString> BUILD_ARCH_MAP{ { "x86_64",  "x86" },
                                           { "i386", "x86" },
                                           { "i686", "x86" },
                                           { "amd64", "x86" },
                                           { "x86", "x86" },
                                           { "sw_64",   "sw" },
                                           { "mips64", "loongson" },
                                           { "loongarch64", "loongarch64" },
                                           { "aarch64", "arm" } };

    return BUILD_ARCH_MAP[PLATFORM_BUILD_ARCH];
}


QString GetOSType()
{
    QSettings settings("/etc/os-version", QSettings::IniFormat);
    settings.beginGroup("Version");

#ifdef QT_DEBUG
    QString type = "Professional";
#else
    QString type = settings.value("EditionName", "Community").toString();
#endif // QT_DEBUG

    return type;
}

OSType GetCurrentType() {
    const QString& type = GetOSType();

    return QMap<QString, OSType>{
        { "Community", OSType::Community },
        { "Professional", OSType::Professional },
        { "Server", OSType::Server },
        { "Personal", OSType::Personal },
        { "Device", OSType::Device }
    }[type];
}

QString GetDiskPolicyArgs() {
    QFile cmd_file("/proc/cmdline");
    cmd_file.open(QIODevice::ReadOnly);
    if (!cmd_file.isOpen()) {
        qWarning() << "GetDiskPolicyArgs: Failed to open file. /proc/cmdline";
        return QString();
    }

    QRegExp exp("full-disk-policy=[1-9]");
    QString info = cmd_file.readAll();
    if (exp.indexIn(info) == -1) {
       return QString();
    }
    QStringList args = exp.capturedTexts().join("").split("=");
    if (args.size() != 2) {
        qWarning() << QString("GetDiskPolicyArgs: Unknown command parameters:%1. /proc/cmdline")
                      .arg(info);
        return QString();
    }

    qInfo() << QString("GetDiskPolicyArgs:%1.  /proc/cmdline")
                  .arg(args.at(1));
    return args.at(1);
}

bool isPexInstall() {
    QFile cmd_file("/proc/cmdline");
    cmd_file.open(QIODevice::ReadOnly);
    if (!cmd_file.isOpen()) {
        qCritical() << "isPexInstall: Failed to open file. /proc/cmdline";
        return true;   // 如果没有读到文件，无法做出是否为pxe的判断， 则始终不加载网络模块，防止产生网络问题
    }
    QString info = cmd_file.readAll();
    if (!info.contains("nfsroot")) {
        return false;
    }

    qInfo() << "isPexInstall: /proc/cmdline <" << info << ">";
    return true;
}

bool GetSettingsBool(const QString& key) {
    if (SettingCustom::Instance()->hasSetting(key)) {
        return SettingCustom::Instance()->getSettingsBool(key);
    }


  const QVariant value = GetSettingsValue(key);
  qDebug() << kInstallerConfigFile <<  "  key = " << key << "  =  " << value;
  if (value.isValid()) {
    return value.toBool();
  }

  qCritical() << "GetSettingsBool() failed with key:" << key;
  return false;
}

int GetSettingsInt(const QString& key) {
  const QVariant value = GetSettingsValue(key);
  if (value.isValid()) {
    return value.toInt();
  }

  qCritical() << "GetSettingsInt() failed with key:" << key;
  return 0;
}

QString GetSettingsString(const QString& key) {
  const QVariant value = GetSettingsValue(key);
  if (value.isValid()) {
    return value.toString();
  }

  qCritical() << "GetSettingsString() failed with key:" << key;
  return QString();
}

QStringList GetSettingsStringList(const QString& key) {
  const QVariant value = GetSettingsValue(key);
  if (!value.isValid()) {
    qCritical() << "GetSettingsStringList() failed with key:" << key;
    return QStringList();
  }

  return value.toString().split(';', QString::SkipEmptyParts);
}

QVariant GetSettingsValue(const QString& key) {
  QSettings settings(kInstallerConfigFile, QSettings::IniFormat);
  if (settings.contains(key)) {
    return settings.value(key);
  }

  // Read default settings
  QSettings default_settings(kDefaultSettingsFile, QSettings::IniFormat);
  if (!default_settings.contains(key)) {
    qWarning() << "getSettingsValue() Invalid key:" << key;
  }
  return default_settings.value(key);
}

QString GetAutoPartFile() {
  QDir oem_dir = GetOemDir();
  QDir builtin_dir(BUILTIN_HOOKS_DIR);

  // First check existence of architecture specific file.
  const QStringList script_files = {
      oem_dir.absoluteFilePath(kAutoPartFile),
      builtin_dir.absoluteFilePath(kAutoPartFile),
  };
  for (const QString filepath : script_files) {
    if (QFile::exists(filepath)) {
      return filepath;
    }
  }

  qCritical() << "GetAutoPartFile() not partition script found!";
  return QString();
}

QStringList GetAvatars() {
  // First, check oem/ dir.
  const QString oem_avatar(GetOemDir().absoluteFilePath("avatar"));
  QStringList avatars = ListAvatarFiles(oem_avatar);
  if (!avatars.isEmpty()) {
    return avatars;
  }

  // Then, check dde-account-faces dir.
  return ListAvatarFiles(GetSettingsString(kSystemInfoDdeAvatarDir));
}

QString GetDefaultAvatar() {
  const QString default_avatar(GetSettingsString(kSystemInfoDefaultAvator));
  if (!default_avatar.isEmpty() && QFile::exists(default_avatar)) {
    // Returns default avatar
    return default_avatar;
  }

  // Pick a random avatar.
  const QStringList avatars = GetAvatars();
  if (avatars.isEmpty()) {
    return "";
  }

  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_int_distribution<int> uniform_dist(0, avatars.size() - 1);

  return avatars.at(uniform_dist(e1));
}

QString GetOemHooksDir() {
  return GetOemDir().absoluteFilePath("hooks");
}

QString GetOemCheckHooksDir() {
    return GetOemDir().absoluteFilePath("check_hooks");
}

QString GetReservedUsernameFile() {
  const QString oem_file = GetOemDir().absoluteFilePath("reserved_usernames");
  if (QFile::exists(oem_file)) {
    return oem_file;
  }

  // Returns default list.
  return RESOURCES_DIR "/reserved_usernames";
}

QString GetVendorLogo() {
  QString oem_file = DSysInfo::distributionOrgLogo(DSysInfo::Distribution, DSysInfo::Normal);
  qDebug() << "oem_file = " << oem_file;
  if (QFile::exists(oem_file)) {
    return oem_file;
  }

  oem_file = GetOemDir().absoluteFilePath("vendor.png");
  qDebug() << "oem_file = " << oem_file;
  if (QFile::exists(oem_file)) {
    return oem_file;
  }

  oem_file = GetOemDir().absoluteFilePath("vendor.svg");
  qDebug() << "oem_file = " << oem_file;
  if (QFile::exists(oem_file)) {
    return oem_file;
  }

  // Returns builtin logo.
  return ":/images/logo.svg";
}

QString GetWindowBackground() {
  const QString oem_file = GetOemDir().absoluteFilePath(kOemWallpaperFilename);
  if (QFile::exists(oem_file)) {
    return oem_file;
  }

  return kDefaultWallpaperFile;
}

QByteArray GetFullDiskInstallPolicy() {
    const QString& type = GetOSType();
    const QString& policy_args = GetDiskPolicyArgs();

    const QStringList list {
        QString("%1/disk_policy/full_disk_policy_%2.json").arg(GetOemDir().path())
                .arg(policy_args.toLower()),
        QString("%1/full_disk_policy_%2.json").arg(GetOemDir().path()).arg(type.toLower()),
        QString("%1/full_disk_policy.json").arg(GetOemDir().path()),
        QString(RESOURCES_DIR "/disk_policy/full_disk_policy_%2.json").arg(policy_args.toLower()),
        QString(RESOURCES_DIR "/override/full_disk_policy_%1.json").arg(type.toLower()),
        QString(RESOURCES_DIR "/full_disk_policy_%1.json").arg(type.toLower()),
        QString(RESOURCES_DIR "/full_disk_policy.json")
    };

    for (const QString& path : list) {
        if (!QFile::exists(path)) continue;

        QFile file(path);
        if (file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            qInfo() << "read file " << file.fileName();
            return file.readAll();
        }
    }

    return "";
}

QString GetSelectedInstallType()
{
    QString type = GetSettingsString(kSelectedInstallType);
    if (!type.isEmpty()){
        return type;
    }

    return GetSettingsString(kSelectComponentDefaultInstallType);
}

QString GetComponentDefault() {
    const QStringList paths{ "/var/run/live/medium/live/", RESOURCES_DIR, SOURCE_DIR "/resources" };

    for (const QString& path : paths) {
        QFile file(path + kComponentDefaultFile);
        if (!file.exists()) {
            continue;
        }
        if (file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            qInfo() << "read file " << file.fileName();
            return file.readAll();
        }
    }

    qCritical() << "component default file not found:" << kComponentDefaultFile;
    return "";
}

QString GetComponentExtra() {
    const QStringList paths{ "/var/run/live/medium/live/", RESOURCES_DIR, SOURCE_DIR "/resources"};

    for (const QString& path : paths) {
        QFile file(path + kComponentExtraFile);
        if (!file.exists()) {
            continue;
        }
        if (file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            qInfo() << "read file " << file.fileName();
            return file.readAll();
        }
    }

    qCritical() << "component extra file not found:" << kComponentExtraFile;
    return "";
}

QString GetComponentSort(){
    const QStringList paths{ "/var/run/live/medium/live/", RESOURCES_DIR, SOURCE_DIR "/resources" };

    for (const QString& path : paths) {
        QFile file(path + kComponentSortFile);
        if (!file.exists()) {
            continue;
        }
        if (file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            qInfo() << "read file " << file.fileName();
            return file.readAll();
        }
    }

    qCritical() << "component sort file not found:" << kComponentSortFile;
    return "";
}

QString GetLanguageSort()
{
    const QStringList paths{ "/var/run/live/medium/live/", RESOURCES_DIR, SOURCE_DIR "/resources" };

    for (const QString& path : paths) {
        QFile file(path + kLanguageSortFile);
        if (!file.exists()) {
            continue;
        }
        if (file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            qInfo() << "read file " << file.fileName();
            return file.readAll();
        }
    }

    qCritical() << "language sort file not found:" << kLanguageSortFile;
    return "";
}

bool AppendConfigFile(const QString& conf_file) {
  if (!QFile::exists(conf_file)) {
    qCritical() << "conf_file not found:" << conf_file;
    return false;
  }

  QSettings target_settings(kInstallerConfigFile, QSettings::IniFormat);
  QSettings new_settings(conf_file, QSettings::IniFormat);

  for (const QString& key : new_settings.allKeys()) {
    const QVariant value = new_settings.value(key);
    target_settings.setValue(key, value);
  }

  return true;
}

bool DeleteConfigFile() {
  QFile file(kInstallerConfigFile);
  if (file.exists()) {
    if (!file.remove()) {
      qCritical() << "Failed to delete installer config file!";
      return false;
    }
  }
  return true;
}

void WriteUEFI(bool is_efi) {
  AppendToConfigFile("DI_UEFI", is_efi);
}

void WriteEnableNvidiaDriver(bool is_enable_nvidia_driver) {
  AppendToConfigFile("DI_ENABLE_NVIDIA_DRIVER", is_enable_nvidia_driver);
}

// 写入是否使用过自动挂载功能
void WriteIsAutoMount(bool is_auto_mount) {
  AppendToConfigFile("DI_AUTO_MOUNT", is_auto_mount);
}

QString ReadLocale() {
  QSettings settings(kInstallerConfigFile, QSettings::IniFormat);
  QString locale;

  // Get user-selected locale.
  locale = settings.value(kLocaleKey).toString();
  if (locale.isEmpty()) {
    // Get default locale in settings.ini.
    locale = settings.value(kSelectLanguageDefaultLocale).toString();

    if (locale.isEmpty()) {
      // Get fallback locale.
      locale = kDefaultLang;
    }
  }

  // Remove codec name from locale.
  const int dot_index = locale.indexOf('.');
  return (dot_index == -1) ? locale : locale.left(dot_index);
}

QStringList ReadComponentUninstallPackages()
{
    QSettings settings(kInstallerConfigFile, QSettings::IniFormat);

    const QVariant value = settings.value(kComponentUninstallPackages);
    if (value.isValid()) {
      return value.toString().split(" ");
    }

    qCritical() << "ReadComponentUninstallPackages() failed with key:" << kComponentUninstallPackages;
    return QStringList();
}

QString GetUIDefaultFont() {
  // TODO: maybe make this a configurable value in the future? since other
  // editions may have different fonts installed.
  return "Noto Sans CJK SC Regular";
}

void WriteAvatar(const QString& avatar) {
  AppendToConfigFile("DI_AVATAR", avatar);
}

void WriteHostname(const QString& hostname) {
  AppendToConfigFile("DI_HOSTNAME", hostname);
}

void WriteKeyboard(const QString& model,
                   const QString& layout,
                   const QString& variant) {
  QSettings settings(kInstallerConfigFile, QSettings::IniFormat);
  settings.setValue("DI_KEYBOARD_MODEL", model);
  settings.setValue("DI_LAYOUT", layout);
  settings.setValue("DI_LAYOUT_VARIANT", variant);
}

void WriteLocale(const QString& locale) {
  AppendToConfigFile("DI_LOCALE", locale);
}

void WritePassword(const QString& password) {
  const QString encoded_password = password.toUtf8().toBase64();
  AppendToConfigFile("DI_PASSWORD", encoded_password);
}

void WriteRootPassword(const QString& password) {
  AppendToConfigFile("DI_ROOTPASSWORD", QString(password.toUtf8().toBase64()));
}

void WriteTimezone(const QString& timezone) {
  QSettings settings(kInstallerConfigFile, QSettings::IniFormat);
  settings.setValue("DI_TIMEZONE", timezone);
}

void WriteTimedate(const QString &timeDate)
{
    QSettings settings(kInstallerConfigFile, QSettings::IniFormat);
    settings.setValue("DI_TIMEDATE", timeDate);
}

void WriteUsername(const QString& username) {
  AppendToConfigFile("DI_USERNAME", username);

  // add user info log
  if (!username.isEmpty()) {
      if (GetSettingsBool(kSystemIsZF)) qInfo() << "create user: " << username;
  }
}

void WritePartitionInfo(const QString& root_disk,
                        const QString& root_partition,
                        const QString& boot_partition,
                        const QString& mount_points) {
  qDebug() << "WritePartitionInfo()"
           << " root_disk:" << root_disk
           << ", root_partition:" << root_partition
           << ", boot_partition:" << boot_partition
           << ", mount_points:" << mount_points;
  QSettings settings(kInstallerConfigFile, QSettings::IniFormat);
  settings.setValue("DI_ROOT_DISK", root_disk);
  settings.setValue("DI_ROOT_PARTITION", root_partition);
  settings.setValue("DI_BOOTLOADER", boot_partition);
  settings.setValue("DI_MOUNTPOINTS", mount_points);
}

void WriteRequiringSwapFile(bool is_required) {
  AppendToConfigFile("DI_SWAP_FILE_REQUIRED", is_required);
}

void WriteUserExperience(bool enable) {
    AppendToConfigFile("DI_USER_EXPERIENCE", enable);

    if (enable) {
        qInfo() << QString("The user agrees to '%1'")\
                   .arg("The user experience The program license agreement");
    } else {
        qInfo() << QString("User cancelled '%1'")\
                   .arg("The user experience The program license agreement");
    }
}

void WriteUserLicenseIsUse(bool enable)
{
    AppendToConfigFile("DI_USER_USE_LICENSE", enable);

    if (enable) {
        qInfo() << QString("The user agrees to '%1'")\
                   .arg("The user license");
    } else {
        qInfo() << QString("User cancelled '%1'")\
                   .arg("The user license");
    }
}

void AddConfigFile() {
    QSettings target_settings(kInstallerConfigFile, QSettings::IniFormat);

    QStringList settingsList;

    // Read default settings
    settingsList << kDefaultSettingsFile;

    // Read override settings
    QString prefix;
    switch (GetCurrentType()) {
        case OSType::Community: prefix = "community"; break;
        case OSType::Professional: prefix = "professional"; break;
        case OSType::Server: prefix = "server"; break;
        case OSType::Personal: prefix = "personal"; break;
        case OSType::Device:prefix = "device";break;
        default: prefix = "community";break;
    }

#ifdef QT_DEBUG
    const QString& arch = GetCurrentPlatform();
    QString        override_settings(RESOURCES_DIR +
                              QString("/platform_%1/%2.override").arg(arch).arg(prefix));
#else
    QString override_settings(RESOURCES_DIR +
                              QString("/override/%1.override").arg(prefix));
#endif  // QT_DEBUG

    qInfo() << "override_settings = " << override_settings;
    if (QFile::exists(override_settings)) {
        settingsList << override_settings;
    }

    // Read oem settings
    const QString oem_file = GetOemDir().absoluteFilePath(kOemSettingsFilename);
    if (QFile::exists(oem_file)) {
        settingsList << oem_file;
    }

    const QString policy_file = QString("%1/disk_policy/settings_%2.ini")
            .arg(GetOemDir().path(), GetDiskPolicyArgs());
    if (QFile::exists(policy_file)) {
        settingsList << policy_file;
    }

    for (const QString& file : settingsList) {
        QSettings settings(file, QSettings::IniFormat);
        for (const QString& key : settings.allKeys()) {
            target_settings.setValue(key, settings.value(key));
        }
    }
}

void WriteFullDiskEncryptPassword(const QString &password)
{
    AppendToConfigFile("DI_CRYPT_PASSWD", password);
}

void BeforeInstallHook()
{
    const QString subPath = "/hooks/before_install/";
    QStringList jobList;
    const QString oemdirpath = installer::GetOemDir().path();
    QStringList nameFilters {"*.job"};

    QString objPath = "/tmp/installer/";
    QDir objDir(objPath);
    if (!objDir.exists()) {
        objDir.mkpath(objPath);
    }

    QString path = oemdirpath + subPath;
    qDebug() << "before installer job path:" << path;
    QDir dir(path);
    QStringList const files = dir.entryList(nameFilters, QDir::Files);
    qDebug() << "before installer job list:" << files;
    foreach (const QString& f, files) {
        const QString fileName = path + f;
        QFile file(objPath + f);
        if (file.exists()) {
            file.remove();
        }
        if (!QFile::copy(fileName, objPath + f)) {
            qCritical() << "copy " << fileName << " to " << objPath << " failed!";
        }

    }

    //TODO
    //Splicing according to the actual situation
    const QString hookManager = "/usr/share/deepin-installer/hooks/hook_manager.sh";
    QDir objdir(objPath);
    QStringList objfiles = objdir.entryList(nameFilters, QDir::Files);
    foreach (const QString job, objfiles) {
        if (!SpawnCmd(hookManager, {objPath + job})) {
            qCritical() << "hook_manager.sh " << job << " failed";
            return;
        }
    }
}

void WritePasswordStrong(bool strongPassword) {
    AppendToConfigFile("DI_STRONG_PASSWORD", strongPassword);
}

void WriteDisplayPort(const QString &display) {
    AppendToConfigFile("DI_DISPLAY_PORT", display);
}

void WriteGrubPassword(const QString &password) {
    AppendToConfigFile("DI_GRUB_PASSWORD", password);
}

void WriteFullDiskDeivce(const QString &deviceName)
{
    AppendToConfigFile("DI_FULLDISK_DEVICE", deviceName);
}

void WriteSwapPartitionSize(const uint size) {
    AppendToConfigFile("DI_SWAP_SIZE", size);
}

void WriteSelectedInstallType(const QString &installType)
{
    AppendToConfigFile(kSelectedInstallType, installType);
}

void WriteComponentPackages(const QString &packages)
{
    AppendToConfigFile("DI_COMPONENT_PACKAGES", packages);

    // add package info log
    qInfo() << "install packages list: " << packages;
}

void WriteComponentUninstallPackages(const QString& packages)
{
    AppendToConfigFile("DI_COMPONENT_UNINSTALL", packages);

    // add package info log
    qInfo() << "uninstall packages list: " <<  packages;
}

void WriteComponentLanguage(const QString& packages) {
    AppendToConfigFile("DI_COMPONENT_LANGUAGE", packages);

    // add package info log
    qInfo() << "language packages list: " << packages;
}

void WriteIsMinimalGraphicsSystem(const bool isMinimalGraphicsSystem)
{
    AppendToConfigFile("DI_IS_MINIMAL_GRAPHICS_SYSTEM", isMinimalGraphicsSystem);
}

void WriteIsMinimalCharacterSystem(const bool isMinimalCharacterSystem)
{
    AppendToConfigFile("DI_IS_MINIMAL_CHARACTER_SYSTEM", isMinimalCharacterSystem);
}

void WriteIsEnableNTP(bool isEnableNTP)
{
    AppendToConfigFile("DI_IS_ENABLE_NTP", isEnableNTP);
}

void WriteFullDiskResolution(const FinalFullDiskResolution& resolution)
{
    const char kFullDiskResolutionDeviceSeparator[] = ";";
    QString key { "" };
    QStringList valueList;
    for (int i = 0; i < resolution.option_list.length(); i++) {
        valueList << resolution.option_list.at(i).device;
    }

    qDebug() << "DI_FULLDISK_MULTIDISK_DEVICE old=" << GetSettingsStringList("DI_FULLDISK_MULTIDISK_DEVICE");
    if (valueList.isEmpty()) {
        // 无人值守，重新设置一遍该值，防止在无人值守的时候没有配置DI_FULLDISK_MULTIDISK_DEVICE选项，导致系统安装报错
        QString devices = GetSettingsString("DI_FULLDISK_MULTIDISK_DEVICE");
        AppendToConfigFile("DI_FULLDISK_MULTIDISK_DEVICE", devices);

    } else {
        // 非无人值守，通过界面设置设备
        AppendToConfigFile("DI_FULLDISK_MULTIDISK_DEVICE", valueList.join(kFullDiskResolutionDeviceSeparator));
    }
    qDebug() << "DI_FULLDISK_MULTIDISK_DEVICE new=" << GetSettingsStringList("DI_FULLDISK_MULTIDISK_DEVICE");

    QString labelKey { "" };
    QStringList labelValueList;
    for (int i = 0; i < resolution.option_list.length(); i++) {
        const FinalFullDiskOption& op = resolution.option_list[i];
        key = QString("DI_FULLDISK_MULTIDISK_POLICY_%1").arg(i);
        labelKey = QString("DI_FULLDISK_MULTIDISK_LABEL_%1").arg(i);
        valueList.clear();
        labelValueList.clear();

        if (i > 0 && op.encrypt) {
            valueList << QString("luks_crypt%1:crypto_luks::100%").arg(i);
            labelValueList << QString("luks_crypt%1").arg(i);
        }
        for (int j = 0; j < op.policy_list.length(); j++) {
            const FinalFullDiskPolicy& policy = op.policy_list[j];
            valueList << QString("%1:%2:%3:%4")
                    .arg(policy.mountPoint)
                    .arg(policy.filesystem)
                    .arg(policy.offset/kMebiByte)
                    .arg(policy.size/kMebiByte);
            labelValueList << policy.label;
            if (op.encrypt && 0 == i && policy.mountPoint == "/boot") {
                valueList << QString("luks_crypt%1:crypto_luks::100%").arg(i);
                labelValueList << QString("luks_crypt%1").arg(i);
            }
        }
        AppendToConfigFile(key, valueList.join(kFullDiskResolutionDeviceSeparator));
        AppendToConfigFile(labelKey, labelValueList.join(kFullDiskResolutionDeviceSeparator));
    }
    WriteFullDiskMode(true);
}

void WriteFullDiskMode(bool value)
{
    AppendToConfigFile("DI_FULLDISK_MODE", value ? "true" : "false");
}

DiskPartitionSetting::DiskPartitionSetting()
    :  swap_file_required(false)
     , uefi_required(false)
{
}

void WriteDiskPartitionSetting(const DiskPartitionSetting& setting)
{
    WritePartitionInfo(setting.root_disk,
                       setting.root_partition,
                       setting.boot_partition,
                       setting.mount_points);
    WriteRequiringSwapFile(setting.swap_file_required);
    WriteUEFI(setting.uefi_required);
}

void WriteInstallSuccessed(bool successed)
{
    AppendToConfigFile("DI_INSTALL_SUCCESSED", successed);
}

void WriteInstallStartTime(const QString& starttime)
{
    AppendToConfigFile("DI_INSTALL_STARTTIME", starttime);
}

void WriteInstallEndTime(const QString& endtime)
{
    AppendToConfigFile("DI_INSTALL_ENDTIME", endtime);
}

void CustomSetSettingsBool(const char* key, bool boolvalue)
{
    AppendToConfigFile(key, boolvalue);//AppendToConfigFile(kSystemInfoSetupAfterReboot, after);
}

SettingCustom *SettingCustom::Instance()
{
    static SettingCustom settingCustom;
    return &settingCustom;
}

void SettingCustom::setSettingsBool(const QString &key, const bool value)
{
    SettingCustom::Instance()->setProperty(key.toLatin1(), QVariant(value));
}

bool SettingCustom::getSettingsBool(const QString &key)
{
    const QVariant value = SettingCustom::Instance()->property(key.toLatin1());
    if (value.isValid()) {
      return value.toBool();
    }

    return false;
}

bool SettingCustom::hasSetting(const QString &key)
{
    const QVariant value = SettingCustom::Instance()->property(key.toLatin1());
    if (value.isValid()) {
      return true;
    }

    return false;
}

void WriteNecuresCliInstallMode(bool mode)
{
    AppendToConfigFile("DI_NECURESCLIINSTALL_MODE", mode);
}

bool isNotebook()
{
    if (GetSettingsBool(kEnablePcTypeCheck)) {
        QDBusInterface upowerInterface("org.freedesktop.UPower",
                                            "/org/freedesktop/UPower",
                                            "org.freedesktop.UPower",
                                            QDBusConnection::systemBus());
        QVariant  defaultDevice(upowerInterface.property("LidIsPresent"));
        if (defaultDevice.type() != QVariant::Type::Bool) {
            qCritical() << "failed get pc type.  " << defaultDevice.type();
            return false;
        }
        qDebug() << "defaultDevice.toBool() = " << defaultDevice.toBool();
        return defaultDevice.toBool();
    } else {
        return false;  // 不开启pc类型的检查，则不处理是否为笔记本的判断
    }
}

void WriteIfDoRecovery(const bool value)
{
    AppendToConfigFile("DI_IF_DO_RECOVERY", value ? "true" : "false");
}

void WriteIsInitRecovery(bool value) {
    AppendToConfigFile("DI_IS_INIT_RECOVERY", value);
}

void WriteSystemDataMountPoint(const QString &value)
{
    AppendToConfigFile("DI_DATA_MOUNT_POINT", value);
}

// 记录设置的密码等级
void WritePasswdLevel(const int &level)
{
    AppendToConfigFile("DI_PASSWDLEVEL", level);
}

void ScanNetDevice()
{
    QDir work_dir = QDir("/sys/bus/pci/devices");

    for (QString device : work_dir.entryList()) {
        // 过滤虚拟设备
        if ((device == ".") || (device == "..")) {
            continue;
        }

        // 读取设备文件
        QFile file(QString("%1/%2/uevent").arg(work_dir.path(), device));

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "failed file open. " << file.fileName();
            continue;
        }

        // 设备文件需要使用qt的流方式读取，否则atEnd()无法正常终止循环
        QTextStream in(&file);
        QString line = in.readLine();
        while (!line.isNull()) {
            line = in.readLine();
            if (line.contains("PCI_ID=")) {
                QStringList pci_id = line.remove("PCI_ID=").split(":", QString::SkipEmptyParts);
                QString pci_type= pci_id.isEmpty() ? QString() : "PCI";
                QString vendor_id = (pci_id.size() == 2) ? pci_id.at(0) : QString();
                QString device_id = (pci_id.size() == 2) ? pci_id.at(1) : QString();

                AppendToConfigFile(QString("DI_VENDOR_%1_DEVICE_%2_%3").arg(vendor_id, device_id, pci_type), "true");
            }
        }

        file.close();
    }

}

}  // namespace installer
