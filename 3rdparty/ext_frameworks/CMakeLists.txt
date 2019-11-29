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
#        kimageformats
# On Linux:
#        KCrash

ExternalProject_Add(
    ext_extra_cmake_modules
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/extra-cmake-modules-5.64.0.zip
    URL_MD5 e8fa4bba6a534feb9d9e39db036923da
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/ecm_install_to_share.diff
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
)


ExternalProject_Add(
    ext_karchive
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/karchive-5.64.0.zip
    URL_MD5 26c7f89bfde97bc85f887f81a2d0f648
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/karchive.diff
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_extra_cmake_modules
)


ExternalProject_Add(
    ext_kconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/kconfig-5.64.0.zip
    URL_MD5 52858f7fdcd17d0c5680acf96dcaae9a
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kconfig.diff
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
               ${GLOBAL_PROFILE}
               -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
               -DBUILD_TESTING=false
               -DKCONFIG_USE_DBUS=off
    UPDATE_COMMAND ""
    DEPENDS ext_karchive
)


ExternalProject_Add(
    ext_kwidgetsaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/kwidgetsaddons-5.64.0.zip
    URL_MD5 d6578f8b5b4bf19fddb8c16918ec0117
    INSTALL_DIR ${EXTPREFIX_frameworks}
#    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kwidgetsaddons.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kconfig
)


ExternalProject_Add(
    ext_kcompletion
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/kcompletion-5.64.0.zip
    URL_MD5 21fef280c2580da83df289707326d42a
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kwidgetsaddons
)


ExternalProject_Add(
    ext_kcoreaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/kcoreaddons-5.64.0.zip
    URL_MD5 670a74cda110da89d1a7b0fd832fca42
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/desktoptojson.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcompletion
)


ExternalProject_Add(
    ext_kguiaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/kguiaddons-5.64.0.zip
    URL_MD5 565dcb39e812463100dddaf3f02bd2a0
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kcoreaddons
)

if(APPLE)
ExternalProject_Add(
    ext_ki18n
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://download.kde.org/stable/frameworks/5.64/ki18n-5.64.0.zip
    URL_MD5 75a86675bf2b352b53cbcaece956b486
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/ki18n-appdatalocation.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kguiaddons ext_gettext
)
else()
ExternalProject_Add(
    ext_ki18n
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://download.kde.org/stable/frameworks/5.64/ki18n-5.64.0.zip
    URL_MD5 75a86675bf2b352b53cbcaece956b486
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/ki18n-appdatalocation.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kguiaddons 
)
endif()

ExternalProject_Add(
    ext_kitemmodels
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/kitemmodels-5.64.0.zip
    URL_MD5 e561031cafe7af08de3c62f01fa154b2
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_ki18n
)

ExternalProject_Add(
    ext_kitemviews
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/kitemviews-5.64.0.zip
    URL_MD5 bbd20e563e3f3cbc80252ba89d27962c
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kitemmodels
)

ExternalProject_Add(
    ext_kimageformats
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/kimageformats-5.64.0.zip
    URL_MD5 744090cf90ddde84f51dcabd167dd607
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kimageformats.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kitemviews
)

ExternalProject_Add(
    ext_kwindowsystem
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/kwindowsystem-5.64.0.zip
    URL_MD5 4e2819edb9d029ff33791d691637065a
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kwindowsystem-x11.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kimageformats
)

ExternalProject_Add(
    ext_kcrash
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.64/kcrash-5.64.0.zip
    URL_MD5 2d4d9c074c2e1d19804618d3f291a568
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kwindowsystem
)
