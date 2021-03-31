find_package(PkgConfig REQUIRED)
find_package(DtkCMake REQUIRED)

## 后续可以通过平台来安装slide资源来裁剪安装器的deb大小
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/deepin-installer/resources/slide
  PATTERN "CMakeLists.txt" EXCLUDE
)
