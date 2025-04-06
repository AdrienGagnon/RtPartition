cmake_minimum_required(VERSION 3.16)
project(MyMidiScoreProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Toolchain vcpkg
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")

# Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# RtMidi (add submodule or use external)
add_subdirectory(third_party/rtmidi)

# Lomse
include_directories("C:/lomse")
include_directories("C:/lomse/build")
include_directories("C:/lomse/include")
include_directories("C:/lomse/src/agg/include")
include_directories("C:/lomse/build/generated")
link_directories("C:/lomse/build/Release")


# Freetype (using vcpkg)
find_package(Freetype REQUIRED)

# PNG
find_package(PNG REQUIRED)

# Sources
file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LomseViewWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LomseViewWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RtMidiHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RtMidiHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/NoteCommand.h
    ${CMAKE_CURRENT_SOURCE_DIR}/NoteCommand.cpp
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set flags for Release/Debug build types
set(CMAKE_CXX_FLAGS_RELEASE "/MD ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "/MDd ${CMAKE_CXX_FLAGS_DEBUG}")

# Linking Libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    lomse
    rtmidi
    Freetype::Freetype
    PNG::PNG
    ZLIB::ZLIB
)

