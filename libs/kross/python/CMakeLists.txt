

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/kross/api/
	${PYTHON_INCLUDE_PATH}
)



add_subdirectory( cxx )
add_subdirectory( scripts )


set(libkrosspythoncxx_SRCS 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/cxx/cxxsupport.cxx 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/cxx/cxx_extensions.cxx 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/cxx/cxxextensions.c 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/cxx/IndirectPythonInterface.cxx
	)

########### next target ###############

set(krosspython_PART_SRCS ${libkrosspythoncxx_SRCS}
   pythonextension.cpp 
   pythonmodule.cpp 
   pythonobject.cpp 
   pythonscript.cpp 
   pythonsecurity.cpp 
   pythoninterpreter.cpp )

kde4_automoc(${krosspython_PART_SRCS})

kde4_add_plugin(krosspython ${krosspython_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krosspython )

target_link_libraries(krosspython  ${KDE4_KDECORE_LIBS} krossapi krossmain ${PYTHON_LIBRARIES})

install(TARGETS krosspython  DESTINATION ${PLUGIN_INSTALL_DIR})


