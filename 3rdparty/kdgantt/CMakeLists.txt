include_directories(
    ${QT_INCLUDES}
    ${KDGANTT_INCLUDES}
)

add_definitions(-DKDAB_NO_UNIT_TESTS -DKDGANTT_SHAREDLIB -DKDGANTT_BUILD_KDGANTT_LIB)

set( kdgantt_LIB_SRCS
    kdganttglobal.cpp
    kdganttview.cpp
    kdganttstyleoptionganttitem.cpp
    kdganttgraphicsview.cpp
    kdganttabstractrowcontroller.cpp
    kdgantttreeviewrowcontroller.cpp
    kdganttlistviewrowcontroller.cpp
    kdganttgraphicsscene.cpp
    kdganttgraphicsitem.cpp
    kdganttconstraint.cpp
    kdganttconstraintproxy.cpp
    kdganttconstraintgraphicsitem.cpp
    kdganttitemdelegate.cpp
    kdganttforwardingproxymodel.cpp
    kdganttsummaryhandlingproxymodel.cpp
    kdganttproxymodel.cpp
    kdganttconstraintmodel.cpp
    kdganttabstractgrid.cpp
    kdganttdatetimegrid.cpp
    kdganttlegend.cpp
    kdgantttimescalezoomdialog.cpp
)

kde4_add_library(calligrakdgantt SHARED ${kdgantt_LIB_SRCS})

target_link_libraries(calligrakdgantt ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
target_link_libraries(calligrakdgantt LINK_INTERFACE_LIBRARIES ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

set_target_properties(calligrakdgantt PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION}
    SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS calligrakdgantt ${INSTALL_TARGETS_DEFAULT_ARGS} )
