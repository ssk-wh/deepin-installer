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

#ifndef INSTALLER_SERVICE_SETTINGS_NAME_H
#define INSTALLER_SERVICE_SETTINGS_NAME_H

namespace installer {

// Keys used in settings ini file.

// Pages
const char kSkipDiskSpaceInsufficientPage[] =
    "skip_disk_space_insufficient_page";
const char kSkipVirtualMachinePage[] = "skip_virtual_machine_page";
const char kSkipSelectLanguagePage[] = "skip_select_language_page";
const char kSkipSelectLanguagePageOnFirstBoot[] = "skip_select_language_page_on_first_boot";
const char kSkipSystemInfoPage[] = "skip_system_info_page";
const char kSkipTimezonePage[] = "skip_timezone_page";
const char kSkipNetworkPage[] = "skip_network_page";
const char kSkipPartitionPage[] = "skip_partition_page";
const char kSkipControlPlatformPage[] = "skip_control_platform_page";
const char kSkipSelectComponentPage[] = "skip_select_component_page";
const char kSkipAutoSyncTimePage[] = "skip_auto_sync_time_page";
const char kSkipRepairSystemPage[] = "skip_repair_system_page";
const char kSkipPxeInstallerSystem[] = "skip_pxe_installer_system";
const char kSkipSystemKeyboardPage[] = "skip_system_keyboard_page";

// System Language List
const char kSelectLanguageDefaultLocale[] = "select_language_default_locale";
const char kSystemDefaultUserExperience[] = "system_default_user_experience";

// Repair System
const char kRepairScriptPoints[] = "repair_script_points";

// System Info
const char kSystemInfoDeepinVersion[] = "system_info_deepin_version";
const char kSystemInfoLsbRelease[] = "system_info_lsb_release";
const char kSystemInfoVendorName[] = "system_info_vendor_name";
const char kSystemInfoSetupAfterReboot[] = "system_info_setup_after_reboot";
const char kSystemInfoDefaultUsername[] = "system_info_default_username";
const char kSystemInfoLockUsername[] = "system_info_lock_username";
const char kSystemInfoUsernameMinLen[] = "system_info_username_min_len";
const char kSystemInfoUsernameMaxLen[] = "system_info_username_max_len";
const char kSystemInfoDefaultHostname[] = "system_info_default_hostname";
const char kSystemInfoLockHostname[] = "system_info_lock_hostname";
const char kSystemInfoHostnameAutoSuffix[] = "system_info_hostname_auto_suffix";
const char kSystemInfoHostnameReserved[] = "system_info_hostname_reserved";
const char kSystemInfoDefaultPassword[] = "system_info_default_password";
const char kSystemInfoLockPassword[] = "system_info_lock_password";
const char kSystemInfoPasswordMinLen[] = "system_info_password_min_len";
const char kSystemInfoPasswordMaxLen[] = "system_info_password_max_len";
const char kSystemInfoPasswordValidate[] = "system_info_password_validate";
const char kSystemInfoPasswordValidateRequired[] = "system_info_password_validate_required";
const char kSystemInfoPasswordRequireNumber[] =
    "system_info_password_require_number";
const char kSystemInfoPasswordRequireLowerCase[] =
    "system_info_password_require_lower_case";
const char kSystemInfoPasswordRequireUpperCase[] =
    "system_info_password_require_upper_case";
const char kSystemInfoPasswordRequireSpecialChar[] =
    "system_info_password_require_special_char";
const char kSystemInfoPasswordStrongCheck[] = "system_info_password_strong_check";
const char kSystemInfoDefaultAvator[] = "system_info_default_avatar";
const char kSystemInfoDdeAvatarDir[] = "system_info_dde_avatar_dir";
const char kSystemInfoDisableAvatorPage[] = "system_info_disable_avatar_page";
const char kSystemInfoEnableGrubEditPwd[] = "system_info_enable_grub_pwd";
const char kRebootWhenInstallFinished[] = "reboot_when_install_finished";
const char kSetRootPasswordFromUser[] = "set_root_password_from_user";

const char kSystemInfoDefaultRootPassword[] = "system_info_default_root_password";

const char kSystemInfoDefaultKeyboardLayout[] =
    "system_info_default_keyboard_layout";
const char kSystemInfoDefaultKeyboardLayoutVariant[] =
    "system_info_default_keyboard_layout_variant";
const char kSystemInfoDisableKeyboardPage[] =
    "system_info_disable_keyboard_page";
const char kSystemInfoDisableLicense[] = "system_info_disable_license";
const char kSystemInfoDisableExperience[] = "system_info_disable_experience";
const char kSystemInfoPasswordDistCheck[] = "system_info_password_dist_check";
const char kSystemInfoPasswordDistPath[] = "system_info_password_dist_path";
const char kSystemInfoPasswordPalindromeCheck[] = "system_info_password_palingrome_check";
const char kSystemInfoPasswordPalindromeLength[] = "system_info_password_palingrome_length";

// Timezone
const char kTimezoneDefault[] = "timezone_default";
const char kTimezoneUseWindowsTime[] = "timezone_use_windows_time";
const char kTimezoneUseGeoIp[] = "timezone_use_geoip";
const char kTimezoneUseRegdomain[] = "timezone_use_regdomain";
const char kTimezoneUseLocalTime[] = "timezone_use_local_time_regardless";

// Partition
const char kPartitionDefaultButton[] = "partition_default_button";
const char kPartitionSkipFullDiskPartitionPage[] =
    "partition_skip_full_disk_partition_page";
const char kPartitionSkipSimplePartitionPage[] =
    "partition_skip_simple_partition_page";
const char kPartitionEnableSwapFile[] = "partition_enable_swap_file";
const char kPartitionEnableSwapFileInAdvancedPage[] =
    "partition_enable_swap_file_in_advanced_page";
const char kPartitionForceSwapFileInSimplePage[] =
    "partition_force_swap_file_in_simple_page";
const char kPartitionMemoryThresholdForSwapArea[] =
    "partition_memory_threshold_for_swap_area";
const char kPartitionSwapPartitionSize[] = "partition_swap_partition_size";
const char kPartitionMinimumDiskSpaceRequired[] =
    "disk_minimum_space_required";
const char kPartitionRecommendedDiskSpace[] =
    "disk_recommended_space";
const char kPartitionRootMiniSpace[] = "partition_root_space_required";
const char kPartitionFullDiskMiniSpace[] = "partition_full_disk_minimum_space";
const char kPartitionDefaultBootSpace[] = "partition_default_boot_space";
const char kPartitionDefaultEFISpace[] = "partition_default_efi_space";
const char kPartitionEFIMinimumSpace[] = "partition_efi_minimum_space";
const char kPartitionSupportedFs[] = "partition_supported_fs";
const char kPartitionDefaultFs[] = "partition_default_fs";
const char kServerPartitionDefaultFs[] = "server_partition_default_fs";
const char kPartitionDoAutoPart[] = "partition_do_auto_part";
const char kPartitionMountPoints[] = "partition_mount_points";
const char kPartitionFormattedMountPoints[] =
    "partition_formatted_mount_points";
const char kPartitionBootOnFirstPartition[] =
    "partition_boot_on_first_partition";
const char kPartitionPreferLogicalPartition[] =
    "partition_prefer_logical_partition";
const char kPartitionBootPartitionFs[] = "partition_boot_partition_fs";
const char kPartitionEnableOsProber[] = "partition_enable_os_prober";
const char kPartitionHideInstallationDevice[] =
    "partition_hide_installation_device";

const char kPartitionFullDiskLargeDiskThreshold[] =
    "partition_full_disk_large_disk_threshold";
const char kPartitionFullDiskSmallLegacyPolicy[] =
    "partition_full_disk_small_legacy_policy";
const char kPartitionFullDiskSmallUEFIPolicy[] =
    "partition_full_disk_small_uefi_policy";
const char kPartitionFullDiskLargeLegacyPolicy[] =
    "partition_full_disk_large_legacy_policy";
const char kPartitionFullDiskLargeUEFIPolicy[] =
    "partition_full_disk_large_uefi_policy";
const char kPartitionFullDiskLargeRootPartRange[] =
    "partition_full_disk_large_root_part_range";

const char KPartitionSkipFullCryptPage[] =
    "partition_skip_partition_crypt_page";

// Nvidia 驱动配置选项
const char KEnableInstallNvidiaDriver[] =
    "enable_install_nvidia_driver";
const char kEnablePcTypeCheck[] =
    "enable_pc_type_check";

const char kPartitionFullDiskSmallLegacyLabel[] =
    "partition_full_disk_small_legacy_label";
const char kPartitionFullDiskSmallUEFILabel[] =
    "partition_full_disk_small_uefi_label";
const char kPartitionFullDiskLargeLegacyLabel[] =
    "partition_full_disk_large_legacy_label";
const char kPartitionFullDiskLargeUEFILabel[] =
    "partition_full_disk_large_uefi_label";

const char kEnableRecoveryPartition[] = "enable_recovery_partition";
const char kRecoveryDefaultSize[] = "recovery_partition_default_size";

// Install progress page
const char kInstallProgressPageDisableSlide[] =
    "install_progress_page_disable_slide";
const char kInstallProgressPageDisableSlideAnimation[] =
    "install_progress_page_disable_slide_animation";
const char kInstallProgressPageAnimationDuration[] =
    "install_progress_page_animation_duration";

const char kInstallProgressOsTypeSlide[] = "install_progress_os_type_slide";

// Install failed page
const char kInstallFailedFeedbackServer[] = "install_failed_feedback_server";
const char kInstallFailedQRErrMsgLen[] = "install_failed_qr_err_msg_len";
const char kInstallFailedErrMsgLen[] = "install_failed_err_msg_len";

// Misc
const char kScreenDefaultBrightness[] = "screen_default_brightness";

// Statistics script runtime
const char kEnableAnalysisScriptTime[] = "enable_analysis_script_time";

// End point control
const char kEndPointControlServerUrl[] = "end_point_control_server_url";
const char kEndPointControlLockServer[] = "end_point_control_lock_server";

// System install default environment type
const char kSelectComponentDefaultInstallType[] = "select_component_default_install_type";
// System install selected environment type
const char kSelectedInstallType[] = "selected_component_install_type";

// Minimum size of Other partitions
const char kPartitionOthersMinimumSize[] = "partition_others_minimum_size";

// Auto detect installation mode
const char kAutoDetectInstallationMode[] = "auto_detect_installation_mode";

// Force the Legacy(MBR) installation mode
const char kForceLegacyInstallationMode[] = "force_legacy_installation_mode";

}  // namespace installer

#endif  // INSTALLER_SERVICE_SETTINGS_NAME_H
