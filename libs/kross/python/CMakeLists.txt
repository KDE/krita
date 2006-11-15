include_directories(
    ${CMAKE_SOURCE_DIR}/libs/kross/core/
	${PYTHON_INCLUDE_PATH}
)

add_subdirectory( cxx )
##add_subdirectory( scripts )

set(libkrosspythoncxx_SRCS 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/cxx/cxxsupport.cxx 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/cxx/cxx_extensions.cxx 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/cxx/cxxextensions.c 
    ${CMAKE_SOURCE_DIR}/libs/kross/python/cxx/IndirectPythonInterface.cxx
)

########### next target ###############

set(krosspython_PART_SRCS ${libkrosspythoncxx_SRCS}
   pythonvariant.cpp
   pythonfunction.cpp
   pythonextension.cpp
   pythonmodule.cpp
   pythonscript.cpp
   pythoninterpreter.cpp
)

kde4_automoc(${krosspython_PART_SRCS})

kde4_add_plugin(krosspython ${krosspython_PART_SRCS})


# -ldl seems to be needed.
# see http://lists.kde.org/?l=koffice-devel&m=116333261328694&w=2
# and http://websvn.kde.org/branches/work/kwin_composite/CMakeLists.txt?rev=603677&r1=603384&r2=603677
target_link_libraries(krosspython krosscore ${PYTHON_LIBRARIES} -ldl)

install(TARGETS krosspython  DESTINATION ${PLUGIN_INSTALL_DIR})

