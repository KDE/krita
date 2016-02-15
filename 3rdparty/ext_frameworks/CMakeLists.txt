SET(EXTPREFIX_frameworks "${EXTPREFIX}" )
#
# All needed frameworks:
#
#        Archive 
#        Config 
#        WidgetsAddons 
#        Completion
#        CoreAddons
#        GuiAddons 
#        I18n 
#        ItemModels 
#        ItemViews
#        WindowSystem
# On Linux:
#        KCrash

ExternalProject_Add(
    ext_extra_cmake_modules
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/extra-cmake-modules-5.17.0.zip
    URL_MD5 c302bb1771b7c366c6eeebf3742b2f54
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
)


ExternalProject_Add(
    ext_karchive
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/karchive-5.17.0.zip
    URL_MD5 d6bf0fcb6dbcffe1328b75f427593fd0
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_extra_cmake_modules
)


ExternalProject_Add(
    ext_kconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/kconfig-5.17.0.zip
    URL_MD5 f321e98ce591fdb3632e04c770a5b0c6
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_karchive
)


ExternalProject_Add(
    ext_kwidgetsaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/kwidgetsaddons-5.17.0.zip
    URL_MD5 bd32206b1973a3a13e7c405f135479c7
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kwidgetsaddons.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_kconfig
)


ExternalProject_Add(
    ext_kcompletion
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/kcompletion-5.17.0.zip
    URL_MD5 f853f34a8bb8433ced2d7c46148361ce
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_kwidgetsaddons
)


ExternalProject_Add(
    ext_kcoreaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/kcoreaddons-5.17.0.zip
    URL_MD5 75145cb1bc3a78fe4f13ce4383e76947
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/desktoptojson.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_kcompletion
)


ExternalProject_Add(
    ext_kguiaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/kguiaddons-5.17.0.zip
    URL_MD5 43b49b25d3ee9025d7172033d255e3b5
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_kcoreaddons
)


ExternalProject_Add(
    ext_ki18n
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/ki18n-5.17.0.zip
    URL_MD5 7d60380d09f98defbf878ea9daba0fbb
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/ki18n.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_gettext ext_kguiaddons
)


ExternalProject_Add(
    ext_kitemmodels
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/kitemmodels-5.17.0.zip
    URL_MD5 708504fd52db768093baac3666503bc3
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_ki18n
)

ExternalProject_Add(
    ext_kitemviews
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/kitemviews-5.17.0.zip
    URL_MD5 9280c5e7e669cd6241b62c5a6958202d
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_kitemmodels
)

ExternalProject_Add(
    ext_kimageformats
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/kimageformats-02-02-2016.tgz
    URL_MD5 09a923703832181455bc61d67c692040
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kimageformats.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_kitemviews
)

ExternalProject_Add(
    ext_kwindowsystem
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/kwindowsystem-5.17.0.zip
    URL_MD5 990c241d7fa88774ccd5176ec6bdcbdc
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kwindowsystem-x11.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_kimageformats
)


if (LINUX)
ExternalProject_Add(
    ext_kcrash
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/kcrash-5.17.0.zip
    URL_MD5 b6b865aff352d697928a9165c6564f8f
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DDEFINE_NO_DEPRECATED=1
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_kwindowsystem
)
endif()
