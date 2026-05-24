cmake_minimum_required(VERSION 3.21)

project(PipeSightConsole VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.10 REQUIRED COMPONENTS
    Quick
    QuickControls2
    Network
    Sql
    Multimedia
    Concurrent
)

qt_standard_project_setup(REQUIRES 6.10)

# --- C++ sources -----------------------------------------------------------
set(COMM_SOURCES
    src/comm/tcp_client.h           src/comm/tcp_client.cpp
    src/comm/protocol_codec.h       src/comm/protocol_codec.cpp
    src/comm/connection_manager.h   src/comm/connection_manager.cpp
)

set(SERVICE_SOURCES
    src/services/camera_service.h       src/services/camera_service.cpp
    src/services/laser_service.h        src/services/laser_service.cpp
    src/services/vehicle_service.h      src/services/vehicle_service.cpp
    src/services/osd_service.h          src/services/osd_service.cpp
    src/services/recording_service.h    src/services/recording_service.cpp
    src/services/report_service.h       src/services/report_service.cpp
)

# ViewModels are QML-exposed types; declared as QML_ELEMENT in headers.
set(VIEWMODEL_SOURCES
    src/viewmodels/camera_viewmodel.h       src/viewmodels/camera_viewmodel.cpp
    src/viewmodels/laser_viewmodel.h        src/viewmodels/laser_viewmodel.cpp
    src/viewmodels/vehicle_viewmodel.h      src/viewmodels/vehicle_viewmodel.cpp
    src/viewmodels/osd_viewmodel.h          src/viewmodels/osd_viewmodel.cpp
    src/viewmodels/config_viewmodel.h       src/viewmodels/config_viewmodel.cpp
    src/viewmodels/advanced_viewmodel.h     src/viewmodels/advanced_viewmodel.cpp
)

set(DATA_SOURCES
    src/data/database.h         src/data/database.cpp
    src/data/app_settings.h     src/data/app_settings.cpp
)

set(QML_FILES
    qml/Main.qml
    qml/panels/CameraPanel.qml
    qml/panels/LaserPanel.qml
    qml/panels/VehicleControlPanel.qml
    qml/panels/VehicleInfoPanel.qml
    qml/panels/OsdPanel.qml
    qml/panels/ConfigPanel.qml
    qml/panels/AdvancedPanel.qml
    qml/components/Joystick.qml
    qml/components/ConnectionIndicator.qml
    qml/components/VideoView.qml
)

# Keep the QML source layout (qml/, qml/panels/, qml/components/) on disk,
# but flatten the qrc resource paths so every QML file sits directly under
# :/qt/qml/PipeSightConsole/<File>.qml — i.e. alongside the auto-generated
# qmldir. Without this, the qmldir lives at /qt/qml/PipeSightConsole/qmldir
# while QML files live at /qt/qml/PipeSightConsole/qml/... and inter-type
# resolution inside the module breaks ("X is not a type").
foreach(_qml_file IN LISTS QML_FILES)
    get_filename_component(_qml_basename "${_qml_file}" NAME)
    set_source_files_properties("${_qml_file}" PROPERTIES
        QT_RESOURCE_ALIAS "${_qml_basename}"
    )
endforeach()

qt_add_executable(appPipeSightConsole
    src/app/main.cpp
    ${COMM_SOURCES}
    ${SERVICE_SOURCES}
    ${DATA_SOURCES}
)

# QML module: QML files + the C++ ViewModels that QML uses (QML_ELEMENT)
qt_add_qml_module(appPipeSightConsole
    URI PipeSightConsole
    VERSION 1.0
    QML_FILES ${QML_FILES}
    SOURCES   ${VIEWMODEL_SOURCES}
)

target_include_directories(appPipeSightConsole PRIVATE
    src
    src/viewmodels
)

target_link_libraries(appPipeSightConsole PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Sql
    Qt6::Multimedia
    Qt6::Concurrent
)

if (ANDROID)
    target_link_options(appPipeSightConsole PRIVATE
        "-Wl,-z,max-page-size=16384"
        "-Wl,-z,common-page-size=16384"
    )
endif()

set_target_properties(appPipeSightConsole PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

## Android packaging hooks (filled in when targeting Android in Qt Creator)
#if (ANDROID)
#    set_target_properties(appPipeSightConsole PROPERTIES
#        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
#    )
#endif()

install(TARGETS appPipeSightConsole
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
