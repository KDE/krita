add_subdirectory( tests )

include_directories( ${KOACTION_INCLUDES} )

set(koaction_LIB_SRCS
	ActionJob_p.cpp
	KoAction.cpp
	KoExecutePolicy.cpp
	KoJobsListPolicy.cpp
)


kde4_add_library(koaction SHARED ${koaction_LIB_SRCS})

target_link_libraries(koaction 
    ${KDE4_KDECORE_LIBS}
    ${KDE4_THREADWEAVER_LIBRARIES}
)

#target_link_libraries(koaction LINK_INTERFACE_LIBRARIES)

set_target_properties(koaction PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS koaction ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES
	ActionJob_p.h
	KoAction.h
	KoExecutePolicy.h
	KoExecutePolicy_p.h
	KoJobsListPolicy.h
	koaction_export.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

