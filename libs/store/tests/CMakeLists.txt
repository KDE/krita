kde4_header()




########### next target ###############

set(storage_test_SRCS storage_test.cpp )

kde4_automoc(${storage_test_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(storage_test ${storage_test_SRCS})

target_link_libraries(storage_test  ${KDE4_KDECORE_LIBS} kstore )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(xmlwritertest_SRCS xmlwritertest.cpp )

kde4_automoc(${xmlwritertest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(xmlwritertest ${xmlwritertest_SRCS})

target_link_libraries(xmlwritertest  ${KDE4_KDECORE_LIBS} kstore )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(storedroptest_SRCS storedroptest.cpp )

kde4_automoc(${storedroptest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(storedroptest ${storedroptest_SRCS})

target_link_libraries(storedroptest  ${KDE4_KDECORE_LIBS} kstore )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

######## General stuff
#
#KDE_CXXFLAGS = $(USE_RTTI)
#INCLUDES= -I$(srcdir)/.. $(all_includes)
#
######## Files
#
#check_PROGRAMS = storage_test xmlwritertest storedroptest
#TESTS = storage_test xmlwritertest
#
#storage_test_SOURCES = storage_test.cpp
#storage_test_LDADD = ../libkstore.la
#
#xmlwritertest_SOURCES = xmlwritertest.cpp
#xmlwritertest_LDADD = ../libkstore.la
#
#storedroptest_SOURCES = storedroptest.cpp
#storedroptest_LDADD = ../libkstore.la
