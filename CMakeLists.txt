cmake_minimum_required(VERSION 3.16)
project(betterMaths)

set(EXE_NAME ${CMAKE_PROJECT_NAME})
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

qt_add_executable(${EXE_NAME}
  ${PROJECT_SOURCES}
  resources.qrc
)

# qt_add_resources(${EXE_NAME} "resources"
#   PREFIX "/"
#   FILES
#     web/index.html
#     web/mathquill.js
#     web/mathquill.css
#     web/bridge.js
# )

target_include_directories(${EXE_NAME} PRIVATE
  include/
)

target_link_libraries(${EXE_NAME} PRIVATE
  XDF
  Qt6::Widgets
  Qt6::WebEngineWidgets
  Qt6::WebChannel
)

