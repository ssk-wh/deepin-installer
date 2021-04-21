#!/bin/bash

. "/deepin-installer/basic_utils.sh"

# 这个文件是防止没有成功进到桌面，直接到lightdm_stop.sh了。
touch /tmp/in_check.file

if [ -d "/deepin-installer/in_check/" ];then
    for i in /deepin-installer/in_check/*; do
        chmod +x $i
        $i
        if [ $? != 0 ]; then
            echo ${system_info_si_password}|sudo -S touch /boot/efi/SI_FAILED
            setNetworkBoot
            echo ${system_info_si_password}|sudo reboot
        fi
    done
fi

# logout to run lightdm-stop.sh
qdbus --system --literal org.freedesktop.login1 /org/freedesktop/login1/session/self org.freedesktop.login1.Session.Terminate
