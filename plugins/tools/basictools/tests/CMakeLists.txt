########### next target ###############

krita_add_broken_unit_test(move_stroke_test.cpp  $<TARGET_PROPERTY:kritatestsdk,SOURCE_DIR>/stroke_testing_utils.cpp
    TEST_NAME MoveStrokeTest
    LINK_LIBRARIES kritadefaulttools_static kritabasicflakes kritaui kritatestsdk
    )