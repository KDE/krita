include_directories(${TEXTLAYOUT_INCLUDES})

#add_subdirectory( tests )

########### next target ###############
set(textlayout_LIB_SRCS
    KoTextLayoutArea.cpp
    KoTextLayoutRootArea.cpp
    KoTextLayoutRootAreaProvider.cpp
    KoTextDocumentLayout.cpp
    ListItemsHelper.cpp
    KoStyleThumbnailer.cpp
    KoTextShapeContainerModel.cpp
    RunAroundHelper.cpp
    Outline.cpp
)


kde4_add_library(textlayout SHARED ${textlayout_LIB_SRCS})

target_link_libraries(textlayout kotext)
target_link_libraries(textlayout LINK_INTERFACE_LIBRARIES kotext)

set_target_properties(textlayout PROPERTIES
   VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS textlayout  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

#install( FILES ttt.tt DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(
    FILES
    KoTextDocumentLayout.h
    KoStyleThumbnailer.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
