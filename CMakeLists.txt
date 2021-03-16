cmake_minimum_required(VERSION 3.1)
project(deepin-installer C CXX)

## 这两行必须放置在根的CMakeLists文件中，否则ctest无法正确找到文件
# 启用CTest检查
include(Dart)
# 启用CTest
#include(CTest)

# Import folders
add_subdirectory(i18n)
add_subdirectory(data)
add_subdirectory(docs)
add_subdirectory(hooks)
add_subdirectory(check_hooks)
add_subdirectory(resources)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)


