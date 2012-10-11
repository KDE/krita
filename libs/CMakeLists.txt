add_subdirectory( koplugin )
add_subdirectory( widgets )
add_subdirectory( odf )
add_subdirectory( textlayout )
add_subdirectory( kotext )
add_subdirectory( flake )
add_subdirectory( pigment )
add_subdirectory( main )
add_subdirectory( kundo2 )

if (SHOULD_BUILD_SCRIPTING)
    add_subdirectory( kokross )
endif (SHOULD_BUILD_SCRIPTING)

if (NOT CREATIVEONLY)
    add_subdirectory( kopageapp )
endif (NOT CREATIVEONLY)

if(NOT TINY)
    add_subdirectory( koproperty ) # TODO move to independent place like kdesupport
endif(NOT TINY)

if (SHOULD_BUILD_CALLIGRADB)
    add_subdirectory( db ) # used by Kexi and Word's Biblio; TODO: remove when Predicate lib arrives
endif (SHOULD_BUILD_CALLIGRADB)

# only build koreport if possible
if (SHOULD_BUILD_KOREPORT)
    add_subdirectory( koreport )
endif (SHOULD_BUILD_KOREPORT)
