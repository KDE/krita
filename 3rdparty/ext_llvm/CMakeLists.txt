SET(EXTPREFIX_llvm "${EXTPREFIX}" )
ExternalProject_Add(
    ext_llvm
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    # Our llvm has the bindings ported to Python3
    URL http://files.kde.org/krita/build/dependencies/llvm-3.9.0.src.tgz
    URL_MD5 c3e8988ca50086663a4869fce9d02b61
    INSTALL_DIR ${EXTPREFIX_llvm}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_llvm}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
               -DPYTHON_EXECUTABLE=${EXTPREFIX_llvm}/bin/python3
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/tools/clang/bindings/python/clang ${PREFIX_ext_llvm}/lib/python3.5
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_python
)

