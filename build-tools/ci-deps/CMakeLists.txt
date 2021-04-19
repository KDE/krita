project (prebuilt-deps-for-krita)

cmake_minimum_required(VERSION 3.7.0)

if (NOT EXTERNALS_DOWNLOAD_DIR)
   set (EXTERNALS_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/download)
endif()

include (ExternalProject)

ExternalProject_Add(
       ext_cideps

       DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
       DOWNLOAD_NO_EXTRACT TRUE

       URL https://binary-factory.kde.org/job/Krita_Nightly_Windows_Dependency_Build/lastSuccessfulBuild/artifact/krita-deps.zip
#       URL_MD5 884d65fb6ed9777966370ad5eb24f2c7

       CONFIGURE_COMMAND ""
       BUILD_COMMAND ""
       INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX} &&
                       ${CMAKE_COMMAND} -E tar xfz ${EXTERNALS_DOWNLOAD_DIR}/krita-deps.zip &&
                       ${CMAKE_COMMAND} -E copy_directory deps-install ${CMAKE_INSTALL_PREFIX} &&
                       ${CMAKE_COMMAND} -E rm -rf deps-install
       UPDATE_COMMAND ""
   )