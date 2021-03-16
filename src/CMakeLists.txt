
#set(CMAKE_C_COMPILER  /usr/bin/clang)
#set(CMAKE_CXX_COMPILER  /usr/bin/clang++)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Only generate Debug and Release configuration types.
set(CMAKE_CONFIGURATION_TYPES Debug Release)
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
#   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -Wextra")
  # -fno-rtti, Disable real-time type information
  # -fno-exceptions Disable exceptions
  # -fsanitize=address Enable memory sanitize plugin
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -Wextra -fno-exceptions")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-implicit-fallthrough")
  # -fsanitize=address, Enable memory address sanitizer.
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  # -fsanitize=leak, Enable memory leak sanitizer.
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
  # -fsanitize=memory, Enable detecting uninitited memory sanitizer.
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory "
  #    "-fsanitize-memory-track-origins -fPIC -pie -fno-omit-frame-pointer")

#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wimplicit-fallthrough=1")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -fPIE")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -Wno-unused-result \
    -Wno-switch-default -Wno-padded -Wno-unused-function -Wno-reorder -Werror -fPIE -pie")

# Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT")
else()
  # -Wl, -O2 Enable linker optimizations
  #-Wl, --gc-sections Remove unused code resulting from -fdsta-sections and -ffunction-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -Wl,-O1 -Wl,--gc-sections -fPIE -pie")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -Wl,-O1 -Wl,--gc-sections -Wno-unused-result \
      -Wno-switch-default -Wno-padded -Wno-unused-function -Wno-reorder -fPIE -pie")
endif()

# Install default_settings.ini based on current architecture.
set(INSTALLER_BUILD_ARCH "")
execute_process(COMMAND uname -m
                OUTPUT_VARIABLE INSTALLER_BUILD_ARCH)
string(REGEX REPLACE "\n$" "" INSTALLER_BUILD_ARCH "${INSTALLER_BUILD_ARCH}")
message("build arch: ${INSTALLER_BUILD_ARCH}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(IS_PROFESSIONAL FALSE)
if (EXISTS "/etc/deepin-version")
  file(STRINGS "/etc/deepin-version" IS_PROFESSIONAL REGEX "Type *= *Professional")
  if(IS_PROFESSIONAL)
      add_definitions("-DPROFESSIONAL")
  endif()
endif()

set(INSTALLER_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/deepin-installer)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

add_definitions("-DPLATFORM_BUILD_ARCH=\"${INSTALLER_BUILD_ARCH}\"")
add_definitions("-DSOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

# Defines path to resource directories
if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions("-DBUILTIN_HOOKS_DIR=\"${CMAKE_SOURCE_DIR}/hooks\"")
  add_definitions("-DBUILTIN_CHECK_HOOKS_DIR=\"${CMAKE_SOURCE_DIR}/check_hooks\"")
  add_definitions("-DI18N_DIR=\"${CMAKE_BINARY_DIR}/i18n\"")
  add_definitions("-DRESOURCES_DIR=\"${CMAKE_SOURCE_DIR}/resources\"")
  add_definitions("-DTOOLS_DIR=\"${CMAKE_SOURCE_DIR}/tools\"")
  add_definitions("-DDATAS_DIR=\"${CMAKE_SOURCE_DIR}/data\"")
  add_definitions("-DXRANDR_DIR=\"${CMAKE_SOURCE_DIR}/src/app\"")
else()
  add_definitions("-DN_DEBUG")
  add_definitions("-DBUILTIN_HOOKS_DIR=\"${INSTALLER_SHARE_DIR}/hooks\"")
  add_definitions("-DBUILTIN_CHECK_HOOKS_DIR=\"${INSTALLER_SHARE_DIR}/check_hooks\"")
  add_definitions("-DI18N_DIR=\"${INSTALLER_SHARE_DIR}/i18n\"")
  add_definitions("-DRESOURCES_DIR=\"${INSTALLER_SHARE_DIR}/resources\"")
  add_definitions("-DTOOLS_DIR=\"${INSTALLER_SHARE_DIR}/tools\"")
  add_definitions("-DDATAS_DIR=\"/usr/bin/\"")
  add_definitions("-DXRANDR_DIR=\"/usr/bin\"")
endif()

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5X11Extras REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5LinguistTools)
find_package(Qt5Svg REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(QTermWidget5 REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

pkg_search_module(Parted REQUIRED libparted)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(X11EXT REQUIRED xext)
pkg_search_module(X11TST REQUIRED xtst)
pkg_search_module(X11RandR REQUIRED xrandr)
pkg_search_module(UDisk2 REQUIRED udisks2-qt5)
#pkg_search_module(ncursesw REQUIRED ncurses++w)
#pkg_search_module(pwquality REQUIRED pwquality)

include_directories(AFTER ${Parted_INCLUDE_DIRS})
include_directories(AFTER ${X11_INCLUDE_DIRS})
include_directories(AFTER ${X11EXT_INCLUDE_DIRS})
include_directories(AFTER ${X11TST_INCLUDE_DIRS})
include_directories(AFTER ${X11RandR_INCLUDE_DIRS})
include_directories(AFTER ${UDisk2_INCLUDE_DIRS})

add_subdirectory(third_party/global_shortcut)
add_subdirectory(third_party/qrencode)
add_subdirectory(third_party/timezones)
add_subdirectory(third_party/zlib_wrapper)

configure_file(base/consts.h base/consts.h @ONLY)

# KF5 network manager
get_target_property(KF5_QT_INCLUDE_DIRS KF5::NetworkManagerQt INTERFACE_INCLUDE_DIRECTORIES)

set(TRY_KF5_LIBRARIES
IMPORTED_LOCATION_DEBIAN
IMPORTED_LOCATION_NOCONFIG
)

foreach(LETTER ${TRY_KF5_LIBRARIES})
    if (NOT KF5_QT_LIBRARIES)
        get_target_property(KF5_QT_LIBRARIES KF5::NetworkManagerQt ${LETTER})
    endif()
endforeach()

if (NOT KF5_QT_LIBRARIES)
    find_library(KF5_QT_LIBRARIES libKF5NetworkManagerQt.so)
endif()
# end KF5 network manager

set(BASE_FILES
    base/command.cpp
    base/command.h
    base/consts.h
    base/file_util.cpp
    base/file_util.h
    base/string_util.cpp
    base/string_util.h
    base/thread_util.cpp
    base/thread_util.h
    base/translator.h
    base/auto_screen_scale.h
    base/utils.h
    )

set(OEM_FILES

    oem/models/oem_settings_item.cpp
    oem/models/oem_settings_item.h
    oem/models/oem_settings_model.cpp
    oem/models/oem_settings_model.h
    oem/views/oem_settings_item_view.cpp
    oem/views/oem_settings_item_view.h

    oem/styles/styles.qrc
    )

set(PARTMAN_FILES
    partman/device.cpp
    partman/device.h
    partman/vg_device.cpp
    partman/vg_device.h
    partman/fs.cpp
    partman/fs.h
    partman/libparted_util.cpp
    partman/libparted_util.h
    partman/operation.cpp
    partman/operation.h
    partman/lvm_operation.cpp
    partman/lvm_operation.h
    partman/os_prober.cpp
    partman/os_prober.h
    partman/partition.cpp
    partman/partition.h
    partman/lv_partition.cpp
    partman/lv_partition.h
    partman/partition_format.cpp
    partman/partition_format.h
    partman/partition_manager.cpp
    partman/partition_manager.h
    partman/partition_usage.cpp
    partman/partition_usage.h
    partman/structs.cpp
    partman/structs.h
    partman/utils.cpp
    partman/utils.h
    )

set(SERVICE_FILES
    service/backend/chroot.cpp
    service/backend/chroot.h
    service/backend/geoip_request_worker.cpp
    service/backend/geoip_request_worker.h
    service/backend/hooks_pack.cpp
    service/backend/hooks_pack.h
    service/backend/hook_worker.cpp
    service/backend/hook_worker.h
    service/backend/wifi_inspect_worker.cpp
    service/backend/wifi_inspect_worker.h

    service/first_boot_hook_worker.cpp
    service/first_boot_hook_worker.h
    service/hooks_manager.cpp
    service/hooks_manager.h
    service/log_manager.cpp
    service/log_manager.h
    service/power_manager.cpp
    service/power_manager.h
    service/process_util.cpp
    service/process_util.h
    service/screen_brightness.cpp
    service/screen_brightness.h
    service/settings_manager.cpp
    service/settings_manager.h
    service/system_language.cpp
    service/system_language.h
    service/settings_name.h
    service/timezone_manager.cpp
    service/timezone_manager.h
    service/language_manager.h
    service/language_manager.cpp
    service/screen_adaptation_manager.h
    service/screen_adaptation_manager.cpp
    )

set(SYSINFO_FILES
    sysinfo/dev_disk.cpp
    sysinfo/dev_disk.h
    sysinfo/iso3166.cpp
    sysinfo/iso3166.h
    sysinfo/keyboard.cpp
    sysinfo/keyboard.h
    sysinfo/machine.cpp
    sysinfo/machine.h
    sysinfo/proc_meminfo.cpp
    sysinfo/proc_meminfo.h
    sysinfo/proc_mounts.cpp
    sysinfo/proc_mounts.h
    sysinfo/proc_partitions.cpp
    sysinfo/proc_partitions.h
    sysinfo/proc_swaps.cpp
    sysinfo/proc_swaps.h
    sysinfo/release_version.cpp
    sysinfo/release_version.h
    sysinfo/timezone.cpp
    sysinfo/timezone.h
    sysinfo/users.cpp
    sysinfo/users.h
    sysinfo/validate_hostname.cpp
    sysinfo/validate_hostname.h
    sysinfo/validate_password.cpp
    sysinfo/validate_password.h
    sysinfo/validate_username.cpp
    sysinfo/validate_username.h
    sysinfo/virtual_machine.cpp
    sysinfo/virtual_machine.h)

set(UI_FILES

    ui/delegates/advanced_partition_animations.cpp
    ui/delegates/advanced_partition_animations.h
    ui/delegates/advanced_partition_delegate.cpp
    ui/delegates/advanced_partition_delegate.h
    ui/delegates/lvm_partition_delegate.cpp
    ui/delegates/lvm_partition_delegate.h
    ui/delegates/partition_delegate.h
    ui/delegates/partition_delegate.cpp
    ui/delegates/network_operate.h
    ui/delegates/network_operate.cpp
    ui/delegates/advanced_validate_state.h
    ui/delegates/advanced_validate_state.cpp
    ui/delegates/avatar_list_delegate.cpp
    ui/delegates/avatar_list_delegate.h
    ui/delegates/frameless_list_delegate.cpp
    ui/delegates/frameless_list_delegate.h
    ui/delegates/full_disk_delegate.cpp
    ui/delegates/full_disk_delegate.h
    ui/delegates/installer_args_parser.cpp
    ui/delegates/installer_args_parser.h
    ui/delegates/install_slide_frame_util.cpp
    ui/delegates/install_slide_frame_util.h
    ui/delegates/language_delegate.cpp
    ui/delegates/language_delegate.h
    ui/delegates/main_window_util.cpp
    ui/delegates/main_window_util.h
    ui/delegates/partition_util.cpp
    ui/delegates/partition_util.h
    ui/delegates/style_delegate.h
    ui/delegates/style_delegate.cpp
    ui/delegates/license_delegate.cpp
    ui/delegates/license_delegate.h

    ui/delegates/popup_menu_delegate.cpp
    ui/delegates/popup_menu_delegate.h
    ui/delegates/simple_partition_delegate.cpp
    ui/delegates/simple_partition_delegate.h
    ui/delegates/timezone_map_util.cpp
    ui/delegates/timezone_map_util.h
    ui/delegates/componentinstallmanager.h
    ui/delegates/componentinstallmanager.cpp
    ui/delegates/user_agreement_delegate.h
    ui/delegates/user_agreement_delegate.cpp
    ui/delegates/disk_installation_delegate.h
    ui/delegates/disk_installation_delegate.cpp
    ui/delegates/disk_installation_detail_delegate.h
    ui/delegates/disk_installation_detail_delegate.cpp
    ui/interfaces/frameproxyinterface.h
    ui/interfaces/frameinterface.h
    ui/interfaces/frameinterfaceprivate.h
    ui/frames/consts.h
    ui/frames/inner/advanced_partition_frame.cpp
    ui/frames/inner/advanced_partition_frame.h
    ui/frames/inner/lvm_partition_frame.cpp
    ui/frames/inner/lvm_partition_frame.h
    ui/frames/inner/edit_partition_frame.cpp
    ui/frames/inner/edit_partition_frame.h
    ui/frames/inner/full_disk_frame.cpp
    ui/frames/inner/full_disk_frame.h

    ui/frames/inner/full_disk_encrypt_frame.h
    ui/frames/inner/full_disk_encrypt_frame.cpp
    ui/frames/inner/install_progress_slide_frame.cpp
    ui/frames/inner/install_progress_slide_frame.h
    ui/frames/inner/new_partition_frame.cpp
    ui/frames/inner/new_partition_frame.h
    ui/frames/inner/new_table_loading_frame.cpp
    ui/frames/inner/new_table_loading_frame.h
    ui/frames/inner/new_table_warning_frame.cpp
    ui/frames/inner/new_table_warning_frame.h
    ui/frames/inner/partition_loading_frame.cpp
    ui/frames/inner/partition_loading_frame.h
    ui/frames/inner/partition_number_limitation_frame.cpp
    ui/frames/inner/partition_number_limitation_frame.h
    ui/frames/inner/partition_table_warning_frame.cpp
    ui/frames/inner/partition_table_warning_frame.h

    ui/frames/inner/prepare_install_frame.cpp
    ui/frames/inner/prepare_install_frame.h
    ui/frames/inner/select_bootloader_frame.cpp
    ui/frames/inner/select_bootloader_frame.h
    ui/frames/inner/select_language_frame.cpp
    ui/frames/inner/select_language_frame.h
    ui/frames/inner/simple_partition_frame.cpp
    ui/frames/inner/simple_partition_frame.h
    ui/frames/inner/system_info_avatar_frame.cpp
    ui/frames/inner/system_info_avatar_frame.h
    ui/frames/inner/system_info_form_frame.cpp
    ui/frames/inner/system_info_form_frame.h
    ui/frames/inner/system_info_keyboard_frame.cpp
    ui/frames/inner/system_info_keyboard_frame.h
    ui/frames/inner/user_agreement_frame.h
    ui/frames/inner/user_agreement_frame.cpp
    ui/frames/inner/systemdateframe.h
    ui/frames/inner/systemdateframe.cpp
    ui/frames/inner/select_time_zone_frame.h
    ui/frames/inner/select_time_zone_frame.cpp
    ui/frames/inner/install_log_frame.h
    ui/frames/inner/install_log_frame.cpp
    ui/frames/timezone_frame.cpp
    ui/frames/timezone_frame.h

    ui/frames/confirm_quit_frame.cpp
    ui/frames/confirm_quit_frame.h
    ui/frames/swap_warnning_frame.cpp
    ui/frames/swap_warnning_frame.h
    ui/frames/control_panel_frame.cpp
    ui/frames/control_panel_frame.h
    ui/frames/disk_space_insufficient_frame.cpp
    ui/frames/disk_space_insufficient_frame.h
    ui/frames/first_boot_loading_frame.cpp
    ui/frames/first_boot_loading_frame.h
    ui/frames/install_failed_frame.cpp
    ui/frames/install_failed_frame.h
    ui/frames/install_progress_frame.cpp
    ui/frames/install_progress_frame.h
    ui/frames/install_success_frame.cpp
    ui/frames/install_success_frame.h
    ui/frames/partition_frame.cpp
    ui/frames/partition_frame.h
    ui/frames/privilege_error_frame.cpp
    ui/frames/privilege_error_frame.h
    ui/frames/language_frame.h
    ui/frames/language_frame.cpp
    ui/frames/system_info_frame.cpp
    ui/frames/system_info_frame.h
    ui/frames/virtual_machine_frame.cpp
    ui/frames/virtual_machine_frame.h
    ui/frames/networkframe.h
    ui/frames/networkframe.cpp
    ui/frames/control_platform_frame.h
    ui/frames/control_platform_frame.cpp
    ui/frames/dynamic_disk_warning_frame.h
    ui/frames/dynamic_disk_warning_frame.cpp
    ui/frames/saveinstallfailedlogframe.h
    ui/frames/saveinstallfailedlogframe.cpp
    ui/frames/install_component_frame.h
    ui/frames/install_component_frame.cpp
    ui/frames/install_results_frame.h
    ui/frames/install_results_frame.cpp
    ui/frames/repair_system_frame.h
    ui/frames/repair_system_frame.cpp

    ui/models/bootloader_list_model.cpp
    ui/models/bootloader_list_model.h
    ui/models/fs_model.cpp
    ui/models/fs_model.h
    ui/models/keyboard_layout_model.cpp
    ui/models/keyboard_layout_model.h
    ui/models/keyboard_layout_variant_model.cpp
    ui/models/keyboard_layout_variant_model.h
    ui/models/language_list_model.cpp
    ui/models/language_list_model.h
    ui/models/mount_point_model.cpp
    ui/models/mount_point_model.h
    ui/models/partition_model.cpp
    ui/models/partition_model.h
    ui/models/partition_type_model.cpp
    ui/models/partition_type_model.h
    ui/models/control_platform_region_model.h
    ui/models/control_platform_region_model.cpp
    ui/models/disk_installation_detail_model.h
    ui/models/disk_installation_detail_model.cpp
    ui/models/continent_model.h
    ui/models/continent_model.cpp

    ui/utils/widget_util.cpp
    ui/utils/widget_util.h
    ui/utils/xutil.cpp
    ui/utils/xutil.h
    ui/utils/keyboardmonitor.h
    ui/utils/keyboardmonitor.cpp
    ui/utils/mouse_shape.h
    ui/utils/mouse_shape.cpp

    ui/views/frameless_list_view.cpp
    ui/views/frameless_list_view.h
    ui/views/pointer_list_view.cpp
    ui/views/pointer_list_view.h
    ui/views/disk_installation_view.h
    ui/views/disk_installation_view.cpp
    ui/views/disk_installation_detail_view.h
    ui/views/disk_installation_detail_view.cpp

    ui/widgets/advanced_partition_button.cpp
    ui/widgets/advanced_partition_button.h
    ui/widgets/advanced_partition_error_label.cpp
    ui/widgets/advanced_partition_error_label.h
    ui/widgets/avatar_button.cpp
    ui/widgets/avatar_button.h
    ui/widgets/comment_label.cpp
    ui/widgets/comment_label.h
    ui/widgets/device_model_label.cpp
    ui/widgets/device_model_label.h
    ui/widgets/expanded_nav_button.cpp
    ui/widgets/expanded_nav_button.h

    ui/widgets/flat_button.cpp
    ui/widgets/flat_button.h
    ui/widgets/full_disk_partition_colorbar.cpp
    ui/widgets/full_disk_partition_colorbar.h
    ui/widgets/partition_color_label.cpp
    ui/widgets/partition_color_label.h
    ui/widgets/icon_button.cpp
    ui/widgets/icon_button.h
    ui/widgets/line_edit.cpp
    ui/widgets/line_edit.h
    ui/widgets/nav_button.cpp
    ui/widgets/nav_button.h
    ui/widgets/partition_size_slider.cpp
    ui/widgets/partition_size_slider.h
    ui/widgets/shadow_widget.h
    ui/widgets/shadow_widget.cpp
    ui/widgets/pointer_button.cpp
    ui/widgets/pointer_button.h
    ui/widgets/popup_menu.cpp

    ui/widgets/popup_menu.h
    ui/widgets/qr_widget.cpp
    ui/widgets/qr_widget.h
    ui/widgets/rounded_progress_bar.cpp
    ui/widgets/rounded_progress_bar.h
    ui/widgets/simple_disk_button.cpp
    ui/widgets/simple_disk_button.h
    ui/widgets/simple_partition_button.cpp
    ui/widgets/simple_partition_button.h
    ui/widgets/spinner_label.cpp
    ui/widgets/spinner_label.h
    ui/widgets/system_info_tip.cpp
    ui/widgets/system_info_tip.h
    ui/widgets/table_combo_box.cpp
    ui/widgets/table_combo_box.h

    ui/widgets/timezone_map.cpp
    ui/widgets/timezone_map.h
    ui/widgets/title_label.cpp
    ui/widgets/title_label.h
    ui/widgets/tooltip_pin.cpp
    ui/widgets/tooltip_pin.h
    ui/widgets/wallpaper_item.cpp
    ui/widgets/wallpaper_item.h
    ui/widgets/component_widget.h
    ui/widgets/component_widget.cpp
    ui/widgets/partition_table_widget.h
    ui/widgets/partition_table_widget.cpp
    ui/widgets/network_device_widget.h
    ui/widgets/network_device_widget.cpp
    ui/widgets/di_scrollarea.h
    ui/widgets/di_scrollarea.cpp
    ui/widgets/multiple_disk_installation_widget.h
    ui/widgets/multiple_disk_installation_widget.cpp
    ui/widgets/time_date_line_edit.h
    ui/widgets/time_date_line_edit.cpp
    ui/widgets/auto_wrap_label.h
    ui/widgets/auto_wrap_label.cpp
    ui/widgets/ddropdown.h
    ui/widgets/ddropdown.cpp
    ui/widgets/auto_elide_label.h
    ui/widgets/auto_elide_label.cpp

    ui/widgets/link_button.h
    ui/widgets/link_button.cpp
    ui/widgets/operator_widget.h
    ui/widgets/operator_widget.cpp
    ui/widgets/ticker_label.cpp
    ui/widgets/ticker_label.h
    ui/widgets/select_button.cpp
    ui/widgets/select_button.h
    ui/widgets/caps_lock_line_edit.cpp
    ui/widgets/caps_lock_line_edit.h
    ui/widgets/tip_list_view.h
    ui/widgets/tip_list_view.cpp

    ui/xrandr/multi_head_manager.cpp
    ui/xrandr/multi_head_manager.h

    ui/xrandr/multi_head_worker.cpp
    ui/xrandr/multi_head_worker.h

    ui/xrandr/xrandr_wrapper.cpp
    ui/xrandr/xrandr_wrapper.h

    ui/images/images.qrc
    ui/styles/styles.qrc
    license/license.qrc

    # Required by SpinnerLabel.
    ui/widgets/spinner/spinner.qrc
    )

set(CLI_UI_FILES
    license/license.qrc
    ui/interfaces_cli/frameinterface.h
    ui/interfaces_cli/frameinterface.cpp
    ui/interfaces_cli/frameinterfaceprivate.h
    ui/delegates/installer_args_parser.h
    ui/delegates/installer_args_parser.cpp
    ui/delegates/componentinstallmanager.h
    ui/delegates/componentinstallmanager.cpp
    ui/delegates/language_delegate.cpp
    ui/delegates/language_delegate.h
    ui/delegates/full_disk_delegate.cpp
    ui/delegates/full_disk_delegate.h
    ui/delegates/partition_delegate.h
    ui/delegates/partition_delegate.cpp
    ui/delegates/partition_util.cpp
    ui/delegates/partition_util.h
    ui/delegates/advanced_partition_animations.cpp
    ui/delegates/advanced_partition_animations.h
    ui/delegates/advanced_partition_delegate.cpp
    ui/delegates/advanced_partition_delegate.h
    ui/delegates/lvm_partition_delegate.cpp
    ui/delegates/lvm_partition_delegate.h
    ui/delegates/partition_delegate.h
    ui/delegates/partition_delegate.cpp
    ui/delegates/network_operate.h
    ui/delegates/network_operate.cpp
    ui/delegates/advanced_validate_state.h
    ui/delegates/advanced_validate_state.cpp
    ui/delegates/license_delegate.cpp
    ui/delegates/license_delegate.h

    ui/models/partition_model.cpp
    ui/models/partition_model.h
    ui/frames_cli/inner/full_disk_frame.h
    ui/frames_cli/inner/full_disk_frame.cpp
    ui/frames_cli/inner/advanced_partition_frame.h
    ui/frames_cli/inner/advanced_partition_frame.cpp
    ui/frames_cli/inner/partition_table_warning_frame.h
    ui/frames_cli/inner/partition_table_warning_frame.cpp
    ui/frames_cli/inner/partition_number_limitation_frame.h
    ui/frames_cli/inner/partition_number_limitation_frame.cpp
    ui/frames_cli/inner/new_partition_frame.h
    ui/frames_cli/inner/new_partition_frame.cpp
    ui/frames_cli/inner/prepare_install_frame.h
    ui/frames_cli/inner/prepare_install_frame.cpp
    ui/frames_cli/inner/create_root_user_frame.h
    ui/frames_cli/inner/create_root_user_frame.cpp
    ui/frames_cli/inner/savelog_frame.h
    ui/frames_cli/inner/savelog_frame.cpp
    ui/frames_cli/language_frame.h
    ui/frames_cli/language_frame.cpp
    ui/frames_cli/privilege_error_frame.h
    ui/frames_cli/privilege_error_frame.cpp
    ui/frames_cli/disk_space_insufficient_frame.h
    ui/frames_cli/disk_space_insufficient_frame.cpp
    ui/frames_cli/system_info_frame.h
    ui/frames_cli/system_info_frame.cpp
    ui/frames_cli/install_component_frame.h
    ui/frames_cli/install_component_frame.cpp
    ui/frames_cli/install_process_frame.h
    ui/frames_cli/install_process_frame.cpp
    ui/frames_cli/install_success_frame.h
    ui/frames_cli/install_success_frame.cpp
    ui/frames_cli/network_frame.h
    ui/frames_cli/network_frame.cpp
    ui/frames_cli/partition_frame.h
    ui/frames_cli/partition_frame.cpp
    ui/frames_cli/time_zone_frame.h
    ui/frames_cli/time_zone_frame.cpp
    ui/frames_cli/virtual_machine_frame.h
    ui/frames_cli/virtual_machine_frame.cpp
    ui/frames_cli/licence_frame.h
    ui/frames_cli/licence_frame.cpp
    ui/frames_cli/keyboard_frame.h
    ui/frames_cli/keyboard_frame.cpp
    ui/models_cli/partition_type_model.h
    ui/models_cli/partition_type_model.cpp
    ui/ncurses_widgets/ncurses_util.h
    ui/ncurses_widgets/ncurses_util.cpp
    ui/ncurses_widgets/ncurses_button.h
    ui/ncurses_widgets/ncurses_button.cpp
    ui/ncurses_widgets/ncurses_windows_base.h
    ui/ncurses_widgets/ncurses_windows_base.cpp
    ui/ncurses_widgets/ncurses_label.h
    ui/ncurses_widgets/ncurses_label.cpp
    ui/ncurses_widgets/ncurses_text_brower.h
    ui/ncurses_widgets/ncurses_text_brower.cpp
    ui/ncurses_widgets/ncurses_list_view.h
    ui/ncurses_widgets/ncurses_list_view.cpp
    ui/ncurses_widgets/ncurses_line_edit.h
    ui/ncurses_widgets/ncurses_line_edit.cpp
    ui/ncurses_widgets/ncurses_checkbox.h
    ui/ncurses_widgets/ncurses_checkbox.cpp
    ui/ncurses_widgets/ncurses_progressbar.h
    ui/ncurses_widgets/ncurses_progressbar.cpp
    ui/ncurses_widgets/ncurses_checkbox_list.h
    ui/ncurses_widgets/ncurses_checkbox_list.cpp
    ui/ncurses_widgets/operator_widget.h
    ui/ncurses_widgets/operator_widget.cpp
    ui/ncurses_widgets/ncurses_quit.h
    ui/ncurses_widgets/ncurses_quit.cpp
    )


set(ROOT_UNITTEST_FILES
    #    partman/operation_test.cpp
    #    partman/partition_usage_test.cpp
    #    partman/os_prober_test.cpp
    #    partman/partition_manager_test.cpp
    partman/libparted_util_test.cpp
    )

set(UNITTEST_FILES
    base/command_test.cpp
    base/file_util_test.cpp
    base/string_util_test.cpp

    partman/operation_test.cpp
    partman/partition_test.cpp

    sysinfo/dev_disk_test.cpp
    sysinfo/iso3166_test.cpp
    sysinfo/keyboard_test.cpp
    sysinfo/proc_meminfo_test.cpp
    sysinfo/proc_mounts_test.cpp
    sysinfo/proc_partitions_test.cpp
    sysinfo/proc_swaps_test.cpp
    sysinfo/timezone_test.cpp
    sysinfo/users_test.cpp
    sysinfo/validate_hostname_test.cpp
    sysinfo/validate_password_test.cpp
    sysinfo/validate_username_test.cpp

    ui/delegates/installer_args_parser_test.cpp
    ui/delegates/install_slide_frame_util_test.cpp
    ui/delegates/timezone_map_util_test.cpp

    tests/component_install_test.cpp
    tests/scalefactor_test.cpp
    )

set(QtCore_LIBS Qt5::Core)
set(Qt_LIBS
    Qt5::Gui
    Qt5::Network
    Qt5::Widgets
    Qt5::X11Extras
    Qt5::Xml
    Qt5::Svg
    Qt5::DBus
    )

set(LINK_LIBS
    ${Qt_LIBS}
    ${Parted_LIBRARIES}
    ${X11_LIBRARIES}
    ${X11EXT_LIBRARIES}
    ${X11TST_LIBRARIES}
    ${X11RandR_LIBRARIES}
    ${UDisk2_LIBRARIES}
    ${DtkWidget_LIBRARIES}

    global_shortcut
    qrencode
    zlib_wrapper
    Xi
    deepin_pw_check
    )

# Main executable
# Generate .qm files from .ts files.
file(GLOB INSTALLER_TRANSLATION_TS
     ${CMAKE_SOURCE_DIR}/i18n/deepin-installer*.ts)
set_source_files_properties(${INSTALLER_TRANSLATION_TS}
                            PROPERTIES OUTPUT_LOCATION ${CMAKE_BINARY_DIR}/i18n)
qt5_add_translation(INSTALLER_TRANSLATION_QM ${INSTALLER_TRANSLATION_TS})
add_custom_target(installer-qm-update DEPENDS ${INSTALLER_TRANSLATION_QM})

add_executable(deepin-installer
               app/deepin_installer.cpp
               ui/main_window.cpp
               ui/main_window.h

               service/password_manager.h
               service/password_manager.cpp

               ${BASE_FILES}
               ${PARTMAN_FILES}
               ${SERVICE_FILES}
               ${SYSINFO_FILES}
               ${UI_FILES}
               )

target_include_directories(deepin-installer PUBLIC
               ${QTERMWIDGET_INCLUDE_DIRS}
               ${KF5_QT_INCLUDE_DIRS}
               #${pwquality_INCLUDE_DIRS}
)

target_link_libraries(deepin-installer ${LINK_LIBS} qtermwidget5 ${KF5_QT_LIBRARIES}
    #pwquality
    )
add_dependencies(deepin-installer installer-qm-update)


add_executable(deepin-installer-cli
              app/deepin_install_cli.cpp
              ui/main_window_cli.h
              ui/main_window_cli.cpp

              service/password_manager.h
              service/password_manager.cpp

              ${CLI_UI_FILES}
              ${BASE_FILES}
              ${PARTMAN_FILES}
              ${SERVICE_FILES}
              ${SYSINFO_FILES}
              )

target_include_directories(deepin-installer-cli PUBLIC
               ${KF5_QT_INCLUDE_DIRS}
)

set_property(TARGET deepin-installer-cli PROPERTY LINK_FLAGS "-no-pie")
target_link_libraries(deepin-installer-cli ${LINK_LIBS} ncurses++w ncursesw panelw ${KF5_QT_LIBRARIES}
    #pwquality
    )
add_dependencies(deepin-installer-cli installer-qm-update)


# Set/get ini files.
add_executable(deepin-installer-settings
               app/deepin_installer_settings.cpp)

target_link_libraries(deepin-installer-settings ${QtCore_LIBS})

add_executable(deepin-installer-simpleini
               app/deepin_installer_simpleini.cpp
               sysinfo/csettings.h
               sysinfo/csettings.cpp)

target_link_libraries(deepin-installer-simpleini ${QtCore_LIBS})

# Reboot setup
add_executable(deepin-installer-first-boot
               app/deepin_installer_first_boot.cpp
               ui/first_boot_setup_window.cpp
               ui/first_boot_setup_window.h

               ui/models/package_manager_model.h
               ui/models/package_manager_model.cpp

               service/password_manager.h
               service/password_manager.cpp

               ${BASE_FILES}
               ${PARTMAN_FILES}
               ${SERVICE_FILES}
               ${SYSINFO_FILES}
               ${UI_FILES}
               )
target_include_directories(deepin-installer-first-boot PUBLIC
               ${QTERMWIDGET_INCLUDE_DIRS}
               ${KF5_QT_INCLUDE_DIRS}
               #${pwquality_INCLUDE_DIRS}
)

target_link_libraries(deepin-installer-first-boot
                      ${LINK_LIBS}
                      qtermwidget5
                      ${KF5_QT_LIBRARIES}
                      #pwquality
                      )

# 用户处理安装器初始，目前主要是和plymouth界面配置，
# 用于设置启动背景, 防止高分辨率安装器启动慢的情况下，plymouth歪屏
# 将设置背景的模块分离出来，主要是防止安装器主界面启动时间是受后续业务逻辑修改而导致的不确定性
add_executable(deepin-installer-init
    app/deepin_installer_init.cpp

    ui/widgets/wallpaper_item.h
    ui/widgets/wallpaper_item.cpp

    service/settings_manager.h
    service/settings_manager.cpp

    base/command.h
    base/command.cpp
)
target_link_libraries(deepin-installer-init ${LINK_LIBS})

## Installation
install(PROGRAMS
        ${CMAKE_CURRENT_SOURCE_DIR}/app/deepin-installer-first-boot-pkexec
        ${CMAKE_CURRENT_SOURCE_DIR}/app/deepin-installer-pkexec
        ${CMAKE_CURRENT_SOURCE_DIR}/app/deepin-installer-both
        ${CMAKE_CURRENT_SOURCE_DIR}/app/deepin-installer-bases
        ${CMAKE_CURRENT_SOURCE_DIR}/app/deepin-installer-cli-install
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        )

install(TARGETS
        deepin-installer
        deepin-installer-cli
        deepin-installer-first-boot    
        deepin-installer-settings
        deepin-installer-simpleini
        deepin-installer-init
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        )

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/i18n
    DESTINATION ${INSTALLER_SHARE_DIR}
    FILES_MATCHING PATTERN "*"
)
