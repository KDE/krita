execute_process( COMMAND perl generate.pl generator_wword6.htm Word95 
		  WORKING_DIRECTORY ${GENERATOR_DIR} )

execute_process( COMMAND perl generate.pl generator_wword8.htm Word97 
		  WORKING_DIRECTORY ${GENERATOR_DIR} )

set( generated_files word95_generated.h word95_generated.cpp word97_generated.h word97_generated.cpp )
foreach( F ${generated_files} )
  execute_process( COMMAND ${CMAKE_COMMAND} -E copy_if_different ${F} ../${F} WORKING_DIRECTORY ${GENERATOR_DIR} )
endforeach( F )

set( generated_tests word95_test.cpp word97_test.cpp )
foreach( T ${generated_tests} )
  execute_process( COMMAND ${CMAKE_COMMAND} -E copy_if_different ${T} ../../tests/${T} WORKING_DIRECTORY ${GENERATOR_DIR} )
endforeach( T )
