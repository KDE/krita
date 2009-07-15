include_directories(${KDE4_INCLUDES} 
    ${CMAKE_SOURCE_DIR}/libs/odf 
    ${CMAKE_SOURCE_DIR}/libs/kobase 
    ${CMAKE_SOURCE_DIR}/libs/widgets)

########### next target ###############

set(koinput_LIB_SRCS
    KoSliderCombo.cpp
    KoUnitDoubleSpinBox.cpp
)


kde4_add_library(koinput SHARED ${koinput_LIB_SRCS})

target_link_libraries(koinput     
    koodf
    kobase
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KFILE_LIBRARY}
)

set_target_properties(koinput PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS koinput ${INSTALL_TARGETS_DEFAULT_ARGS} )
