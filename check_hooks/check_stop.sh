#!/bin/bash

. "/deepin-installer/basic_utils.sh"

clean_disk_cryption() {
    local key_file=/boot/keyfile
    local cryp_conf_file=/etc/crypttab

    mv ${cryp_conf_file}.real ${cryp_conf_file}
    rm -fr $key_file
    /usr/sbin/update-initramfs -u
}

# remove check mode files and test user
clean_check_mode() {
    userdel -rf ${SI_USERNAME}
    rm -rf /deepin-installer
    installer_set "system_check_mode" "false"
}

clean_disk_cryption
clean_check_mode
reboot
