set(textlayout_LIB_SRCS
    KoTextLayoutCellHelper.cpp
    KoTextLayoutArea.cpp
    KoTextLayoutArea_paint.cpp
    KoTextLayoutEndNotesArea.cpp
    KoTextLayoutTableArea.cpp
    KoTextLayoutNoteArea.cpp
    KoTextLayoutRootArea.cpp
    KoTextLayoutRootAreaProvider.cpp
    KoTextDocumentLayout.cpp
    ListItemsHelper.cpp
    KoTextShapeContainerModel.cpp
    RunAroundHelper.cpp
    KoTextLayoutObstruction.cpp
    FrameIterator.cpp
    TableIterator.cpp
    KoPointedAt.cpp
    KoTextShapeData.cpp
    FloatingAnchorStrategy.cpp
    InlineAnchorStrategy.cpp
    AnchorStrategy.cpp
    ToCGenerator.cpp
    DummyDocumentLayout.cpp
    IndexGeneratorManager.cpp
    KoStyleThumbnailer.cpp
    TextLayoutDebug.cpp
)


kis_add_legacy_text_library(kritatextlayout SHARED ${textlayout_LIB_SRCS})
generate_export_header(kritatextlayout BASE_NAME kritatextlayout)

target_link_libraries(kritatextlayout kritatext)
target_link_libraries(kritatextlayout LINK_INTERFACE_LIBRARIES kritatext)

set_target_properties(kritatextlayout PROPERTIES
   VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritatextlayout  ${INSTALL_TARGETS_DEFAULT_ARGS})
