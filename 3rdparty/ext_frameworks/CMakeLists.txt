SET(EXTPREFIX_frameworks "${EXTPREFIX}" )
#
# All needed frameworks:
#
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
    URL https://download.kde.org/stable/frameworks/5.101/extra-cmake-modules-5.101.0.tar.xz
    URL_HASH SHA256=8c4c561310db587d390a6c84afc97e1addbaddd73b9d7a4c7309c5da9b9bc8f2

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Android-Skip-setting-up-fastlane.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Android-Recursively-find-dependency-of-dependencies.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Revert-Use-BIN_INSTALL_DIR-data-for-DATAROOTDIR-on-W.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-ANDROID-Find-dependencies-of-manually-specified-libr.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""

    DEPENDS ${QT_DEP}
)

ExternalProject_Add(
    ext_kconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/kconfig-5.101.0.tar.xz
    URL_HASH SHA256=ca747e485756e6d65e4eed85d1187c80b7facc958f379a68fbe862f068cb5383

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Patch-kconfig-to-save-the-config-to-AppConfigLocatio.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Work-around-CMake-multi-config-generators.patch
        # TODO: remove when bumped to 5.102
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Android-Fix-a-bug-where-new-file-wouldn-t-be-created.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF -DKCONFIG_USE_DBUS=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_extra_cmake_modules
)

set (_karchive_extra_flags "")
if (ANDROID)
    set(_karchive_extra_flags "-DCMAKE_DISABLE_FIND_PACKAGE_PkgConfig=1")
endif()

ExternalProject_Add(
    ext_karchive
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/karchive-5.101.0.tar.xz
    URL_HASH SHA256=58bcb6c61bc4a3fe48a0cbeaf392d42b6b8d33e1bc4c51faf24db0912ecfa9b0

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE} ${_karchive_extra_flags}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_extra_cmake_modules
)

ExternalProject_Add(
    ext_kwidgetsaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/kwidgetsaddons-5.101.0.tar.xz
    URL_HASH SHA256=78913d3303e94329db964bf072b9e565bd582fbbd026310dce6e994c8d083823

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_kconfig
)

ExternalProject_Add(
    ext_kcompletion
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/kcompletion-5.101.0.tar.xz
    URL_HASH SHA256=d80f6fbe23285f9f914ed67ffd2a825d36850c086121666bbb62257126553f30

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_kwidgetsaddons
)

ExternalProject_Add(
    ext_kcoreaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/kcoreaddons-5.101.0.tar.xz
    URL_HASH SHA256=77b64fc17f7ebbbd53bbb3614b4c3c73a0a9b21607e96e6abb2823ef1972b72a

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_kcompletion
)


ExternalProject_Add(
    ext_kguiaddons
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/kguiaddons-5.101.0.tar.xz
    URL_HASH SHA256=9894e4193b3cb7b4ed5e3a591cf12043fb87683b09db77e465aa4453c7c757b7

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-geo-scheme-handler-Don-t-make-it-into-a-bundle-i.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF -DWITH_WAYLAND=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_kcoreaddons
)

if (ANDROID)
    set (KI18N_DEP ext_libintl-lite)
elseif (APPLE)
    set (KI18N_DEP ${GETTEXT_DEP})
endif()

ExternalProject_Add(
    ext_ki18n
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/ki18n-5.101.0.tar.xz
    URL_HASH SHA256=bf1530be9279d476c0531988eeb1c032e208f6010c8f00880bb5d694229fef65
    INSTALL_DIR ${EXTPREFIX_frameworks}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-ki18n-fix-loading-catalogs-with-patched-gettext.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Ensure-langenv-string-is-null-terminated.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Patch-the-location-where-ki18n-looks-for-catalogs.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_kguiaddons ${KI18N_DEP}
)

ExternalProject_Add(
    ext_kitemmodels
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/kitemmodels-5.101.0.tar.xz
    URL_HASH SHA256=5aaee068a8d0c85dfc50c5ade2c5eb913c5936f68e902c302c36bbc2ac70f426

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_ki18n
)

ExternalProject_Add(
    ext_kitemviews
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/kitemviews-5.101.0.tar.xz
    URL_HASH SHA256=aca259eeff2dde1434cdf3ff3d979e9b17dd55bad15dd7341f92700cf8418fb1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_kitemmodels
)

ExternalProject_Add(
    ext_kimageformats
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/kimageformats-5.101.0.tar.xz
    URL_HASH SHA256=ae19b07141fdf977035e6decc50fa45855c0e635f57333a2443d3c24464c625f

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-comment-out-jxl-0.9.0-API-change.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE} -DKIMAGEFORMATS_JXL=OFF

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_karchive ext_kitemviews
)

ExternalProject_Add(
    ext_kwindowsystem
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/kwindowsystem-5.101.0.tar.xz
    URL_HASH SHA256=ef6b9e733e320be63a4557da0453b1a8ea81519e01c1737eff69b66401302eb8

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_kimageformats
)

ExternalProject_Add(
    ext_kcrash
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/frameworks/5.101/kcrash-5.101.0.tar.xz
    URL_HASH SHA256=6dd10951db01918681a8ce6e9e3a99d3f8b5170d2c5a0b5212007e68db0c937a

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${QT_DEP} ext_kwindowsystem
)
