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
    URL http://download.kde.org/stable/frameworks/5.44/extra-cmake-modules-5.44.0.zip
    URL_MD5 74aa8fc501e27024390b01c81f2925eb
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
    URL http://download.kde.org/stable/frameworks/5.44/karchive-5.44.0.zip
    URL_MD5 c60a8e22b88cc7328610041638459689
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
    URL http://download.kde.org/stable/frameworks/5.44/kconfig-5.44.0.zip
    URL_MD5 d0223ea471bbf463ec42c2a2355a5183
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
    URL http://download.kde.org/stable/frameworks/5.44/kwidgetsaddons-5.44.0.zip
    URL_MD5 a9911d8d0f8aaf7a7afd84c41c8f80a1
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/kwidgetsaddons.diff
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
    URL http://download.kde.org/stable/frameworks/5.44/kcompletion-5.44.0.zip
    URL_MD5 0647885a702c338a1b656eb4f311ad16
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
    URL http://download.kde.org/stable/frameworks/5.44/kcoreaddons-5.44.0.zip
    URL_MD5 16a7379f3e2941d1c19d6f80939f15e8
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
    URL http://download.kde.org/stable/frameworks/5.44/kguiaddons-5.44.0.zip
    URL_MD5 440eefbf5abcafc492dcf857f7e4eaf5
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
        URL http://download.kde.org/stable/frameworks/5.44/ki18n-5.44.0.zip
    URL_MD5 333ab0a3f65a298e928d746144d4dc8e
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
        URL http://download.kde.org/stable/frameworks/5.44/ki18n-5.44.0.zip
    URL_MD5 333ab0a3f65a298e928d746144d4dc8e
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
    URL http://download.kde.org/stable/frameworks/5.44/kitemmodels-5.44.0.zip
    URL_MD5 ea43a5e2cc7033eb672796b108d7403b
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
    URL http://download.kde.org/stable/frameworks/5.44/kitemviews-5.44.0.zip
    URL_MD5 8b15c703313c7a790c7db897ef17de7d
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
    URL http://download.kde.org/stable/frameworks/5.44/kimageformats-5.44.0.zip
    URL_MD5 02a98b682f9cb655592148d7ebcc05e7
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
    URL http://download.kde.org/stable/frameworks/5.44/kwindowsystem-5.44.0.zip
    URL_MD5 75329f47cf8cd413fa1d15a57c298563
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
    URL http://download.kde.org/stable/frameworks/5.44/kcrash-5.44.0.zip
    URL_MD5 61adc0e125c65288968d958acf25f4aa
    INSTALL_DIR ${EXTPREFIX_frameworks}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} 
                ${GLOBAL_PROFILE}
                -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX} 
                -DBUILD_TESTING=false
    UPDATE_COMMAND ""
    DEPENDS ext_kwindowsystem
)
