set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

include_directories(SYSTEM
                    ${AVCODEC_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS} ${SWSCALE_INCLUDE_DIRS})

set(KRITA_RECORDERDOCKER_SOURCES
    recorder_config.cpp
    recorder_writer.cpp
    recorderdocker.cpp
    recorderdocker_dock.cpp
)
# TODO REMOVE
#    encoder.cpp
#    encoder_queue.cpp

ki18n_wrap_ui(KRITA_RECORDERDOCKER_SOURCES
    recorderdocker.ui
)

add_library(kritarecorderdocker MODULE ${KRITA_RECORDERDOCKER_SOURCES})
target_link_libraries(kritarecorderdocker kritaui ${AVCODEC_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVUTIL_LIBRARIES} ${SWSCALE_LIBRARIES} Threads::Threads)

install(TARGETS kritarecorderdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
