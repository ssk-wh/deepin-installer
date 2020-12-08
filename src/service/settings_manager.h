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

#ifndef INSTALLER_SETTINGS_MANAGER_H
#define INSTALLER_SETTINGS_MANAGER_H

#include <QDir>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace installer {

    enum class OSType {
        Community,
        Professional,
        Server,
        Personal
    };

// Get absolute path to oem/ folder. Note that oem folder may not exist.
QDir GetOemDir();

OSType GetCurrentType();

QString GetCurrentPlatform();

// Read settings value from ini file.

class SettingCustom : public QObject
{
    Q_OBJECT
public:
    static SettingCustom* Instance();

    void setSettingsBool(const QString& key, const bool value);
    bool getSettingsBool(const QString& key);
    bool hasSetting(const QString& key);

private:
    QObject settingObject;
};

bool isNotebook();

// Get boolean option value from settings file.
// If |key| does not exist, returns false.
bool GetSettingsBool(const QString& key);

// Get integer option value from settings file.
// If |key| does not exist, return 0.
int GetSettingsInt(const QString& key);

// 获取系统是否为pxe安装
bool isPexInstall();

// Get string option value from settings file.
// If |key| does not exist, returns an empty string.
QString GetSettingsString(const QString& key);

// Get string list option value form settings file.
// Items in value are separated by ;
// If |key| does not exist, returns an empty string list.
QStringList GetSettingsStringList(const QString& key);

// Get variant option value from settings file.
QVariant GetSettingsValue(const QString& key);

// Returns absolute path to oem/auto_part.sh
QString GetAutoPartFile();

// Get all available avatars in avatar folder.
QStringList GetAvatars();

// Get default user avatar, is specified. Or else returns a random avatar.
QString GetDefaultAvatar();

// Get absolute path to oem hooks folder.
QString GetOemHooksDir();

// Get absolute path to oem check_hooks foler.
QString GetOemCheckHooksDir();

// Returns absolute path to reserved_usernames file.
QString GetReservedUsernameFile();

// Get vendor logo.
QString GetVendorLogo();

// Get image path of installer window background.
// First try to find installer background image in oem folder.
// If not found, use the fallback image.
QString GetWindowBackground();

// Get Full install policy, it is JSON.
QByteArray GetFullDiskInstallPolicy();

// Get Component Files
QString GetSelectedInstallType();
QString GetComponentDefault();
QString GetComponentExtra();
QString GetComponentSort();
QString GetLanguageSort();

// Append settings in |conf_file| into default conf file.
bool AppendConfigFile(const QString& conf_file);

// Operations of /etc/deepin-installer.conf
bool DeleteConfigFile();

// Setup uefi mode or not.
void WriteUEFI(bool is_efi);

void WriteEnableNvidiaDriver(bool is_enable_nvidia_driver);

//void WriteInstallerMode(bool is_simple_mode);

// Get current locale.
QString ReadLocale();

// Get intend to uninstall packages.
QStringList ReadComponentUninstallPackages();

QString GetUIDefaultFont();

void WriteIsAutoMount(bool auto_mount);

void WriteAvatar(const QString& avatar);
void WriteHostname(const QString& hostname);
// Write keyboard model, layout and layout variant name.
void WriteKeyboard(const QString& model,
                   const QString& layout,
                   const QString& variant);
void WriteLocale(const QString& locale);
void WritePassword(const QString& password);
void WriteRootPassword(const QString& password);
void WriteTimezone(const QString& timezone);
void WriteIsEnableNTP(bool isEnableNTP);
void WriteTimedate(const QString& timeDate);
void WriteUsername(const QString& username);
void WriteFullDiskDeivce(const QString &deviceName);
void WriteFullDiskEncryptPassword(const QString &password);
void WritePasswordStrong(bool storePassword);
void WriteDisplayPort(const QString &display);
void WriteGrubPassword(const QString &password);

void WriteSelectedInstallType(const QString& installType);
void WriteComponentPackages(const QString& packages);
void WriteComponentUninstallPackages(const QString& packages);
void WriteComponentLanguage(const QString& packages);
void WriteIsMinimalGraphicsSystem(const bool isMinimalGraphicsSystem);
void WriteIsMinimalCharacterSystem(const bool isMinimalCharacterSystem);
void WriteInstallSuccessed(bool successed);
void CustomSetSettingsBool(const char* key, bool boolvalue);
// Write disk info.
//  * |root_disk|, device path to install system into, like /dev/sda;
//  * |root_partition|, partition path to install system into;
//  * |boot_partition|, partition path to install grub into;
//  * |mount_point|, a list of partition path and mount-point info,
//    items are separated by ';'
void WritePartitionInfo(const QString& root_disk,
                        const QString& root_partition,
                        const QString& boot_partition,
                        const QString& mount_points);

// Whether swap file is required. Swap file is created in before_chroot/.
void WriteRequiringSwapFile(bool is_required);

// The user experience configuration options
void WriteUserExperience(bool checked);

// The user license configuration options
void WriteUserLicenseIsUse(bool enable);

// Save current settings to /etc/deepin-installer.conf
// Other settings will be updated later.
void AddConfigFile();

//Run some job before install
void BeforeInstallHook();

// Save swap size for FullDiskInstall
void WriteSwapPartitionSize(const uint size);

//Full disk policy settings.
struct FinalFullDiskPolicy {
    QString       filesystem;
    QString       mountPoint;
    QString       label;
    QString       device;
    qint64        offset;
    qint64        size;
};

typedef QList<FinalFullDiskPolicy> FinalFullDiskPolicyList;

struct FinalFullDiskOption {
    QString                  device;
    QString                  password;
    FinalFullDiskPolicyList  policy_list;
    bool                     encrypt;
};

typedef QList<FinalFullDiskOption> FinalFullDiskOptionList;

struct FinalFullDiskResolution {
    FinalFullDiskOptionList  option_list;
};

void WriteFullDiskResolution(const FinalFullDiskResolution& resolution);

void WriteFullDiskMode(bool value);

struct DiskPartitionSetting {
    DiskPartitionSetting();
    QString  root_disk;
    QString  root_partition;
    QString  boot_partition;
    QString  mount_points;
    bool     swap_file_required;
    bool     uefi_required;
};

void WriteDiskPartitionSetting(const DiskPartitionSetting& setting);

void WriteNecuresCliInstallMode(bool mode);
}  // namespace installer

#endif  // INSTALLER_SETTINGS_MANAGER_H
