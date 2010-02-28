add_subdirectory( widgets )
add_subdirectory( koplugin )
add_subdirectory( odf )
add_subdirectory( kotext )
add_subdirectory( flake )
add_subdirectory( pigment )
add_subdirectory( main )
if (SHOULD_BUILD_SCRIPTING)
    add_subdirectory( kokross )
endif (SHOULD_BUILD_SCRIPTING)
add_subdirectory( kopageapp )
add_subdirectory( koproperty ) # TODO move to independent place like kdesupport

#atm, koreport depends on kdchart.  this will hopefully
#fix builds for people who dont build kchart
if (WITH_kchart)
    add_subdirectory( koreport )
endif (WITH_kchart)