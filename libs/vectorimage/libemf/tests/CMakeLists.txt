include_directories(
	${ENHMETAFILE_SOURCE_DIR}/tests/
	${CMAKE_CURRENT_BINARY_DIR}
	
)

link_directories(${ENHMETAFILE_BINARY_DIR})

set( snp_tests_bin_SRCS snp_tests.cpp )  
qt4_automoc( ${snp_tests_bin_SRCS} )
add_executable( snp_tests ${snp_tests_bin_SRCS} )
target_link_libraries( snp_tests Qt5::Gui Qt5::Test EnhMetaFile )
add_test( SnpTests ${CMAKE_CURRENT_BINARY_DIR}/snp_tests )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/snp-1.emf ${CMAKE_CURRENT_BINARY_DIR}/snp-1.emf COPYONLY )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/snp-2.emf ${CMAKE_CURRENT_BINARY_DIR}/snp-2.emf COPYONLY )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/snp-3.emf ${CMAKE_CURRENT_BINARY_DIR}/snp-3.emf COPYONLY )

set( no_such_bin_SRCS no_such.cpp )  
qt4_automoc( ${no_such_bin_SRCS} )
add_executable( no_such_file ${no_such_bin_SRCS} )
target_link_libraries( no_such_file Qt5::Gui Qt5::Test EnhMetaFile )
add_test( NoSuchFile ${CMAKE_CURRENT_BINARY_DIR}/no_such_file )

set( bad_header_bin_SRCS bad_header.cpp )  
qt4_automoc( ${bad_header_bin_SRCS} )
add_executable( bad_header ${bad_header_bin_SRCS} )
target_link_libraries( bad_header Qt5::Gui Qt5::Test EnhMetaFile )
add_test( BadHeader ${CMAKE_CURRENT_BINARY_DIR}/bad_header )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cof.wmf ${CMAKE_CURRENT_BINARY_DIR}/cof.wmf COPYONLY)

set( pyemf_tests_bin_SRCS pyemf_tests.cpp )  
qt4_automoc( ${pyemf_tests_bin_SRCS} )
add_executable( pyemf_tests ${pyemf_tests_bin_SRCS} )
target_link_libraries( pyemf_tests Qt5::Gui Qt5::Test EnhMetaFile )
add_test(PyEmfTests ${CMAKE_CURRENT_BINARY_DIR}/pyemf_tests)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-1.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-1.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-arc-chord-pie.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-arc-chord-pie.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-deleteobject.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-deleteobject.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-drawing1.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-drawing1.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-fontbackground.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-fontbackground.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-optimize16bit.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-optimize16bit.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-paths1.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-paths1.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-poly1.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-poly1.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-poly2.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-poly2.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-selectclippath1.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-selectclippath1.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-setpixel.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-setpixel.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-viewport-window-origin.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-viewport-window-origin.emf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyemf-worldtransform1.emf ${CMAKE_CURRENT_BINARY_DIR}/pyemf-worldtransform1.emf COPYONLY)

set( visio_tests_bin_SRCS visio_tests.cpp )  
qt4_automoc( ${visio_tests_bin_SRCS} )
add_executable( visio_tests ${visio_tests_bin_SRCS} )
target_link_libraries( visio_tests Qt5::Gui Qt5::Test EnhMetaFile )
add_test( VisioTests ${CMAKE_CURRENT_BINARY_DIR}/visio_tests )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/visio-1.emf ${CMAKE_CURRENT_BINARY_DIR}/visio-1.emf COPYONLY )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/visio-kde41.emf ${CMAKE_CURRENT_BINARY_DIR}/visio-kde41.emf COPYONLY )

set( render_bin_SRCS render.cpp )  
qt4_automoc( ${render_bin_SRCS} )
add_executable( render ${render_bin_SRCS} )
target_link_libraries( render Qt5::Gui EnhMetaFile )
add_test( QPainter-Render ${CMAKE_CURRENT_BINARY_DIR}/render )

