set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(KRITA_RECORDERDOCKER_SOURCES
    recorder_const.cpp
    recorder_config.cpp
    recorder_export_config.cpp
    recorder_writer.cpp
    recorder_export.cpp
    recorder_profile_settings.cpp
    recorder_ffmpeg_wrapper.cpp
    recorder_snapshots_manager.cpp
    recorder_snapshots_scanner.cpp
    recorder_directory_cleaner.cpp
    recorderdocker.cpp
    recorderdocker_dock.cpp
)

ki18n_wrap_ui(KRITA_RECORDERDOCKER_SOURCES
    recorderdocker.ui
    recorder_export.ui
    recorder_profile_settings.ui
    recorder_snapshots_manager.ui
)

install(FILES
    recorder.action

    DESTINATION ${DATA_INSTALL_DIR}/krita/actions
)

add_library(kritarecorderdocker MODULE ${KRITA_RECORDERDOCKER_SOURCES})
target_link_libraries(kritarecorderdocker kritaui Threads::Threads)

install(TARGETS kritarecorderdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
