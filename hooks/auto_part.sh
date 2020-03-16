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

# Automatically create disk partitions.
# Partition policy is defined in settings.ini.
declare -i  DEVICE_SIZE AVL_SIZE PART_NUM=0 LVM_NUM=0 LAST_END=1
declare CRYPT_INFO DEVICE PART_POLICY PART_LABEL MP_LIST VG_NAME="vg0" PART_TYPE="primary" \
        LARGE="false" EFI="false" CRYPT="false" LVM="false"

# Check device capacity of $DEVICE.
check_device_size(){
  local index=$1
  echo "index={${index}}"
  local minimum_disk_size=$(installer_get disk_minimum_space_required)
  local large_disk_threshold=$(installer_get partition_full_disk_large_disk_threshold)
  DEVICE_SIZE=$(($(blockdev --getsize64 "$DEVICE") / 1024**2))

  if ((DEVICE_SIZE < minimum_disk_size * 1024 && index == 0)); then
    error "At least ${minimum_disk_size}Gib is required to install system!"
  elif ((DEVICE_SIZE > large_disk_threshold * 1024)); then
    declare -g LARGE=true
  fi
}

# Check boot mode is UEFI or not.
check_efi_mode(){
  if [ -d "/sys/firmware/efi" ]; then
    declare -g EFI=true
  fi
  local force_efi=$(installer_get force_efi_mode)
  if [ x"$force_efi" = "xtrue" ];then
    declare -g EFI=true
  fi
}

chech_use_crypt(){
  DI_CRYPT_PASSWD=$(installer_get DI_CRYPT_PASSWD)
  if [ -n "$DI_CRYPT_PASSWD" ]; then
    declare -g CRYPT=true
    installer_set DI_CRYPT_PASSWD "NULL"
  fi
}

# Flush kernel message.
flush_message(){
  udevadm settle --timeout=5
}

# Format partition at $1 with filesystem $2.
format_part(){
  local part_path="$1" part_fs="$2" part_label="$3"
  local part_fs_="$part_fs"
  if [ "$part_fs_" = "recovery" ]; then
     part_fs_=ext4
  fi

  yes |\
  case "$part_fs_" in
    fat32)
      mkfs.vfat -F32 -n "$part_label" "$part_path";;
    efi)
      mkfs.vfat -F32 -n "$part_label" "$part_path";;
    fat16)
      mkfs.vfat -F16 -n "$part_label" "$part_path";;
    ntfs)
      mkfs.ntfs --fast -L "$part_label" "$part_path";;
    linux-swap)
      mkswap "$part_path";;
    ext4)
      if is_loongson || is_sw; then
        mkfs.ext4 -O ^64bit -F -L "$part_label" "$part_path"
      else
        mkfs.ext4 -L "$part_label" "$part_path"
      fi
    ;;
    *)
      mkfs -t "$part_fs" -L "$part_label" "$part_path";;
  esac || error "Failed to create $part_fs filesystem on $part_path!"
}

get_max_capacity_device(){
  local name size max_device max_size=0
  while read name size; do
    if ((size >= max_size)); then
      max_size="$size"
      max_device="/dev/$name"
    fi
  done < <(lsblk -ndb -o NAME,SIZE)
  DEVICE="$max_device"
}

# Create new partition table.
new_part_table(){
  {
    $EFI && local part_table="gpt"
  } || local part_table="msdos"

  parted -s "$DEVICE" mktable "$part_table" ||\
    error "Failed to create $part_table partition on $DEVICE!"
}

umount_devices(){
  # Umount all swap partitions.
  swapoff -a

  # Umount /target
  [ -d /target ] && umount -R /target
}

# Create partition.
create_part(){
  local part="$1"
  local label="$2"
  local swap_size part_path part_mp part_fs _part_fs part_start part_end mapper_name

  let PART_NUM++
  echo "============PART_NUM: $PART_NUM============"

  echo "PART:{${part}},label:{${label}}}"

  # Create extended partition.
  if ((PART_NUM == 4)) && ! ($EFI || $LVM); then
    echo "Create extended partition..."
    parted -s "$DEVICE" mkpart extended "${LAST_END}Mib" 100% ||\
      error "Failed to create extended partition on $DEVICE!"
    let PART_NUM++
    echo "============PART_NUM: $PART_NUM============"
    PART_TYPE="logical"
  fi

  # Get partition info.
  [[ "$part" =~ ^(.*):(.*):(.*):(.*)$ ]] || error "Bad partition info!"
  part_mp="${BASH_REMATCH[1]}"
  part_fs="${BASH_REMATCH[2]}"
  part_size="${BASH_REMATCH[4]}"
  if ! $LVM; then
    part_start="${BASH_REMATCH[3]}"
    if [ -z "$part_start" ]; then
      if [ "$PART_TYPE" = "logical" ]; then
        part_start=$((LAST_END + 1))
      else
        part_start="$LAST_END"
      fi
    fi
    AVL_SIZE=$((DEVICE_SIZE - 1 - part_start))
  else
    [[ "$(vgs -ovg_free --readonly --units m $VG_NAME)" =~ ([0-9]+) ]] &&\
      AVL_SIZE="${BASH_REMATCH[1]}"
  fi

  echo "Avaiable size: $AVL_SIZE"

  if [[ "$part_size" =~ %$ ]]; then
    part_size=$((AVL_SIZE * ${part_size%\%} / 100))
  elif [ "$part_size" = swap-size ]; then
    swap_size=$(installer_get DI_SWAP_SIZE)
    if ((swap_size > 0)); then
      part_size=$((swap_size * 1024))
    else
      part_size=1024
    fi
  fi

  echo "Calculated partition size: $part_size"
  if [ ${part_size} -gt ${AVL_SIZE} ] && [ ${AVL_SIZE} -gt 0 ]; then
      echo "adjuest part_size:${part_size}=>${AVL_SIZE}"
      part_size=${AVL_SIZE}
  fi

  # Check root partition size.
  if $LARGE && [ "$part_mp" = '/' ] &&\
  [[ "$(installer_get partition_full_disk_large_root_part_range)" =~ ^([0-9]+):([0-9]+)$ ]]; then
    local root_min=$((BASH_REMATCH[1] * 1024)) root_max=$((BASH_REMATCH[2] * 1024))

    echo "Arrange root partition size based on the restriction rules, root_min: $root_min root_max: $root_max"

    part_size=$((part_size < root_min ? root_min : part_size))
    part_size=$((part_size > root_max ? root_max : part_size))
  fi

  echo "part: $part_mp, $part_fs, $part_start, $part_size"

  # Create new partition.
  if ! $LVM; then
    case "$part_fs" in
      efi)
        _part_fs=fat32;;
      crypto_luks)
        _part_fs='';;
      recovery)
        _part_fs=ext4;;
      *)
        printf -v _part_fs '%q' "$part_fs";;
    esac
    part_end=$((part_start + part_size))
    if parted -s "$DEVICE" mkpart "$PART_TYPE" $_part_fs "${part_start}Mib" "${part_end}Mib"; then
      if [[ "$DEVICE" =~ [0-9]$ ]]; then
        part_path="${DEVICE}p${PART_NUM}"
      else
        part_path="${DEVICE}${PART_NUM}"
      fi
      LAST_END="$part_end"
    else
      error "Failed to create partition $part_mp!"
    fi
  else
    let LVM_NUM++
    echo "{LVM_NUM:{${LVM_NUM},label:{${label:-LVM_NUM}} vg_name:{${VG_NAME}}"
    lvcreate --wipesignatures y -n"${label:-LVM_NUM}" -L"$part_size" "$VG_NAME" ||\
    error "Failed to create logical volume ${label:-LVM_NUM} on $VG_NAME!"
    part_path="/dev/$VG_NAME/${label:-LVM_NUM}"
  fi

  flush_message

  # Create filesystem.
  case "$part_fs" in
    efi)
      {
        format_part "$part_path" fat32 "$label" &&\
        # Set esp flag.
        parted -s "$DEVICE" set "$PART_NUM" esp on
      } || error "Failed to create ESP($part_path) on $DEVICE!"

      # No need to append EFI partition to mount point list.
      # MP_LIST="${MP_LIST+$MP_LIST;}$part_path=$part_mp"
      installer_set DI_BOOTLOADER "$part_path"
      ;;
    crypto_luks)
      mapper_name="$part_mp"
      part_mp="/dev/mapper/$mapper_name"


      installer_set DI_CRYPT_ROOT "true"

      [[ -n ${CRYPT_INFO} ]] && CRYPT_INFO+=";"
      CRYPT_INFO+="$part_path:$mapper_name"

      {
        echo -n "$DI_CRYPT_PASSWD" | cryptsetup -v luksFormat "$part_path" &&\
        echo -n "$DI_CRYPT_PASSWD" | cryptsetup open "$part_path" "$mapper_name"
      } || error "Failed to create luks partition($part_path)!"

      {
        pvcreate "$part_mp" -ffy &&\
        vgcreate "$VG_NAME" "$part_mp"
      } || error "Failed to create volume group: $VG_NAME!"
      declare -g LVM="true"
      ;;
    *)
      format_part "$part_path" "$part_fs" "$label" ||\
        error "Failed to create $part_fs filesystem on $part_path!"
      [ -n "$part_mp" ] && MP_LIST="${MP_LIST+$MP_LIST;}$part_path=$part_mp"
      ;;
  esac

  flush_message

  # Set boot flag.
  case "$part_mp" in
    /boot)
      # Set boot flag in legacy mode.
      $EFI || parted -s "$DEVICE" set "$PART_NUM" boot on
      ;;
    /)
      installer_set "DI_ROOT_PARTITION" "$part_path"
      # Set boot flag if /boot is not used in legacy mode.
      ($EFI || $LVM || [[ "$PART_POLICY" =~ "/boot:" ]]) ||\
        parted -s "$DEVICE" set "$PART_NUM" boot on
      ;;
  esac || error "Failed to set boot flag on $part_path!"

  flush_message
}

main(){
  # Based on the following process:
  #  * Umount devices.
  #  * Get boot mode.
  #  * Get partitioning policy.
  #  * Create new partition table.
  #  * Create partitions.
  #  * Notify kernel.

  installer_set DI_UEFI "false"
  umount_devices

  local policy_device="DI_FULLDISK_MULTIDISK_DEVICE"
  local policy_name="DI_FULLDISK_MULTIDISK_POLICY_"
  local policy_name_label="DI_FULLDISK_MULTIDISK_LABEL_"

  local PART_DEVICE=$(installer_get $policy_device)
  local part_device_array=(${PART_DEVICE//;/ })
  declare -i index=0

  chech_use_crypt

  for j in "${part_device_array[@]}"; do
     DEVICE="${j}"
     VG_NAME="vg${index}"
     echo "Target device: {$DEVICE} VG_NAME:{${VG_NAME}}"

     DEVICE_SIZE=0
     AVL_SIZE=0
     PART_NUM=0
     LVM_NUM=0
     LAST_END=1

     LVM="false"
     PART_TYPE="primary"
     LARGE="false"
     EFI="false"
     CRYPT="false"
     LVM="false"

     if [ "$DEVICE" = auto_max ]; then
        get_max_capacity_device
     fi
     [ -b "$DEVICE" ] || error "Device not found!"

    check_device_size ${index}
    check_efi_mode
    echo "Device size: $DEVICE_SIZE"

    PART_POLICY=$(installer_get ${policy_name}${index})
    PART_LABEL=$(installer_get ${policy_name_label}${index})
    echo "POLICY:{${PART_POLICY}}"
    echo "LABEL:{${PART_LABEL}}"

    new_part_table "$DEVICE"

    local part_policy_array=(${PART_POLICY//;/ })
    local part_label_array=(${PART_LABEL//;/ })
    echo "policy#:${#part_policy_array[@]} label:${#part_label_array[@]}"

    for i in "${!part_policy_array[@]}"; do
        create_part ${part_policy_array[$i]} ${part_label_array[$i]}
    done
    index=index+1
  done

  echo "MOUNTPOINTS:{${MP_LIST}}"
  echo "ROOT_DISK:{${part_device_array[0]}}"

  installer_set DI_MOUNTPOINTS "$MP_LIST"

#  # Write boot method.
  if $EFI; then
    installer_set DI_UEFI "true"
  else
    installer_set DI_BOOTLOADER "${part_device_array[0]}"
  fi

  echo "CRYPT_INFO:{${CRYPT_INFO}}"
  installer_set DI_CRYPT_INFO "${CRYPT_INFO}"
  installer_set DI_ROOT_DISK "${part_device_array[0]}"
  installer_set DI_FULLDISK_MODE "true"
}

. ./basic_utils.sh

DI_CUSTOM_PARTITION_SCRIPT=$(installer_get DI_CUSTOM_PARTITION_SCRIPT)
if [ -f "$DI_CUSTOM_PARTITION_SCRIPT" ]; then
  echo "Call custom partition script($DI_CUSTOM_PARTITION_SCRIPT)..."
  bash "$DI_CUSTOM_PARTITION_SCRIPT"
else
  main
fi
