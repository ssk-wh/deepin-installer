#!/bin/bash

set -x

. ./basic_utils.sh

DI_LOCALE=$(installer_get "DI_LOCALE")
DI_TIMEZONE=$(installer_get "DI_TIMEZONE")

deepin-installer-settings set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "LanguageSelected" ${DI_LOCALE}
deepin-installer-settings set "/etc/deepin/deepin-user-experience" "ExperiencePlan" "TimeZoneSelected" ${DI_TIMEZONE}

sync

return 0
