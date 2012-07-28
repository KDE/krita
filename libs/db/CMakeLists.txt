add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44000)

include_directories(${CMAKE_SOURCE_DIR}/libs ${CMAKE_CURRENT_SOURCE_DIR}/parser ${KDE4_INCLUDES})

add_subdirectory( drivers )

ADD_DEFINITIONS( 
-D__KEXIDB__= 
-DYYERROR_VERBOSE=1
)

########### generate parser/lexer files ###############
# as described at http://public.kitware.com/pipermail/cmake/2002-September/003028.html

# Create target for the parser
ADD_CUSTOM_TARGET(parser echo "Creating parser/lexer files")

# Create custom command for flex/lex (note the outputs)
ADD_CUSTOM_COMMAND(
COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/parser/generate_parser_code.sh
TARGET parser
OUTPUTS ${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlparser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/sqlparser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlscanner.h
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlscanner.cpp)

# mark files as generated
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlparser.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlparser.cpp GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlscanner.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/parser/sqlscanner.cpp GENERATED)

set(calligradbparser_STAT_SRCS
   parser/sqlscanner.cpp 
   parser/sqlparser.cpp 
   parser/parser.cpp 
   parser/parser_p.cpp )

set(calligradb_LIB_SRCS
   ${calligradbparser_STAT_SRCS}
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
)

kde4_add_library(calligradb SHARED ${calligradb_LIB_SRCS})
target_link_libraries(calligradb
   ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KPARTS_LIBS})

set_target_properties(calligradb PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )

if(MSVC)
   # avoid LNK1169 errors
   set_target_properties(calligradb PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif(MSVC)

install(TARGETS calligradb ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES calligradb_driver.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

if(FALSE) # TODO: install when we move to independent place
   install( FILES  connection.h connectiondata.h cursor.h
    driver.h drivermanager.h error.h expression.h field.h fieldlist.h global.h object.h
    schemadata.h tableschema.h queryschema.h queryschemaparameter.h indexschema.h
    relationship.h parser/parser.h parser/sqlparser.h transaction.h preparedstatement.h
    RecordData.h utils.h
    calligradb_export.h calligradb_global.h DESTINATION ${INCLUDE_INSTALL_DIR}/calligradb COMPONENT Devel)
endif(FALSE)
