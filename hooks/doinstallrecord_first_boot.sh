#!/bin/bash

set -x

. ./basic_utils.sh

DI_USER_EXPERIENCE=$(installer_get "DI_USER_EXPERIENCE")

# if not use experience then return 
if [ x${DI_USER_EXPERIENCE} == "xtrue" ]; then
   DI_LOCALE=$(installer_get "DI_LOCALE")
   DI_TIMEZONE=$(installer_get "DI_TIMEZONE")

   deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "LanguageSelected" ${DI_LOCALE}
   deepin-installer-simpleini set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "TimeZoneSelected" ${DI_TIMEZONE}
   sync
else
   sudo rm /etc/deepin/deepin-user-experience
fi

return 0
