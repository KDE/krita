
set(kundo2_LIB_SRCS
	kundo2stack.cpp
	kundo2group.cpp
	kundo2view.cpp
	kundo2model.cpp
	kundo2magicstring.cpp
	kundo2commandextradata.cpp
)

add_library(kundo2 SHARED ${kundo2_LIB_SRCS})
generate_export_header(kundo2 BASE_NAME kundo2)

target_link_libraries(kundo2 KF5::IconThemes KF5::I18n KF5::XmlGui KF5::ConfigWidgets Qt5::Core Qt5::Widgets)

set_target_properties(kundo2 PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kundo2 ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kundo2_export.h
    kundo2stack.h
    kundo2qstack.h
    kundo2group.h
    kundo2view.h
    kundo2model.h
    kundo2command.h
    kundo2magicstring.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel
)
