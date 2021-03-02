find_package(PkgConfig REQUIRED)
find_package(DtkCMake REQUIRED)

if (${INSTALLER_BUILD_ARCH} MATCHES "x86")
  if (OS_DEEPIN_PROFESSIONAL)
    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
      DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/slide
      PATTERN "CMakeLists.txt" EXCLUDE
    )
  else()
    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
      DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/slide
      PATTERN "CMakeLists.txt" EXCLUDE
    )
  endif()
elseif(${INSTALLER_BUILD_ARCH} MATCHES "sw")
  install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
      DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/slide
      PATTERN "CMakeLists.txt" EXCLUDE
  )
elseif(${INSTALLER_BUILD_ARCH} MATCHES "mips")
  install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
      DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/slide
      PATTERN "CMakeLists.txt" EXCLUDE
  )
elseif(${INSTALLER_BUILD_ARCH} MATCHES "aarch64")
  install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
      DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/slide
      PATTERN "CMakeLists.txt" EXCLUDE
  )
elseif(${INSTALLER_BUILD_ARCH} MATCHES "loongarch64")
  install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
      DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/slide
      PATTERN "CMakeLists.txt" EXCLUDE
  )
else()
  message(FATAL_ERROR "Not supportted architecture: ${INSTALLER_BUILD_ARCH}")
endif ()
