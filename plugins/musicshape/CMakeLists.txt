add_subdirectory( fonts )
add_subdirectory( pics )
add_subdirectory( core )
add_subdirectory( tests)

include_directories(  )

########### Flake Plugin library ###############

set( musiccore_SRCS
    core/Global.cpp
    core/Sheet.cpp
    core/Part.cpp
    core/PartGroup.cpp
    core/Staff.cpp
    core/Voice.cpp
    core/Bar.cpp
    core/VoiceBar.cpp
    core/VoiceElement.cpp
    core/StaffElement.cpp
    core/Chord.cpp
    core/Note.cpp
    core/Clef.cpp
    core/KeySignature.cpp
    core/TimeSignature.cpp
    core/StaffSystem.cpp

    core/MusicXmlWriter.cpp
    core/MusicXmlReader.cpp
)

SET (musicshape_LIB_SRCS
    MusicShape.cpp
    MusicShapeFactory.cpp
    MusicTool.cpp
    MusicToolFactory.cpp
    SimpleEntryTool.cpp
    SimpleEntryToolFactory.cpp
    MusicStyle.cpp
    Engraver.cpp
    Renderer.cpp
    MusicCursor.cpp

    dialogs/PartsWidget.cpp
    dialogs/PartDetailsDialog.cpp
    dialogs/SimpleEntryWidget.cpp
    dialogs/PartsListModel.cpp
    dialogs/KeySignatureDialog.cpp
    dialogs/StaffElementPreviewWidget.cpp

    actions/AbstractMusicAction.cpp
    actions/AbstractNoteMusicAction.cpp
    actions/NoteEntryAction.cpp
    actions/AccidentalAction.cpp
    actions/EraserAction.cpp
    actions/DotsAction.cpp
    actions/SetClefAction.cpp
    actions/TimeSignatureAction.cpp
    actions/KeySignatureAction.cpp
    actions/RemoveBarAction.cpp
    actions/TiedNoteAction.cpp
    actions/SelectionAction.cpp
    
    commands/AddPartCommand.cpp
    commands/RemovePartCommand.cpp
    commands/ChangePartDetailsCommand.cpp
    commands/CreateChordCommand.cpp
    commands/AddNoteCommand.cpp
    commands/SetAccidentalsCommand.cpp
    commands/AddBarsCommand.cpp
    commands/RemoveNoteCommand.cpp
    commands/RemoveChordCommand.cpp
    commands/AddDotCommand.cpp
    commands/SetClefCommand.cpp
    commands/RemoveStaffElementCommand.cpp
    commands/SetTimeSignatureCommand.cpp
    commands/SetKeySignatureCommand.cpp
    commands/RemoveBarCommand.cpp
    commands/MakeRestCommand.cpp
    commands/ToggleTiedNoteCommand.cpp

    ${musiccore_SRCS}
)

kde4_add_ui_files(musicshape_LIB_SRCS
    dialogs/PartsWidget.ui
    dialogs/PartDetailsDialog.ui
    dialogs/SimpleEntryWidget.ui
    dialogs/KeySignatureDialog.ui
)

kde4_add_plugin(musicshape ${musicshape_LIB_SRCS})

target_link_libraries(musicshape flake)
install(TARGETS musicshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES musicshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
