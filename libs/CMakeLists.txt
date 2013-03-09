add_subdirectory( koplugin )
add_subdirectory( widgetutils )
add_subdirectory( widgets )
add_subdirectory( odf )
add_subdirectory( textlayout )
add_subdirectory( kotext )
add_subdirectory( flake )
add_subdirectory( basicflakes )
add_subdirectory( pigment )
add_subdirectory( main )
add_subdirectory( kundo2 )

if (SHOULD_BUILD_SCRIPTING)
    add_subdirectory( kokross )
endif (SHOULD_BUILD_SCRIPTING)

if (SHOULD_BUILD_LIB_VECTORIMAGE)
    add_subdirectory( vectorimage )
endif (SHOULD_BUILD_LIB_VECTORIMAGE)

if (SHOULD_BUILD_LIB_KOPAGEAPP)
    add_subdirectory( kopageapp )
endif (SHOULD_BUILD_LIB_KOPAGEAPP)

if(SHOULD_BUILD_LIB_KOPROPERTY)
    add_subdirectory( koproperty ) # TODO move to independent place like kdesupport
endif(SHOULD_BUILD_LIB_KOPROPERTY)

if (SHOULD_BUILD_LIB_CALLIGRADB)
    add_subdirectory( db ) # used by Kexi and Word's Biblio; TODO: remove when Predicate lib arrives
endif (SHOULD_BUILD_LIB_CALLIGRADB)

# only build koreport if possible
if (SHOULD_BUILD_LIB_KOREPORT)
    add_subdirectory( koreport )
endif (SHOULD_BUILD_LIB_KOREPORT)

if (Soprano_FOUND)
    add_subdirectory( rdf )
endif (Soprano_FOUND)
