set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/..  )

########### next target ###############

set(SheetTest_test_SRCS
    SheetTest.cpp
    ../Global.cpp
    ../Part.cpp
    ../Sheet.cpp
    ../PartGroup.cpp
    ../Bar.cpp
    ../Voice.cpp
    ../Staff.cpp
    ../VoiceBar.cpp
    ../StaffElement.cpp
    ../Clef.cpp
    ../StaffSystem.cpp
    ../VoiceElement.cpp
    ../KeySignature.cpp
    ../TimeSignature.cpp
    ../Note.cpp
    ../Chord.cpp
)

kde4_add_unit_test(SheetTest TESTNAME shapes-musicshape-SheetTest ${SheetTest_test_SRCS})
target_link_libraries(SheetTest ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})


########### next target ###############

set(VoiceBarTest_test_SRCS
    VoiceBarTest.cpp
    ../Global.cpp
    ../VoiceBar.cpp
    ../VoiceElement.cpp
    ../Sheet.cpp
    ../Bar.cpp
    ../PartGroup.cpp
    ../Part.cpp
    ../StaffSystem.cpp
    ../Staff.cpp
    ../StaffElement.cpp
    ../Voice.cpp
    ../Clef.cpp
    ../KeySignature.cpp
    ../TimeSignature.cpp
    ../Note.cpp
    ../Chord.cpp
)

kde4_add_unit_test(VoiceBarTest TESTNAME shapes-musicshape-VoiceBarTest ${VoiceBarTest_test_SRCS})
target_link_libraries(VoiceBarTest ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(MusicXmlWriterTest_test_SRCS
    MusicXmlWriterTest.cpp
    ../Global.cpp
    ../MusicXmlWriter.cpp
    ../Part.cpp
    ../Sheet.cpp
    ../PartGroup.cpp
    ../Bar.cpp
    ../Voice.cpp
    ../Staff.cpp
    ../VoiceBar.cpp
    ../StaffElement.cpp
    ../Clef.cpp
    ../StaffSystem.cpp
    ../VoiceElement.cpp
    ../Chord.cpp
    ../Note.cpp
    ../KeySignature.cpp
    ../TimeSignature.cpp
)

kde4_add_unit_test(MusicXmlWriterTest TESTNAME shapes-musicshape-MusicXmlWriterTest ${MusicXmlWriterTest_test_SRCS})
target_link_libraries(MusicXmlWriterTest ${QT_QTTEST_LIBRARY} kostore)



