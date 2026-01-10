add_subdirectory( tests )

include(CheckFunctionExists)
check_function_exists(backtrace HAVE_BACKTRACE)
configure_file(config-debug.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-debug.h)

option(HAVE_MEMORY_LEAK_TRACKER "Enable memory leak tracker (always disabled in release build)" OFF)
option(HAVE_BACKTRACE_SUPPORT "Enable recording of backtrace in memory leak tracker" OFF)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-memory-leak-tracker.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-memory-leak-tracker.h) ### WRONG PLACE???

set(kritaglobal_LIB_SRCS
    kis_assert.cpp
    kis_debug.cpp
    kis_algebra_2d.cpp
    kis_memory_leak_tracker.cpp
    kis_shared.cpp
    kis_dom_utils.cpp
    kis_painting_tweaks.cpp
    KisHandlePainterHelper.cpp
    KisHandleStyle.cpp
    kis_signal_compressor.cpp
    kis_signal_compressor_with_param.cpp
    kis_thread_safe_signal_compressor.cpp
    kis_acyclic_signal_connector.cpp
    kis_latency_tracker.cpp
    KisQPainterStateSaver.cpp
    KisRollingMeanAccumulatorWrapper.cpp
    KisRollingSumAccumulatorWrapper.cpp
    kis_config_notifier.cpp
    KisDeleteLaterWrapper.cpp
    KisUsageLogger.cpp
    KisFileUtils.cpp
    KisSignalMapper.cpp
    KisRegion.cpp
    KoID.cpp
    KoUnit.cpp
    KisBezierUtils.cpp
    KisBezierPatch.cpp
    KisBezierMesh.cpp
    KisRectsGrid.cpp
    KisSynchronizedConnection.cpp
    KisFilteredRollingMean.cpp
    KisBackup.cpp
    KisSampleRectIterator.cpp
    KisCursorOverrideLock.cpp
)

if(WIN32)
    set(kritaglobal_LIB_SRCS
        ${kritaglobal_LIB_SRCS}
        KisWindowsPackageUtils.cpp
    )
endif(WIN32)

if (ANDROID)
    list(APPEND kritaglobal_LIB_SRCS
        KisAndroidCrashHandler.cpp
        KisAndroidUtils.cpp
    )
    include_directories(${unwindstack_INCLUDE_DIR})
    include_directories(${Qt5AndroidExtras_INCLUDE_DIRS})
endif()

kis_add_library(kritaglobal SHARED ${kritaglobal_LIB_SRCS}
   )
generate_export_header(kritaglobal BASE_NAME kritaglobal)

target_link_libraries(kritaglobal 
    PUBLIC
        kritaversion
        Qt5::Concurrent 
        Qt5::Core 
        Qt5::Gui 
        Qt5::Widgets 
        Qt5::Xml 
        KF5::I18n
        Boost::boost
        zug
        lager
    PRIVATE
        Eigen3::Eigen
)

if (GSL_FOUND)
  target_link_libraries(kritaglobal PRIVATE GSL::gsl)
endif ()

if (ANDROID)
    target_link_libraries(kritaglobal PRIVATE ${unwindstack_LIBRARIES} Qt5::AndroidExtras log)
endif()

if(APPLE)
    target_link_libraries(kritaglobal PRIVATE
        kritamacosutils
    )
endif ()

set_target_properties(kritaglobal PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritaglobal  ${INSTALL_TARGETS_DEFAULT_ARGS})

