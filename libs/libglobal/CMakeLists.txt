include(CheckFunctionExists)
check_function_exists(backtrace HAVE_BACKTRACE)
configure_file(config-debug.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-debug.h)

option(HAVE_MEMORY_LEAK_TRACKER "Enable memory leak tracker (always disabled in release build)" OFF)
option(HAVE_BACKTRACE_SUPPORT "Enable recording of backtrace in memory leak tracker" OFF)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-memory-leak-tracker.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-memory-leak-tracker.h) ### WRONG PLACE???

set(kritaglobal_LIB_SRCS
  kis_assert.cpp
  kis_debug.cpp
  kis_memory_leak_tracker.cpp
  kis_shared.cpp
)

add_library(kritaglobal SHARED ${kritaglobal_LIB_SRCS} )
generate_export_header(kritaglobal BASE_NAME kritaglobal)

target_link_libraries(kritaglobal 
    PUBLIC
        Qt5::Concurrent 
        Qt5::Core 
        Qt5::Gui 
        Qt5::Widgets 
        KF5::I18n
)

set_target_properties(kritaglobal PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritaglobal  ${INSTALL_TARGETS_DEFAULT_ARGS})

