

if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)

########### next target ###############


########### install files ###############

install( FILES  kis_tileddatamanager.h  DESTINATION ${INCLUDE_INSTALL_DIR})

