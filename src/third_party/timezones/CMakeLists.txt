find_package(Gettext REQUIRED)

## 获取当前目录下的子目录
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()
SUBDIRLIST(DIRLIST ${CMAKE_CURRENT_SOURCE_DIR})

## 动态转换po文件为mo文件，以后增加po文件的目录后，不用再修改工程代码
FOREACH(subdir ${DIRLIST})
    set(PO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}")
    if(NOT EXISTS "${PO_PATH}/CMakeLists.txt")
        file(WRITE ${PO_PATH}/CMakeLists.txt "gettext_process_po_files(${subdir} ALL INSTALL_DESTINATION share/locale PO_FILES \${CMAKE_CURRENT_SOURCE_DIR}/deepin-installer-timezones.po)")
    endif()
    add_subdirectory(${subdir})
ENDFOREACH()
