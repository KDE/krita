include_directories( ${CMAKE_SOURCE_DIR}/plugins/formulashape
                     ${CMAKE_SOURCE_DIR}/plugins/formulashape/elements
                     ${QT_INCLUDES}
                     ${KOMAIN_INCLUDES}
)

########### KFormula Library ###############

set( kformulalib_PART_SRCS
   AttributeManager.cpp
   FormulaRenderer.cpp
   FormulaData.cpp
   FormulaCursor.cpp
   FormulaEditor.cpp
   FormulaCommand.cpp
   ElementFactory.cpp 
   Dictionary.cpp

   elements/BasicElement.cpp
   elements/FixedElement.cpp
   elements/RowElement.cpp
   elements/FormulaElement.cpp 
   elements/TextElement.cpp
   elements/FractionElement.cpp
   elements/SpaceElement.cpp
   elements/GlyphElement.cpp
   elements/IdentifierElement.cpp
   elements/OperatorElement.cpp
   elements/MultiscriptElement.cpp
   elements/SubSupElement.cpp
   elements/UnderOverElement.cpp
   elements/FencedElement.cpp 
   elements/TableElement.cpp
   elements/TableRowElement.cpp
   elements/TableDataElement.cpp
   elements/RootElement.cpp
   elements/EncloseElement.cpp
   elements/ActionElement.cpp
   elements/PaddedElement.cpp
   elements/ErrorElement.cpp
   elements/StyleElement.cpp
   elements/TokenElement.cpp
   elements/NumberElement.cpp
   elements/StringElement.cpp
   elements/PhantomElement.cpp
   elements/SquareRootElement.cpp
   elements/AnnotationElement.cpp
   elements/UnknownElement.cpp
)

kde4_add_library(kformulalib SHARED ${kformulalib_PART_SRCS})

target_link_libraries(kformulalib komain)
target_link_libraries(kformulalib LINK_INTERFACE_LIBRARIES komain)

set_target_properties(kformulalib PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )

install(TARGETS kformulalib ${INSTALL_TARGETS_DEFAULT_ARGS})

########### FormulaShape Plugin ###############

find_package(Cauchy)

macro_optional_find_package(Cauchy)
macro_log_feature(M2MML_FOUND "Cauchy's M2MML" "Matlab/Octave to MathML compiler" "https://bitbucket.org/cyrille/cauchy" FALSE "" "Required for the matlab/octave formula tool")

if(M2MML_FOUND)
    set(M2MML_TOOL_LIBRARIES ${M2MML_LIBRARY})
    include_directories(${CAUCHY_INCLUDE_DIR})
    add_definitions(-DHAVE_M2MML)
endif()

set( formulashape_PART_SRCS
   KoFormulaShapePlugin.cpp
   KoFormulaShape.cpp
   FormulaCommandUpdate.cpp
   KoFormulaShapeFactory.cpp
   KoFormulaTool.cpp
   FormulaCommand.cpp
   KoFormulaToolFactory.cpp
   FormulaToolWidget.cpp
   FormulaDocument.cpp
   FormulaPart.cpp
   KoM2MMLForumulaTool.cpp
   # itexToMML files
   itexToMML/lex.yy.c
   itexToMML/y.tab.c
)

kde4_add_ui_files( formulashape_PART_SRCS
    FormulaToolWidget.ui
)

kde4_add_plugin(formulashape ${formulashape_PART_SRCS})

target_link_libraries(formulashape kformulalib ${M2MML_TOOL_LIBRARIES})


#set_target_properties(kformulalib PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )
install(TARGETS formulashape  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES formulashape.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory( pics ) 
add_subdirectory( fonts ) 
add_subdirectory( tests )
add_subdirectory( templates )

install( FILES  kformulapart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

