set(KRITA_flipbookDOCKER_PART_SRCS 
    flipbookdocker.cpp 
    flipbookdocker_dock.cpp 
    FlipbookView.cpp
    sequence_viewer.cpp
)

kde4_add_ui_files(KRITA_flipbookDOCKER_PART_SRCS
    wdgflipbookdocker.ui
)
kde4_add_plugin(kritaflipbookdocker ${KRITA_flipbookDOCKER_PART_SRCS})

target_link_libraries(kritaflipbookdocker kritaui)

#install(TARGETS kritaflipbookdocker  DESTINATION ${PLUGIN_INSTALL_DIR})
#install( FILES  krita_flipbookdocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

