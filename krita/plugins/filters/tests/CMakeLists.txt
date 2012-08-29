set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}  )
add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")

########### next target ###############

set(kis_all_filter_test_SRCS kis_all_filter_test.cpp )
kde4_add_executable(KisAllFilterTest TEST ${kis_all_filter_test_SRCS})
target_link_libraries(KisAllFilterTest ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_crash_filter_test_SRCS kis_crash_filter_test.cpp )
kde4_add_executable(KisCrashFilterTest TEST ${kis_crash_filter_test_SRCS})
target_link_libraries(KisCrashFilterTest ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})
