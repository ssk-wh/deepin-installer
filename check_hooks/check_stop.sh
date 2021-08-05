#!/bin/bash

. "/deepin-installer/basic_utils.sh"

# remove check mode files and test user
userdel -rf ${SI_USERNAME}
rm -rf /deepin-installer
installer_set "system_check_mode" "false"
reboot
