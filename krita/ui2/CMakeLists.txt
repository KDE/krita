add_subdirectory(tests)

########### next target ###############

set(kritapart_LIB_SRCS
    kis_canvas.cpp
    kis_opengl_canvas.cpp
    kis_qpainter_canvas.cpp
    kis_view_converter.cpp
    kis_view.cpp
)

kde4_automoc(${kritapart_LIB_SRCS})

kde4_add_library(kritapart SHARED ${kritapart_LIB_SRCS})

target_link_libraries(kritapart ${KDE4_KDECORE_LIBS} kritaimage kofficecore kopainter flake)

if(HAVE_OPENGL)
    target_link_libraries(kritapart
        ${OPENGL_LIBRARIES}
        ${QT_QTOPENGL_LIBRARY}
        )
endif(HAVE_OPENGL)

set_target_properties(kritapart PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritapart  DESTINATION ${LIB_INSTALL_DIR})


