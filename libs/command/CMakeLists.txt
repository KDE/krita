add_subdirectory( tests )

set(kritacommand_LIB_SRCS
	kundo2stack.cpp
	kundo2group.cpp
	kundo2view.cpp
	kundo2model.cpp
	kundo2magicstring.cpp
	kundo2commandextradata.cpp
        kis_undo_store.cpp
        kis_undo_stores.cpp
        kis_command_utils.cpp
        KisCumulativeUndoData.cpp
)

kis_add_library(kritacommand SHARED ${kritacommand_LIB_SRCS})
generate_export_header(kritacommand BASE_NAME kritacommand)

target_link_libraries(kritacommand
    PUBLIC
        kritawidgetutils 
        KF${KF_MAJOR}::I18n 
        KF${KF_MAJOR}::ConfigGui
        Qt${QT_MAJOR_VERSION}::Core 
        Qt${QT_MAJOR_VERSION}::Widgets
        Boost::boost
)

set_target_properties(kritacommand PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritacommand ${INSTALL_TARGETS_DEFAULT_ARGS})
