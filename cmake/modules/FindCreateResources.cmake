if(CREATERESOURCES_INCLUDE_DIR)

  set(CREATERESOURCES_FOUND TRUE)

else(CREATERESOURCES_INCLUDE_DIR)

    find_path( CREATERESOURCES_INCLUDE_DIR gradients/karbon14/simple.kgr
               PATHS
               ${CMAKE_INSTALL_PREFIX}
               /usr /usr/local /usr/pkg
               /opt /opt/local /opt/csw
               PATH_SUFFIXES share/create/ )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(CreateResources DEFAULT_MSG
                                      CREATERESOURCES_INCLUDE_DIR)

endif(CREATERESOURCES_INCLUDE_DIR)
