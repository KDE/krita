SET(PREFIX_ext_seexpr "${EXTPREFIX}" )
ExternalProject_Add(
    ext_seexpr
    GIT_REPOSITORY https://invent.kde.org/graphics/seexpr.git
    GIT_TAG b807bfebfec9edc4197ca3561f53625895edc96e

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_seexpr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DFLAVOR=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DBOOST_INCLUDE_DIR=${PREFIX_ext_seexpr}/include -DBOOST_ROOT=${PREFIX_ext_seexpr} -DBOOST_LIB_DIR=${PREFIX_ext_seexpr}/lib -DENABLE_LLVM_BACKEND=OFF -DENABLE_SSE4=OFF -DBUILD_DOC=OFF -DBUILD_TESTS=OFF -DBUILD_UTILS=OFF -DUSE_PYTHON=OFF

    UPDATE_COMMAND ""
    # TODO when moving this library to a packaged release, please add ext_extra_cmake_modules so that translations can be built in
    DEPENDS ext_qt
)
