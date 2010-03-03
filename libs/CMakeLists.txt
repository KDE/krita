add_subdirectory( koplugin )
add_subdirectory( widgets )
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

# only build koreport if possible
if (WITH_kchart AND SHOULD_BUILD_KCHART AND (SHOULD_BUILD_KEXI OR SHOULD_BUILD_KPLATO ) )
    add_subdirectory( koreport )
endif (WITH_kchart AND SHOULD_BUILD_KCHART AND (SHOULD_BUILD_KEXI OR SHOULD_BUILD_KPLATO ) )
