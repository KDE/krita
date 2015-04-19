execute_process( COMMAND perl converter.pl generator_wword6.htm generator_wword8.htm
		  WORKING_DIRECTORY ${GENERATOR_DIR} )

set( generated_files convert.cpp convert.h )
foreach( F ${generated_files} )
  execute_process( COMMAND ${CMAKE_COMMAND} -E copy_if_different ${F} ../${F} WORKING_DIRECTORY ${GENERATOR_DIR} )
endforeach( F )
