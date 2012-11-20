include_directories(
    ${QT_INCLUDES}
    ${KDGANTT_INCLUDES}
)

add_definitions(-DKDAB_NO_UNIT_TESTS -DKDGANTT_SHAREDLIB -DKDGANTT_BUILD_KDGANTT_LIB)

#FIXME: We should be able to remove
#       "${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/" once the library is used
#        everywhere.
set( kdgantt_LIB_SRCS
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttglobal.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttview.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttstyleoptionganttitem.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttgraphicsview.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttabstractrowcontroller.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdgantttreeviewrowcontroller.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttlistviewrowcontroller.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttgraphicsscene.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttgraphicsitem.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttconstraint.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttconstraintproxy.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttconstraintgraphicsitem.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttitemdelegate.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttforwardingproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttsummaryhandlingproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttconstraintmodel.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttabstractgrid.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttdatetimegrid.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttlegend.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdgantttimescalezoomdialog.cpp
)

kde4_add_library(kdgantt SHARED ${kdgantt_LIB_SRCS})

target_link_libraries(kdgantt ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
target_link_libraries(kdgantt LINK_INTERFACE_LIBRARIES ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

set_target_properties(kdgantt PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kdgantt ${INSTALL_TARGETS_DEFAULT_ARGS} )
