
if(KDE4_BUILD_TESTS)

########### next target ###############

set(storage_test_SRCS storage_test.cpp )

kde4_automoc(${storage_test_SRCS})

kde4_add_executable(storage_test RUN_UNINSTALLED ${storage_test_SRCS})

target_link_libraries(storage_test  ${KDE4_KDECORE_LIBS} kstore )

add_test(kostore-storage_test storage_test)

########### next target ###############

set(xmlwritertest_SRCS xmlwritertest.cpp )

kde4_automoc(${xmlwritertest_SRCS})

kde4_add_executable(xmlwritertest RUN_UNINSTALLED ${xmlwritertest_SRCS})

target_link_libraries(xmlwritertest  ${KDE4_KDECORE_LIBS} kstore )

add_test(kostore-xmlwritertest xmlwritertest)

########### next target ###############

set(koxmlreader_SRCS koxmlreadertest.cpp )

kde4_automoc(${koxmlreader_SRCS})

kde4_add_executable(koxmlreadertest RUN_UNINSTALLED ${koxmlreader_SRCS})

target_link_libraries(koxmlreadertest kofficecore ${QT_QTTEST_LIBRARY} )

add_test(kofficecore-koxmlreadertest koxmlreadertest)

########### next target ###############

set(storedroptest_SRCS storedroptest.cpp )

kde4_automoc(${storedroptest_SRCS})

kde4_add_executable(storedroptest RUN_UNINSTALLED ${storedroptest_SRCS})

target_link_libraries(storedroptest  ${KDE4_KDECORE_LIBS} kstore )

########### next target ###############

set(kodomtest_SRCS kodomtest.cpp )

kde4_automoc(${kodomtest_SRCS})

kde4_add_executable(kodomtest RUN_UNINSTALLED ${kodomtest_SRCS})

target_link_libraries(kodomtest ${KDE4_KDECORE_LIBS} kstore )

add_test(kostore-kodomtest kodomtest)

########### end ###############

endif(KDE4_BUILD_TESTS)

