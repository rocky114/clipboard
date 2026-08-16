cmake_minimum_required(VERSION 3.28)
project(clipboard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Qt6_DIR "/opt/homebrew/opt/qt/lib/cmake/Qt6" CACHE PATH "directory where Qt6Config.cmake exists")

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

# todo sanitizers
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(clipboard src/main.cpp src/clipboardmanager.cpp)
target_link_libraries(clipboard PRIVATE project_options Qt6::Core Qt6::Gui Qt6::Widgets)


# todo: add a proper icon for the app
set_target_properties(clipboard PROPERTIES MACOSX_BUNDLE TRUE)
