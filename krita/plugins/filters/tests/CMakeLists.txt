set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES} )
add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")

########### next target ###############

set(kis_all_filter_test_SRCS kis_all_filter_test.cpp )

kde4_add_unit_test(KisAllFilterTest TESTNAME krita-filter-AllFilterTest ${kis_all_filter_test_SRCS})

target_link_libraries(KisAllFilterTest ${KDE4_KDEUI_LIBS} komain kritaimage ${QT_QTTEST_LIBRARY})

