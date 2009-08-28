
include_directories( ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_INCLUDE_DIR} )

set( musicwidgettest_SRCS
    MusicWidget.cpp
    MusicWidgetTest.cpp

    ../core/Bar.cpp
    ../core/Chord.cpp
    ../core/Clef.cpp
    ../core/Global.cpp
    ../core/KeySignature.cpp
    ../core/Note.cpp
    ../core/Part.cpp
    ../core/PartGroup.cpp
    ../core/Sheet.cpp
    ../core/Staff.cpp
    ../core/StaffElement.cpp
    ../core/StaffSystem.cpp
    ../core/TimeSignature.cpp
    ../core/Voice.cpp
    ../core/VoiceBar.cpp
    ../core/VoiceElement.cpp

    ../Engraver.cpp
    ../Renderer.cpp
    ../MusicStyle.cpp
)

kde4_add_executable(musicwidgettest TEST ${musicwidgettest_SRCS})
target_link_libraries(musicwidgettest ${QT_QTGUI_LIBRARY})
