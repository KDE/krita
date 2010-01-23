set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOMAIN_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set(kopropertiestest_SRCS kopropertiestest.cpp )
kde4_add_unit_test(KoPropertiesTest TESTNAME libs-kobase-KoPropertiesTest ${kopropertiestest_SRCS})
target_link_libraries(KoPropertiesTest  ${KDE4_KDEUI_LIBS} kobase ${QT_QTTEST_LIBRARY})

set(TestKoUnit_SRCS TestKoUnit.cpp)
kde4_add_unit_test(TestKoUnit TESTNAME libs-kobase-TestKoUnit ${TestKoUnit_SRCS})
target_link_libraries(TestKoUnit kobase ${QT_QTTEST_LIBRARY})

