add_subdirectory( pics ) 
add_subdirectory( fonts ) 

include_directories(${QT_INCLUDES} ${KOFFICEUI_INCLUDES} )

set( formulashape_PART_SRCS
   KoFormulaShapePlugin.cpp
   KoFormulaShape.cpp
   KoFormulaShapeFactory.cpp
   KoFormulaTool.cpp
   KoFormulaToolFactory.cpp
   AttributeManager.cpp
   FormulaRenderer.cpp
   FormulaCursor.cpp 
   BasicElement.cpp 
#   SequenceElement.cpp
   FormulaElement.cpp 
#   MultiscriptElement.cpp
#   UnderOverElement.cpp
#   TextElement.cpp 
#   BracketElement.cpp 
#   MatrixElement.cpp
#   MatrixRowElement.cpp
#   MatrixEntryElement.cpp
#   FractionElement.cpp
#   RootElement.cpp
#   SpaceElement.cpp
#   EncloseElement.cpp
#   ActionElement.cpp
#   PaddedElement.cpp
#   GlyphElement.cpp
#   ErrorElement.cpp
#   TokenStyleElement.cpp
#   StyleElement.cpp
#   TokenElement.cpp
#   NumberElement.cpp
#   IdentifierElement.cpp
#   OperatorElement.cpp
   OperatorDictionary.cpp
   Entities.cpp
   ElementFactory.cpp 
   MatrixDialog.cc 
#   contextstyle.cc 
   symboltable.cc 
   kformulaconfigpage.cc 
   symbolaction.cc 
#   fontstyle.cc 
)

kde4_automoc(${formulashape_PART_SRCS})

kde4_add_plugin(formulashape ${formulashape_PART_SRCS})

target_link_libraries(formulashape  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kofficeui)

#set_target_properties(kformulalib PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS formulashape  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES formulashape.desktop DESTINATION ${SERVICES_INSTALL_DIR})

