include_directories(${KUNDO2_INCLUDES})

set(kritaundo2_LIB_SRCS
	kundo2stack.cpp
	kundo2group.cpp
	kundo2view.cpp
	kundo2model.cpp
	kundo2magicstring.cpp
	kundo2commandextradata.cpp
)

add_library(kritaundo2 SHARED ${kritaundo2_LIB_SRCS})
generate_export_header(kritaundo2 BASE_NAME kritaundo2)

target_link_libraries(kritaundo2 kritawidgetutils KF5::IconThemes KF5::I18n KF5::XmlGui Qt5::Core Qt5::Widgets)

set_target_properties(kritaundo2 PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kritaundo2 ${INSTALL_TARGETS_DEFAULT_ARGS})
