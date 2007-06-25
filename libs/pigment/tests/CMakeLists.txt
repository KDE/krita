if(KDE4_BUILD_TESTS)

include_directories( ${PIGMENT_INCLUDES} )

########### next target ###############

set(TestColorConversion_test_SRCS TestColorConversion.cpp )

kde4_automoc(${TestColorConversion_test_SRCS})

kde4_add_executable(TestColorConversion RUN_UNINSTALLED ${TestColorConversion_test_SRCS})

target_link_libraries(TestColorConversion  ${KDE4_KDECORE_LIBS} pigment ${QT_QTTEST_LIBRARY})

add_test(libs-pigment-TestColorConversion ${EXECUTABLE_OUTPUT_PATH}/TestColorConversion)

########### next target ###############

set(TestKoColorSpaceAbstract_test_SRCS TestKoColorSpaceAbstract.cpp )

kde4_automoc(${TestKoColorSpaceAbstract_test_SRCS})

kde4_add_executable(TestKoColorSpaceAbstract RUN_UNINSTALLED ${TestKoColorSpaceAbstract_test_SRCS})

target_link_libraries(TestKoColorSpaceAbstract  ${KDE4_KDECORE_LIBS} pigment ${QT_QTTEST_LIBRARY})

add_test(libs-pigment-TestKoColorSpaceAbstract ${EXECUTABLE_OUTPUT_PATH}/TestKoColorSpaceAbstract)

########### next target ###############

set(TestKoLcmsColorProfile_test_SRCS TestKoLcmsColorProfile.cpp )

kde4_automoc(${TestKoLcmsColorProfile_test_SRCS})

kde4_add_executable(TestKoLcmsColorProfile RUN_UNINSTALLED ${TestKoLcmsColorProfile_test_SRCS})

target_link_libraries(TestKoLcmsColorProfile  ${KDE4_KDECORE_LIBS} pigment ${QT_QTTEST_LIBRARY})

add_test(libs-pigment-TestKoLcmsColorProfile ${EXECUTABLE_OUTPUT_PATH}/TestKoLcmsColorProfile)

########### install files ###############

endif(KDE4_BUILD_TESTS)
