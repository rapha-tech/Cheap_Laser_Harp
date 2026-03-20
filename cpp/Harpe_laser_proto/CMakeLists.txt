cmake_minimum_required(VERSION 3.16)

project(Harpe_laser_proto VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        miniaudio_io.h
        RtMidi.cpp
        RtMidi.h
        tsf.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Harpe_laser_proto
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        piano_ui.h piano_ui.cpp
        PianoKey.h

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Harpe_laser_proto APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Harpe_laser_proto SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Harpe_laser_proto
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Harpe_laser_proto PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_compile_options(Harpe_laser_proto PRIVATE -Wall)

if(WIN32)
    target_compile_definitions(Harpe_laser_proto PRIVATE __WINDOWS_MM__)
    target_link_libraries(Harpe_laser_proto PRIVATE winmm)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(Harpe_laser_proto PRIVATE __LINUX_ALSA__)
    target_link_libraries(Harpe_laser_proto PRIVATE pthread asound)
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Harpe_laser_proto)
endif()
set_target_properties(Harpe_laser_proto PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Harpe_laser_proto
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Harpe_laser_proto)
endif()
