########### next target ###############

set(storage_test_SRCS storage_test.cpp )

kde4_automoc(${storage_test_SRCS})

kde4_add_unit_test(storage_test TESTNAME libs-store-storage_test ${storage_test_SRCS})

target_link_libraries(storage_test  ${KDE4_KDECORE_LIBS} kstore )

########### next target ###############

set(storage_test2_SRCS storage_test2.cpp )

kde4_automoc(${storage_test2_SRCS})

kde4_add_unit_test(storage_test2 TESTNAME libs-store-storage_test2 ${storage_test2_SRCS})

target_link_libraries(storage_test2  ${KDE4_KDECORE_LIBS} kstore )

########### next target ###############

set(xmlwritertest_SRCS xmlwritertest.cpp )

kde4_automoc(${xmlwritertest_SRCS})

kde4_add_unit_test(xmlwritertest TESTNAME libs-store-xmlwritertest ${xmlwritertest_SRCS})

target_link_libraries(xmlwritertest  ${KDE4_KDECORE_LIBS} kstore )

########### next target ###############

set(koxmlreader_SRCS koxmlreadertest.cpp )

kde4_automoc(${koxmlreader_SRCS})

kde4_add_unit_test(koxmlreadertest TESTNAME libs-store-koxmlreadertest ${koxmlreader_SRCS})

target_link_libraries(koxmlreadertest kstore ${QT_QTTEST_LIBRARY} )

########### next target ###############
if(KDE4_BUILD_TESTS)

set(storedroptest_SRCS storedroptest.cpp )

kde4_automoc(${storedroptest_SRCS})

# this test shows a GUI and expects interaction; so don't run it with a 'make test'
kde4_add_executable(storedroptest RUN_UNINSTALLED ${storedroptest_SRCS})

target_link_libraries(storedroptest  ${KDE4_KDECORE_LIBS} kstore )
endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kodomtest_SRCS kodomtest.cpp )

kde4_automoc(${kodomtest_SRCS})

kde4_add_unit_test(kodomtest TESTNAME libs-store-kodomtest ${kodomtest_SRCS})

target_link_libraries(kodomtest ${KDE4_KDECORE_LIBS} kstore ${QT_QTTEST_LIBRARY} )

########### end ###############
