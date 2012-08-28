set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CALLIGRADB_INCLUDES})

#add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")

########### next target ###############

kde4_add_unit_test(ConnectionTest TESTNAME ConnectionTest ConnectionTest.cpp)
target_link_libraries(ConnectionTest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${CALLIGRADB_LIBS})
