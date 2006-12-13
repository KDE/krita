include_directories(
	${PYTHON_INCLUDE_PATH}
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

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

# -ldl seems to be needed (on Linux at least).
# see http://lists.kde.org/?l=koffice-devel&m=116333261328694&w=2
# and http://websvn.kde.org/branches/work/kwin_composite/CMakeLists.txt?rev=603677&r1=603384&r2=603677
target_link_libraries(krosspython ${KDE4_KROSSCORE_LIBS} ${PYTHON_LIBRARIES} ${CMAKE_DL_LIBS})

install(TARGETS krosspython  DESTINATION ${PLUGIN_INSTALL_DIR})

