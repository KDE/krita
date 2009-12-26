EXECUTE_PROCESS( COMMAND perl generate.pl generator_wword6.htm Word95 
		  WORKING_DIRECTORY ${GENERATOR_DIR} )

EXECUTE_PROCESS( COMMAND perl generate.pl generator_wword8.htm Word97 
		  WORKING_DIRECTORY ${GENERATOR_DIR} )

SET( generated_files word95_generated.h word95_generated.cpp word97_generated.h word97_generated.cpp )
FOREACH( F ${generated_files} )
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different ${F} ../${F} WORKING_DIRECTORY ${GENERATOR_DIR} )
ENDFOREACH( F )

SET( generated_tests word95_test.cpp word97_test.cpp )
FOREACH( T ${generated_tests} )
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different ${T} ../../tests/${T} WORKING_DIRECTORY ${GENERATOR_DIR} )
ENDFOREACH( T )
