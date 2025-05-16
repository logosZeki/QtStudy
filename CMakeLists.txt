cmake_minimum_required(VERSION 3.10)  #cmake版本最低3.10

#SET(CMAKE_TOOLCHAIN_FILE  "H:/Code/vs_C++/C++/build/thirdparty_install/vcpkg/scripts/buildsystems/vcpkg.cmake")

# 获取当前子CMake文件所在目录的完整路径 
get_filename_component(CURRENT_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE) 
# 从完整路径中提取目录名称
get_filename_component(CURRENT_DIR_NAME "${CURRENT_DIR_PATH}" NAME) 
project(${CURRENT_DIR_NAME})

set(CMAKE_CXX_STANDARD 11)  

if(MSVC)
	add_compile_options(/Zc:__cplusplus)
endif()

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets Gui Svg LinguistTools Xml REQUIRED)

file(GLOB UI_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")
file(GLOB RCC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*qrc")
file(GLOB HEADER_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h" 
    "${CMAKE_CURRENT_BINARY_DIR}/*.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/chart/*.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/util/*.h"
)
file(GLOB CPP_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/chart/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/util/*.cpp" 
)

# 显式添加页面设置对话框文件
list(APPEND HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/pagesettingdialog.h")
list(APPEND CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/pagesettingdialog.cpp")

qt5_wrap_ui(UI_HEADERS ${UI_FILES})

source_group("Form Files" FILES ${UI_FILES})
source_group("Resource Files" FILES ${RCC_FILES})



add_executable(${PROJECT_NAME} WIN32 ${HEADER_FILES} ${CPP_FILES} ${UI_FILES} ${RCC_FILES} )


target_link_libraries(${PROJECT_NAME} 
	Qt5::Widgets
	Qt5::Core
	Qt5::Gui
	Qt5::Svg
	Qt5::Xml
)
