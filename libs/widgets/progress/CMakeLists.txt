add_subdirectory( tests )
include_directories(${KDE4_INCLUDES} ${CMAKE_SOURCE_DIR}/libs/widgets)

########### next target ###############

set(koprogress_LIB_SRCS
    KoProgressBar.cpp
    KoProgressUpdater.cpp
    KoUpdater.cpp
    KoUpdaterPrivate.cpp
)

kde4_add_library(koprogress SHARED ${koprogress_LIB_SRCS})

target_link_libraries(koprogress ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(koprogress PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS koprogress  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install( FILES
    KoProgressUpdater.h
    KoProgressBar.h
    KoUpdater.h
DESTINATION ${INCLUDE_INSTALL_DIR}
COMPONENT Devel)
