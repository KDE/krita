SET(EXTPREFIX_libintl-lite  "${EXTPREFIX}")

ExternalProject_Add(
    ext_libintl-lite
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://github.com/j-jorge/libintl-lite.git
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_libintl-lite}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
    UPDATE_COMMAND ""
)
