set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(KRITA_RECORDERDOCKER_SOURCES
    recorder_const.cpp
    recorder_config.cpp
    recorder_export_config.cpp
    recorder_format.cpp
    recorder_writer.cpp
    recorder_export.cpp
    recorder_snapshots_manager.cpp
    recorder_snapshots_scanner.cpp
    recorder_directory_cleaner.cpp
    recorderdocker.cpp
    recorderdocker_dock.cpp
)

set(KRITA_RECORDERDOCKER_UIS
    recorderdocker.ui
    recorder_export.ui
    recorder_snapshots_manager.ui
)

if(NOT ANDROID)
    list(APPEND KRITA_RECORDERDOCKER_SOURCES recorder_profile_settings.cpp)
    list(APPEND KRITA_RECORDERDOCKER_UIS recorder_profile_settings.ui)
endif()

ki18n_wrap_ui(KRITA_RECORDERDOCKER_SOURCES ${KRITA_RECORDERDOCKER_UIS})

install(FILES
    recorder.action

    DESTINATION ${KDE_INSTALL_DATADIR}/krita/actions
)

kis_add_library(kritarecorderdocker MODULE ${KRITA_RECORDERDOCKER_SOURCES})
target_link_libraries(kritarecorderdocker kritaui Threads::Threads)

install(TARGETS kritarecorderdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
