cmake_minimum_required(VERSION 3.16)

project(AgentBox_Device_Discovery VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_CXX_FLAGS "-static")
set(QT_STATIC_LINK TRUE)
set(QT_USE_STATIC_LIBS ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Network)

set(PROJECT_SOURCES
        main.cpp
)

if(WIN32)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc
        "IDI_ICON1 ICON DISCARDABLE \"${CMAKE_CURRENT_SOURCE_DIR}/logo_icon.ico\"\n"
    )
    set(APP_ICON_RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AgentBox_Device_Discovery
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AgentBox_Device_Discovery APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(AgentBox_Device_Discovery SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AgentBox_Device_Discovery
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(AgentBox_Device_Discovery PRIVATE
      Qt${QT_VERSION_MAJOR}::Widgets
      dwmapi
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AgentBox_Device_Discovery)
endif()
set_target_properties(AgentBox_Device_Discovery PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AgentBox_Device_Discovery
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AgentBox_Device_Discovery)
endif()
