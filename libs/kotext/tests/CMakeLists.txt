

if(KDE4_BUILD_TESTS)

include_directories( ${KOTEXT_INCLUDES} )

########### next target ###############

set(kotextformattertest_SRCS kotextformattertest.cpp )

kde4_automoc(${kotextformattertest_SRCS})

kde4_add_executable(kotextformattertest RUN_UNINSTALLED ${kotextformattertest_SRCS})

target_link_libraries(kotextformattertest kotext)

########### next target ###############

set(kobordertest_SRCS kobordertest.cpp )

kde4_automoc(${kobordertest_SRCS})

kde4_add_executable(kobordertest RUN_UNINSTALLED ${kobordertest_SRCS})

target_link_libraries(kobordertest kotext)

########### next target ###############

set(kovariabletest_SRCS kovariabletest.cpp)

kde4_automoc(${kovariabletest_SRCS})

kde4_add_executable(kovariabletest RUN_UNINSTALLED ${kovariabletest_SRCS})

target_link_libraries(kovariabletest kotext ${QT_QTTEST_LIBRARY})

add_test(kotext-kovariabletest ${CMAKE_CURRENT_BINARY_DIR}/kovariabletest)

########### end ###############

endif(KDE4_BUILD_TESTS)





#original Makefile.am contents follow:

#INCLUDES = $(KOFFICECORE_INCLUDES) $(KOFFICEUI_INCLUDES) $(KOTEXT_INCLUDES) $(all_includes)
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#METASOURCES = AUTO
#
#check_PROGRAMS = kotextformattertest kobordertest kovariabletest
#
#TESTS = kotextformattertest kovariabletest
#
#LDADD = ../libkotext.la
#AM_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#
#kobordertest_SOURCES = kobordertest.cpp
#kotextformattertest_SOURCES = kotextformattertest.cpp
#kovariabletest_SOURCES = kovariabletest.cpp
#
#check_LTLIBRARIES = kunittest_KoUserStyleTester.la 
#
#kunittest_KoUserStyleTester_la_SOURCES = KoUserStyleTester.cpp
#kunittest_KoUserStyleTester_la_LIBADD = ../libkotext.la -lkunittest
#kunittest_KoUserStyleTester_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
#check-local:
#	kunittestmodrunner
