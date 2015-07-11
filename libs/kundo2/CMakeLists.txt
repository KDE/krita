
set(kundo2_LIB_SRCS
	kundo2stack.cpp
	kundo2group.cpp
	kundo2view.cpp
	kundo2model.cpp
	kundo2magicstring.cpp
	kundo2commandextradata.cpp
)

add_library(kundo2 SHARED ${kundo2_LIB_SRCS})

target_link_libraries(kundo2 KF5::KDELibs4Support Qt5::Core Qt5::Widgets)

set_target_properties(kundo2 PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kundo2 ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kundo2_export.h
    kundo2stack.h
    kundo2qstack.h
    kundo2group.h
    kundo2view.h
    kundo2model.h
    kundo2command.h
    kundo2magicstring.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel
)
