#!/bin/bash

# SI = System Integrity

. "/deepin-installer/basic_utils.sh"
. "/usr/share/deepin-installer/hooks/basic_utils.sh"

if [ -d "/deepin-installer/before_check/" ];then
    JOBS=$(ls /deepin-installer/before_check/*.job)
    for i in $JOBS; do
        chmod +x $i
        $i
        if [ $? != 0 ]; then
            echo "Check Mode faild: $i" >> /var/log/deepin-installer.log
            touch /boot/SI_FAILED
            setNetworkBoot
            reboot
        fi
    done
fi

# wait for lightdm
sleep 5
