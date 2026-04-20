cmake_minimum_required(VERSION 3.16)
project(Chalk)

set(TARGET_NAME "Chalk")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external/xdf)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
  Widgets
  WebEngineWidgets
  WebChannel
)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
  include/*.hpp
  include/*.h
  src/*.cpp
)

qt_add_executable(${TARGET_NAME}
  ${PROJECT_SOURCES}
  resources.qrc
)

if (WIN32)
  set_target_properties(${TARGET_NAME} PROPERTIES WIN32_EXECUTABLE ON)
endif()

# qt_add_resources(${EXE_NAME} "resources"
#   PREFIX "/"
#   FILES
#     web/index.html
#     web/mathquill.js
#     web/mathquill.css
#     web/bridge.js
# )

target_include_directories(${TARGET_NAME} PRIVATE
  include/
)

target_link_libraries(${TARGET_NAME} PRIVATE
  XDF
  Qt6::Widgets
  Qt6::WebEngineWidgets
  Qt6::WebChannel
)

