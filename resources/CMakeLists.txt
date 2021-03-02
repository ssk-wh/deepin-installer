# Install resources/ folder, excluding desktop file and icons.

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/default_wallpaper.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/languages.json
    ${CMAKE_CURRENT_SOURCE_DIR}/oem_settings.json
    ${CMAKE_CURRENT_SOURCE_DIR}/reserved_usernames
    ${CMAKE_CURRENT_SOURCE_DIR}/timezone_alias
    ${CMAKE_CURRENT_SOURCE_DIR}/packages_default.json
    ${CMAKE_CURRENT_SOURCE_DIR}/packages_choice.json
    ${CMAKE_CURRENT_SOURCE_DIR}/packages_sort.json
    ${CMAKE_CURRENT_SOURCE_DIR}/language_sort.json
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources
)

# Install PolicyKit file.
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.pkexec.installer.policy
    ${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.pkexec.installer-first-boot.policy
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions/
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/deepin-installer-oem.desktop
    ${CMAKE_CURRENT_SOURCE_DIR}/deepin-installer.desktop
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/deepin-installer-oem.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/deepin-installer.svg
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps
)


# Install default_settings.ini based on current architecture.
set(INSTALLER_BUILD_ARCH "")
execute_process(COMMAND uname -m
                OUTPUT_VARIABLE INSTALLER_BUILD_ARCH)
message("build arch: ${INSTALLER_BUILD_ARCH}")

if (${INSTALLER_BUILD_ARCH} MATCHES "x86")
    file(GLOB OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/platform_x86/*)
  install(FILES ${OVERRIDE}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/override)
elseif(${INSTALLER_BUILD_ARCH} MATCHES "sw")
    file(GLOB OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/platform_sw/*)
    install(FILES ${OVERRIDE}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/override)
elseif(${INSTALLER_BUILD_ARCH} MATCHES "mips")
    file(GLOB OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/platform_loongson/*)
    install(FILES ${OVERRIDE}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/override)
elseif(${INSTALLER_BUILD_ARCH} MATCHES "loongarch64")
    file(GLOB OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/platform_loongarch64/*)
    install(FILES ${OVERRIDE}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/override)
elseif (${INSTALLER_BUILD_ARCH} MATCHES "aarch64")
    file(GLOB OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/platform_arm/*)
    install(FILES ${OVERRIDE}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/override)
else()
  message(FATAL_ERROR "Not supported architecture: ${INSTALLER_BUILD_ARCH}")
endif ()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/default_settings.ini
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources)

file(GLOB FULL_DISK_POLICY ${CMAKE_CURRENT_SOURCE_DIR}/full_disk_policy*.json)

install(FILES ${FULL_DISK_POLICY}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources)

add_subdirectory(slide)
