SET(EXTPREFIX_kddockwidgets "${EXTPREFIX}" )
ExternalProject_Add( ext_kddockwidgets
    GIT_REPOSITORY https://github.com/KDAB/KDDockWidgets.git
    GIT_TAG v1.3.0

    INSTALL_DIR ${EXTPREFIX_kddockwidgets}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_kddockwidgets} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DKDDockWidgets_EXAMPLES=false  ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
