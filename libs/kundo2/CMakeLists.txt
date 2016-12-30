set(kritaundo2_LIB_SRCS
	kundo2stack.cpp
	kundo2group.cpp
	kundo2view.cpp
	kundo2model.cpp
	kundo2magicstring.cpp
	kundo2commandextradata.cpp
        kis_undo_store.cpp
        kis_undo_stores.cpp
        kis_command_utils.cpp
)

add_library(kritaundo2 SHARED ${kritaundo2_LIB_SRCS})
generate_export_header(kritaundo2 BASE_NAME kritaundo2)

target_link_libraries(kritaundo2 
    PUBLIC
        kritawidgetutils 
        KF5::I18n 
        KF5::ConfigGui
        Qt5::Core 
        Qt5::Widgets
)

set_target_properties(kritaundo2 PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaundo2 ${INSTALL_TARGETS_DEFAULT_ARGS})
