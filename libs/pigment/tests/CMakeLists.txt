if(KDE4_BUILD_TESTS)

include_directories( ${PIGMENT_INCLUDES} )

########### next target ###############

set(TestColorConversion_test_SRCS TestColorConversion.cpp )

kde4_automoc(${TestColorConversion_test_SRCS})

kde4_add_executable(TestColorConversion RUN_UNINSTALLED ${TestColorConversion_test_SRCS})

target_link_libraries(TestColorConversion  ${KDE4_KDECORE_LIBS} pigment ${QT_QTTEST_LIBRARY})

add_test(pigment-TestColorConversion ${EXECUTABLE_OUTPUT_PATH}/TestColorConversion)

########### install files ###############

endif(KDE4_BUILD_TESTS)
