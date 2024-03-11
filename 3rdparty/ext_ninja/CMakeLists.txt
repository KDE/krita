SET(PREFIX_ext_ninja "${EXTPREFIX}" )

if (NOT CMAKE_CROSSCOMPILING OR APPLE)
ExternalProject_Add(ext_ninja
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    DOWNLOAD_NAME ninja-v1.11.0.tar.gz
    URL https://github.com/ninja-build/ninja/archive/v1.11.0.tar.gz
    URL_HASH SHA256=3c6ba2e66400fe3f1ae83deb4b235faf3137ec20bd5b08c29bfc368db143e4c6

    INSTALL_DIR ${PREFIX_ext_ninja}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ninja} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
else()
    set(_meson_prefix "${PREFIX_ext_ninja}")
    # Ninja needs to be brought manually.
    ExternalProject_Add(ext_ninja
        DOWNLOAD_COMMAND ""

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Installing ninja natively"
        BUILD_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONUSERBASE=${_meson_prefix}
            ${Python_EXECUTABLE} -m pip install --upgrade ninja==1.11.1 --user --ignore-installed --verbose
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env 
            PYTHONPATH=${_krita_pythonpath}
            ${Python_EXECUTABLE} -m pip show -f ninja

        UPDATE_COMMAND ""
    )
endif()
