if (SHOULD_BUILD_LIB_CALLIGRA)
    add_subdirectory( version )
    add_subdirectory( koplugin )
#    add_subdirectory( widgetutils )
#    add_subdirectory( widgets )
#    add_subdirectory( odf )
#    add_subdirectory( textlayout )
#    add_subdirectory( kotext )
#    add_subdirectory( flake )
#    add_subdirectory( basicflakes )
    add_subdirectory( pigment )
#    add_subdirectory( main )
#    add_subdirectory( kundo2 )
endif ()

if (SHOULD_BUILD_FEATURE_SCRIPTING)
    add_subdirectory( kokross )
endif ()

if (SHOULD_BUILD_LIB_KOVECTORIMAGE)
    add_subdirectory( vectorimage )
endif ()

if (SHOULD_BUILD_LIB_KOPAGEAPP)
    add_subdirectory( kopageapp )
endif ()

if(SHOULD_BUILD_LIB_KOPROPERTY)
    add_subdirectory( koproperty ) # TODO move to independent place like kdesupport
endif()

if (SHOULD_BUILD_LIB_CALLIGRADB)
    add_subdirectory( db ) # used by Kexi and Word's Biblio; TODO: remove when Predicate lib arrives
endif ()

# only build koreport if possible
if (SHOULD_BUILD_LIB_KOREPORT)
    add_subdirectory( koreport )
endif ()

if (SHOULD_BUILD_FEATURE_RDF)
    add_subdirectory( rdf )
endif ()
