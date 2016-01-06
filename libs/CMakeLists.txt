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
    add_subdirectory( libglobal )
    add_subdirectory( koplugin )
    add_subdirectory( widgetutils )
    add_subdirectory( widgets )
    add_subdirectory( store )
    add_subdirectory( odf )
    add_subdirectory( flake )
    add_subdirectory( basicflakes )
    add_subdirectory( pigment )
    add_subdirectory( kundo2 )
    add_subdirectory( impex )
endif ()

if (SHOULD_BUILD_LIB_KOVECTORIMAGE)
    add_subdirectory( vectorimage )
endif ()
