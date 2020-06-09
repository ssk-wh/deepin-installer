#!/usr/bin/env python3
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

# Update i18n/installer.ts and i18n/oem.ts

import json
import os
import sys

def get_language_list():
    """Parse language list and returns locale list."""
    path = "resources/languages.json"
    with open(path) as fh:
        obj = json.load(fh)
    if obj:
        return [lang["locale"] for lang in obj]
    else:
        return []

def generate_ts(ts_path, is_installer):
    """Generate new ts file.

    Scan source files in `ui` module recursively.
    And save result to |ts_path|.

    If |is_installer| is True, generate files for installer project.
    Else generate ts files for oem project.
    """
    if is_installer:
        paths = (
            "src/partman",
            "src/ui/delegates",
            "src/ui/first_boot_setup_window.cpp",
            "src/ui/first_boot_setup_window.h",
            "src/ui/frames",
            "src/ui/main_window.cpp",
            "src/ui/main_window.h",
            "src/ui/models",
            "src/ui/views",
            "src/ui/widgets",
            "src/ui/frames_cli",
            "src/ui/models_cli",
            "src/ui/ncurses_widgets",
            "src/base",
            "src/ui/main_window_cli.cpp",
            "src/ui/main_window_cli.h"
        )
    else:
        # Only include oem folder.
        paths = ["src/oem"]

    # Add -I. option to solve namespace error
    cmd = " ".join((
        "lupdate -recursive -I.",
        " ".join(paths),
        "-ts", ts_path,
        ))
    os.system(cmd)

def main():
    # Make sure that PWD is root of source repo.
    if not os.path.isdir("i18n"):
        print("Run script in parent folder of `i18n`")
        sys.exit(1)

    default_installer_ts = "i18n/deepin-installer.ts"
    generate_ts(default_installer_ts, True)

    # Then, generate ts files for oem project.
    oem_ts = "i18n/oem-zh_CN.ts"
    generate_ts(oem_ts, False)

if __name__ == "__main__":
    main()
