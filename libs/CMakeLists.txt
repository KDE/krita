add_definitions(
  -DTRANSLATION_DOMAIN=\"calligra\"
  -DQT_USE_QSTRINGBUILDER
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
  -DQT_USE_FAST_CONCATENATION
)

if (SHOULD_BUILD_LIB_CALLIGRA)
    add_subdirectory( version )
    add_subdirectory( koplugin )
    add_subdirectory( widgetutils )
    add_subdirectory( widgets )
    add_subdirectory( store )
    add_subdirectory( odf )
    add_subdirectory( textlayout )
    add_subdirectory( kotext )
    add_subdirectory( flake )
    add_subdirectory( basicflakes )
    add_subdirectory( pigment )
    add_subdirectory( kundo2 )
endif ()

if (SHOULD_BUILD_LIB_KOMAIN)
    add_subdirectory( main )
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

if (SHOULD_BUILD_FEATURE_RDF)
    add_subdirectory( rdf )
endif ()
