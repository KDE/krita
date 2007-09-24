set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${PIGMENT_INCLUDES} )

########### next target ###############

set(TestColorConversion_test_SRCS TestColorConversion.cpp )

kde4_add_unit_test(TestColorConversion TESTNAME libs-pigment-TestColorConversion  ${TestColorConversion_test_SRCS})

target_link_libraries(TestColorConversion  ${KDE4_KDECORE_LIBS} pigment ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoColorSpaceAbstract_test_SRCS TestKoColorSpaceAbstract.cpp )

kde4_add_unit_test(TestKoColorSpaceAbstract TESTNAME libs-pigment-TestKoColorSpaceAbstract  ${TestKoColorSpaceAbstract_test_SRCS})

target_link_libraries(TestKoColorSpaceAbstract  ${KDE4_KDECORE_LIBS} pigment ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoLcmsColorProfile_test_SRCS TestKoLcmsColorProfile.cpp )

kde4_add_unit_test(TestKoLcmsColorProfile TESTNAME libs-pigment-TestKoLcmsColorProfile  ${TestKoLcmsColorProfile_test_SRCS})

target_link_libraries(TestKoLcmsColorProfile  ${KDE4_KDECORE_LIBS} pigment ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoColorSpaceRegistry_test_SRCS TestKoColorSpaceRegistry.cpp )

kde4_add_unit_test(TestKoColorSpaceRegistry TESTNAME libs-pigment-TestKoColorSpaceRegistry  ${TestKoColorSpaceRegistry_test_SRCS})

target_link_libraries(TestKoColorSpaceRegistry  ${KDE4_KDECORE_LIBS} pigment ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoColorSpaceMaths_test_SRCS TestKoColorSpaceMaths.cpp)

kde4_add_unit_test(TestKoColorSpaceMaths TESTNAME libs-pigment-TestKoColorSpaceMaths ${TestKoColorSpaceMaths_test_SRCS})

target_link_libraries(TestKoColorSpaceMaths  ${KDE4_KDECORE_LIBS} pigment ${QT_QTTEST_LIBRARY})

