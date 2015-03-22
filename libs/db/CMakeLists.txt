add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44000)

# To hide all the warnings from embedded 3rd party software like icu
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    add_definitions(-Wno-unused-local-typedefs)
    add_definitions(-Wno-sign-compare)
    add_definitions(-Wno-unused-function)
    add_definitions(-Wno-unused-parameter)
endif ()

option(CALLIGRADB_DEBUG_GUI "Debugging GUI for CalligraDB" OFF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-calligradb.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-calligradb.h )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/libs
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${KOPLUGIN_INCLUDES}
    ${KDE4_INCLUDES}
    ${ICU_INCLUDE_DIRS}
)

add_subdirectory( drivers )

add_definitions( 
-D__KEXIDB__= 
-DYYERROR_VERBOSE=1
)

########### generate parser/lexer files ###############
# as described at http://public.kitware.com/pipermail/cmake/2002-September/003028.html

# Create target for the parser
add_custom_target(parser echo "Creating parser/lexer files")

# Create custom command for flex/lex (note the outputs)
add_custom_command(
COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/parser/generate_parser_code.sh
TARGET parser
OUTPUTS ${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlparser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/sqlparser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlscanner.h
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlscanner.cpp)

# mark files as generated
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlparser.h GENERATED)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlparser.cpp GENERATED)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlscanner.h GENERATED)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlscanner.cpp GENERATED)

set(calligradbparser_STAT_SRCS
   parser/sqlscanner.cpp 
   parser/sqlparser.cpp 
   parser/parser.cpp 
   parser/parser_p.cpp )

set(calligradb_LIB_SRCS
   ${calligradbparser_STAT_SRCS}
   pluginloader.cpp
   drivermanager.cpp 
   driver.cpp 
   driver_p.cpp 
   connection.cpp 
   keywords.cpp 
   object.cpp 
   field.cpp 
   utils.cpp 
   expression.cpp 
   connectiondata.cpp 
   RecordData.cpp
   fieldlist.cpp 
   tableschema.cpp 
   cursor.cpp 
   transaction.cpp 
   indexschema.cpp 
   queryschema.cpp 
   queryschemaparameter.cpp
   schemadata.cpp 
   global.cpp 
   relationship.cpp 
   roweditbuffer.cpp 
   msghandler.cpp 
   preparedstatement.cpp 
   dbproperties.cpp 
   admin.cpp 
   lookupfieldschema.cpp 
   simplecommandlineapp.cpp
   tableviewdata.cpp
   tableviewcolumn.cpp
   validator.cpp
)

kde4_add_library(calligradb SHARED ${calligradb_LIB_SRCS})
target_link_libraries(calligradb koplugin ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${ICU_I18N_LIBRARY})

target_link_libraries(calligradb LINK_INTERFACE_LIBRARIES koplugin ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
                                 ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

set_target_properties(calligradb PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )

if(MSVC)
   # avoid LNK1169 errors
   set_target_properties(calligradb PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif()

install(TARGETS calligradb ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES calligradb_driver.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

if(FALSE) # TODO: install when we move to independent place
   install( FILES  connection.h connectiondata.h cursor.h
    driver.h drivermanager.h error.h expression.h field.h fieldlist.h global.h object.h
    schemadata.h tableschema.h queryschema.h queryschemaparameter.h indexschema.h
    relationship.h parser/parser.h parser/sqlparser.h transaction.h preparedstatement.h
    RecordData.h utils.h
    calligradb_export.h calligradb_global.h DESTINATION ${INCLUDE_INSTALL_DIR}/calligradb COMPONENT Devel)
endif()

add_subdirectory( tests )
