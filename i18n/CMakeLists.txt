# 生成翻译
## 设置QT语言cmake模块
find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)

## 通过源代码生成ts文件
set(CREATE_TS_DIR
    ${CMAKE_SOURCE_DIR}/src/partman
    ${CMAKE_SOURCE_DIR}/src/base
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/service
)
find_program(LUPDATE_EXECUTABLE lupdate)
execute_process(
    COMMAND ${LUPDATE_EXECUTABLE} -source-language en
    -no-obsolete
    -recursive ${CREATE_TS_DIR}
    -ts ${CMAKE_SOURCE_DIR}/i18n/deepin-installer.ts)

## 获取ts源文件
file(GLOB INSTALLER_TRANSLATION_TS
    ${CMAKE_SOURCE_DIR}/i18n/deepin-installer*.ts)

## 将ts源文件显示在qtcreate中
add_custom_target(ts SOURCES ${INSTALLER_TRANSLATION_TS})




