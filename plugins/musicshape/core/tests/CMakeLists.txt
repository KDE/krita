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

add_executable(SheetTestNAME shapes-musicshape-SheetTest ${SheetTest_test_SRCS})
add_test(SheetTest SheetTest)
ecm_mark_as_test(SheetTest)
target_link_libraries(SheetTest Qt5::Core Qt5::Test)


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

add_executable(VoiceBarTestNAME shapes-musicshape-VoiceBarTest ${VoiceBarTest_test_SRCS})
add_test(VoiceBarTest VoiceBarTest)
ecm_mark_as_test(VoiceBarTest)
target_link_libraries(VoiceBarTest Qt5::Core Qt5::Test)

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

add_executable(MusicXmlWriterTestNAME shapes-musicshape-MusicXmlWriterTest ${MusicXmlWriterTest_test_SRCS})
add_test(MusicXmlWriterTest MusicXmlWriterTest)
ecm_mark_as_test(MusicXmlWriterTest)
target_link_libraries(MusicXmlWriterTest Qt5::Test koodf)



