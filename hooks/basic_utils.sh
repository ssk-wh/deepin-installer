#!/bin/bash
#
# Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# This module defines basic utilities used in almost all scripts.

# Set environment
export LANG=C LC_ALL=C
export DEBIAN_FRONTEND="noninteractive"
export APT_OPTIONS='-y -o Dpkg::Options::="--force-confdef" \
  -o Dpkg::Options::="--force-confold" --force-yes --no-install-recommends \
  --allow-unauthenticated'

# Absolute path to config file.
# Do not read from/write to this file, call installer_get/installer_set instead.
CONF_FILE=/etc/deepin-installer.conf
EXPERIENCE_FILE=/etc/deepin/deepin-user-experience

# Print error message and exit
error() {
  local msg="$@"
  echo " "
  echo "!! Error: ${msg}" >&2
  echo " "
  exit 1
}

warn() {
  local msg="$@"
  echo "Warn: ${msg}" >&2
}

warn_exit() {
  local msg="$@"
  echo "Warn: ${msg}" >&2
  exit 0
}

# standard message
msg() {
  local msg="$@"
  echo "Info: ${msg}"
}

debug() {
  local msg="$@"
  echo "Debug: ${msg}"
}

# Get value in conf file. Section name is ignored.
# NOTE(xushaohua): Global variant or environment $CONF_FILE must not be empty.
installer_get() {
  local key="$1"
  [ -z "${CONF_FILE}" ] && exit "CONF_FILE is not defined"
  which deepin-installer-settings 1>/dev/null || \
    exit "deepin-installer-settings not found!"
  deepin-installer-settings get "${CONF_FILE}" "${key}"
}

installer_record_set(){
   #chroot /target
   local recordsession="$1"
   local recordkey="$2"
   local recordvalue="$3"
   [ -z "${EXPERIENCE_FILE}" ] && exit "EXPERIENCE_FILE is not defined"
   which deepin-installer-settings 1>/dev/null || \
     exit "deepin-installer-settings not found!"
   deepin-installer-settings set "${EXPERIENCE_FILE}" "${recordsession}" "${recordkey}" "${recordvalue}"
}

update_local() {
    local DI_LOCALE=$(installer_get "DI_LOCALE")
    DI_LOCALE=${DI_LOCALE:-en_US}

    export LANGUAGE=${DI_LOCALE}
    export LANG=${DI_LOCALE}.UTF-8
    export LC_ALL=
}

# update grub config by locale
update_grub_local() {
    update_local
    [ -x /usr/sbin/update-grub ] && /usr/sbin/update-grub
}

# Set value in conf file. Section name is ignored.
installer_set() {
  local key="$1"
  local value="$2"
  [ -z "${CONF_FILE}" ] && exit "CONF_FILE is not defined"
  which deepin-installer-settings 1>/dev/null || \
    exit "deepin-installer-settings not found!"
  deepin-installer-settings set "${CONF_FILE}" "${key}" "${value}"
}

# Check whether current platform is loongson or not.
is_loongson() {
  case $(uname -m) in
    loongson | mips* | loongarch64)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}

is_loongarch() {
  case $(uname -m) in
    loongarch64)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}


# Check whether current platform is sw or not.
is_sw() {
  case $(uname -m) in
    sw*)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}

# Check whether current platform is x86/x86_64 or not.
is_x86() {
  case $(uname -m) in
    i386 | i686 | amd64 | x86 | x86_64)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}

# Check whether current platform is arm64 or not.
is_arm64() {
  case $(uname -m) in
    arm64 | aarch64)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}

# Check whether graphics card is integrated or not.
is_loongson_integrated_graphics() {
    local ret=$(lshw -c video | grep "driver=loongson-drm")
    if [ ! -z "${ret}" ]; then
        return 0
    else
        return 1
    fi
}

install_package() {
#  DEBIAN_FRONTEND="noninteractive" apt-get -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" --no-install-recommends --allow-unauthenticated install $@
  for i in $@;
  do
    DEBIAN_FRONTEND="noninteractive" apt-get -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" --no-install-recommends --allow-unauthenticated install $i
    if [ $? -eq 0 ]; then
      echo "succeed"
    else
      echo "Install Failed : $i"
    fi
  done
}

setup_lightdm_auto_login() {
  if [ x$(installer_get "lightdm_enable_auto_login") != "xtrue" ]; then
    return 0
  fi

  local USERNAME
  USERNAME=$(installer_get "DI_USERNAME")
  [ -f /etc/lightdm/lightdm.conf ] || warn_exit "lightdm.conf not found!"
  deepin-installer-simpleini set /etc/lightdm/lightdm.conf \
    "Seat:*" "autologin-user" "${USERNAME}"
}

encryption_file() {
    local file=$1
    cat $file | base64 > $file.tmp
    mv $file.tmp $file && chmod 600 $file
}

decryption_file() {
    local file=$1
    cat $file | base64 -d > $file.tmp
    mv $file.tmp $file  && chmod 600 $file
}

add_start_option() {
    local arch_info=$@
    local bootloader_id=$(installer_get "system_bootloader_id")
    local start_option=$(installer_get "system_startup_option")

    ## 基础启动项,默认UOS
    grub-install $arch_info --efi-directory=/boot/efi --bootloader-id="${bootloader_id}" --recheck \
        || error "grub-install failed with $arch_info" "${bootloader_id}"

    ## 附加启动项
    if [ -n "$start_option" ]; then
        [ -d /boot/efi/EFI/${start_option} ] || mkdir -p /boot/efi/EFI/${start_option}
        cp -vf /boot/efi/EFI/${bootloader_id}/* /boot/efi/EFI/${start_option}
        grub-install $arch_info --efi-directory=/boot/efi --bootloader-id="${start_option}" --recheck \
            || error "grub-install failed with $arch_info"  "${start_option}"
    fi

    # Copy signed grub efi file.
    [ -d /boot/efi/EFI/ubuntu ] || mkdir -p /boot/efi/EFI/ubuntu
    cp -vf /boot/efi/EFI/${bootloader_id}/grub* /boot/efi/EFI/ubuntu/
    [ -d /boot/efi/EFI/boot ] || mkdir -p /boot/efi/EFI/boot
    cp -vf /boot/efi/EFI/${bootloader_id}/grub* /boot/efi/EFI/boot/

    # Backup fallback efi first.
    fallback_efi=/boot/efi/EFI/boot/bootaa64.efi
    fallback_efi_bak="${fallback_efi}-$(date +%s).bak"
    [ -f "${fallback_efi}" ] && cp "${fallback_efi}" "${fallback_efi_bak}"
    # Override fallback efi with shim.
    if ls /boot/efi/EFI/${bootloader_id}/shim* 1>/dev/null 2>&1; then
      cp -vf /boot/efi/EFI/${bootloader_id}/shim*.efi "${fallback_efi}"
    else
      cp -vf /boot/efi/EFI/${bootloader_id}/grubaa64.efi "${fallback_efi}"
    fi
}

fix_boot_order(){
  command -v efibootmgr >/dev/null 2>&1 || \
    warn "Require efibootmgr installed but not found. Skip"
  return

  local bootinfo=$(efibootmgr)
  echo "bootinfo: ${bootinfo}"
  IFS=$'\n'
  for line in $bootinfo;do
    case $line in
      Boot[0-9A-F][0-9A-F][0-9A-F][0-9A-F]\*\ "${BOOTLOADER_ID}")
        line="${line%%\**}"
        default_bootid="${line##Boot}"
      ;;
    esac
  done

  [ -z ${default_bootid} ] && warn_exit "No ${BOOTLOADER_ID} found, exit..."

  declare -a orderids
  for line in $bootinfo;do
    case $line in
      Boot[0-9A-F][0-9A-F][0-9A-F][0-9A-F]\*\ "${BOOTLOADER_ID}")
        ;;

      Boot[0-9A-F][0-9A-F][0-9A-F][0-9A-F]\*\ ?*)
        line="${line%%\**}"
        orderids[${#orderids[@]}]="${line##Boot}"
        ;;
    esac
  done

  local cmdargs=${default_bootid}
  for arg in ${orderids[@]}; do
    cmdargs=${cmdargs}","${arg}
  done
  efibootmgr -o ${cmdargs}
}

init_backup() {
    echo "init_backup"
}

init_checkmode() {
    echo "init_checkmode"
}

init_firstboot() {
    echo "init_firstboot"
    local CONF_FILE=/etc/lightdm/lightdm.conf
    local TEMP_CONF_FILE=/etc/lightdm/lightdm.conf.real
    if [ -f "${CONF_FILE}" ]; then
        install -v -m644 "${CONF_FILE}" "${TEMP_CONF_FILE}"
    fi

    cat > "${CONF_FILE}" <<EOF
[Seat:*]
display-setup-script=/usr/bin/deepin-installer-bases
greeter-setup-script=/usr/bin/deepin-installer-first-boot
EOF
}


