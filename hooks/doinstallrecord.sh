#!/bin/bash

set -x

. ./basic_utils.sh

DI_USER_EXPERIENCE = DI_TIMEZONE=$(installer_get "DI_USER_EXPERIENCE")

# if not use experience then return
if [ x${DI_USER_EXPERIENCE} == "xtrue" ]; then
   DI_FULLDISK_MODE=$(installer_get "DI_FULLDISK_MODE")
   partition_do_auto_part=$(installer_get "partition_do_auto_part")
   DI_AUTO_MOUNT=$(installer_get "DI_AUTO_MOUNT")
   DI_INSTALL_DURATIONTIME=$(installer_get "DI_INSTALL_DURATIONTIME")
   DI_INSTALL_SUCCESSED=$(installer_get "DI_INSTALL_SUCCESSED")
   DI_CRYPT_PASSWD=$(installer_get "DI_CRYPT_PASSWD")
   DI_LOCALE=$(installer_get "DI_LOCALE")
   DI_TIMEZONE=$(installer_get "DI_TIMEZONE")

   # mount all point
   /tmp/installer/hook_manager.sh /tmp/installer/before_chroot/11_mount_target.job
   /tmp/installer/hook_manager.sh /tmp/installer/before_chroot/41_setup_mount_points.job

   if [ x${partition_do_auto_part} == "xtrue" ]; then
       chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "InstallType" "AutoInstall"
   else
       if [ x${DI_FULLDISK_MODE} == "xtrue" ]; then
           chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "InstallType" "FullDiskPartition"
       else
           chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "InstallType" "AdvancedPartition"
       fi
   fi
   if [ x${DI_AUTO_MOUNT} != "x" ]; then
       chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "AutoMount" ${DI_AUTO_MOUNT}
   else
       chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "AutoMount" "false"
   fi
   chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "InstallDuration" ${DI_INSTALL_DURATIONTIME}
   chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "InstallSuccess" ${DI_INSTALL_SUCCESSED}
   if [ x${DI_CRYPT_PASSWD} != "x" ]; then
       chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "FullDiskEncrypt" "true"
   else
       chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "FullDiskEncrypt" "false"
   fi
   chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "LanguageSelected" ${DI_LOCALE}
   chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "TimeZoneSelected" ${DI_TIMEZONE}
   IS_HASRECOVERY=`lsblk | grep "/recovery" | awk '{print $7}'`
   if [ x${IS_HASRECOVERY} != "x" ];then
       chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "UseRecovery" "true"
   else
       chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "UseRecovery" "false"
   fi
   #chroot /target deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "PasswordStrength"

   # umount all point
   /tmp/installer/hook_manager.sh /tmp/installer/after_chroot/90_unmount.job

   sync
else
   sudo rm /etc/deepin/deepin-user-experience
fi

return 0
