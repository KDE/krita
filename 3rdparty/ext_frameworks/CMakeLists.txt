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
    URL http://download.kde.org/stable/frameworks/5.60/extra-cmake-modules-5.60.0.zip
    URL_MD5 c205e17d89028ab0b034e68081d5ebfb
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
    URL http://download.kde.org/stable/frameworks/5.60/karchive-5.60.0.zip
    URL_MD5 50ae1495ef5b19898a06e43160d9b84d
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
    URL http://download.kde.org/stable/frameworks/5.60/kconfig-5.60.0.zip
    URL_MD5 db96fdd78a574d07b09abb720dae1a06
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kconfig.diff
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_karchive
)


ExternalProject_Add(
    ext_kwidgetsaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://download.kde.org/stable/frameworks/5.60/kwidgetsaddons-5.60.0.zip
    URL_MD5 1b09306fe482dd5665b16a243eb2be94
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
    URL http://download.kde.org/stable/frameworks/5.60/kcompletion-5.60.0.zip
    URL_MD5 a9848cb26ff96246769c56244f0db25f
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
    URL http://download.kde.org/stable/frameworks/5.60/kcoreaddons-5.60.0.zip
    URL_MD5 da21c1532042c2c8a7e56f444c581196
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
    URL http://download.kde.org/stable/frameworks/5.60/kguiaddons-5.60.0.zip
    URL_MD5 f8aeea516e264c80df470efafab6ded0
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
        URL http://download.kde.org/stable/frameworks/5.60/ki18n-5.60.0.zip
    URL_MD5 e53f479f22ea17629319fec710dda036
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
        URL http://download.kde.org/stable/frameworks/5.60/ki18n-5.60.0.zip
    URL_MD5 e53f479f22ea17629319fec710dda036
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
    URL http://download.kde.org/stable/frameworks/5.60/kitemmodels-5.60.0.zip
    URL_MD5 c8f02881a65d496d6114bb2155cdfc36
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
    URL http://download.kde.org/stable/frameworks/5.60/kitemviews-5.60.0.zip
    URL_MD5 9d94b8da72f43fac39aa782f7efd941e
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
    URL http://download.kde.org/stable/frameworks/5.60/kimageformats-5.60.0.zip
    URL_MD5 a8b413560d12cd956d689784a056d6e0
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
    URL http://download.kde.org/stable/frameworks/5.60/kwindowsystem-5.60.0.zip
    URL_MD5 c15c29141b5edca67143b735a94d3f43
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
    URL http://download.kde.org/stable/frameworks/5.60/kcrash-5.60.0.zip
    URL_MD5 7449a6f2b314b944ab2bed00124fe69b
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kwindowsystem
)
