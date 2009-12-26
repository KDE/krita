EXECUTE_PROCESS( COMMAND perl converter.pl generator_wword6.htm generator_wword8.htm
		  WORKING_DIRECTORY ${GENERATOR_DIR} )

SET( generated_files convert.cpp convert.h )
FOREACH( F ${generated_files} )
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different ${F} ../${F} WORKING_DIRECTORY ${GENERATOR_DIR} )
ENDFOREACH( F )
