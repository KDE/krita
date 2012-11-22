# Those paths need to be absolute since this file is included from other CMakeLists.txt files
set(libkdgantt_SRCS
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttglobal.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttview.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttstyleoptionganttitem.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttgraphicsview.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttabstractrowcontroller.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdgantttreeviewrowcontroller.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttlistviewrowcontroller.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttgraphicsscene.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttgraphicsitem.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttconstraint.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttconstraintproxy.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttconstraintgraphicsitem.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttitemdelegate.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttforwardingproxymodel.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttsummaryhandlingproxymodel.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttproxymodel.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttconstraintmodel.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttabstractgrid.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttdatetimegrid.cpp
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttlegend.cpp
  
  ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdgantttimescalezoomdialog.cpp
)

add_definitions(-DKDAB_NO_UNIT_TESTS)

# Generate moc_foo.cpp files since that's what the cpp files include (it works automatically with qmake, this way...)
# List of source files generated with "grep -l moc_ *.cpp | sed -e 's/\.cpp$//'"
# Plus kdganttgraphicsview_p
set(libkdgantt_mocable_files
  kdganttabstractgrid
  kdganttconstraintmodel
  kdganttconstraintproxy
  kdganttdatetimegrid
  kdganttforwardingproxymodel
  kdganttgraphicsscene
  kdganttgraphicsview
  kdganttitemdelegate
  kdganttproxymodel
  kdganttsummaryhandlingproxymodel
  kdganttview
  kdganttlegend
  kdgantttimescalezoomdialog
)
foreach(it ${libkdgantt_mocable_files})
  qt4_generate_moc(${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/${it}.h ${CMAKE_BINARY_DIR}/3rdparty/kdgantt/moc_${it}.cpp)
  macro_add_file_dependencies(${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/${it}.cpp ${CMAKE_BINARY_DIR}/3rdparty/kdgantt/moc_${it}.cpp)
endforeach(it)

qt4_generate_moc(${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttgraphicsview_p.h ${CMAKE_BINARY_DIR}/3rdparty/kdgantt/moc_kdganttgraphicsview_p.cpp)
macro_add_file_dependencies(${CMAKE_SOURCE_DIR}/3rdparty/kdgantt/kdganttgraphicsview.cpp ${CMAKE_BINARY_DIR}/3rdparty/kdgantt/moc_kdganttgraphicsview_p.cpp)
