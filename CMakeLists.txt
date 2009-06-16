project(koffice)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

if(KDE4_BUILD_TESTS)
    # only with this definition will the FOO_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

# define the generic version of the KOffice libraries here
# this makes it easy to advance it when the next KOffice release comes
set(GENERIC_KOFFICE_LIB_VERSION "5.0.0")
set(GENERIC_KOFFICE_LIB_SOVERSION "5")

cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)

# search packages used by KDE
set(KDE_MIN_VERSION "4.1.0")
find_package(KDE4 4.1.0 REQUIRED)
find_package(Qt4 4.5.0 REQUIRED)

include(KDE4Defaults)
include(MacroLibrary)

find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)


macro_optional_find_package(OpenEXR)
macro_bool_to_01(OPENEXR_FOUND HAVE_OPENEXR)
configure_file(config-openexr.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-openexr.h )


# NO_PIGMENT disabled everything that uses pigment in the crudest matter possible
if (NO_PIGMENT)
    add_definitions(-DNO_PIGMENT)
    set (FULL_PIGMENT FALSE)
    set (SMALL_PIGMENT FALSE)
else (NO_PIGMENT)

    include_directories(${CMAKE_SOURCE_DIR}/libs/pigment)

    # SMALL_PIGMENT provides just the classed needed to compile koffice libs, but not not Krita.
    if (SMALL_PIGMENT) 
        add_definitions(-DSMALL_PIGMENT)
    else (SMALL_PIGMENT)
        macro_optional_find_package(LCMS)
        set(REQUIRED_LCMS_VERSION 118)

        if(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
            # FULL_PIGMENT is the most desirable state, we can compile Krita!
            set(HAVE_REQUIRED_LCMS_VERSION TRUE)
            set(FULL_PIGMENT TRUE)
            add_definitions(-DFULL_PIGMENT)
        else(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
            set(HAVE_REQUIRED_LCMS_VERSION FALSE)
            set(SMALL_PIGMENT TRUE)
            set(FULL_PIGMENT FALSE)
        endif(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})

        macro_log_feature(HAVE_REQUIRED_LCMS_VERSION "LittleCMS" "Color management engine" "http://www.littlecms.com" FALSE "1.18" "Required by for color management and for Krita")

    endif (SMALL_PIGMENT)
endif (NO_PIGMENT)

macro_optional_find_package(OpenGL)
set(HAVE_OPENGL 0)

if(OPENGL_FOUND)
    message(STATUS "Found OpenGL: ${OPENGL_LIBRARIES}")
    if(QT_QTOPENGL_FOUND)
        message(STATUS "Found Qt OpenGL support")
        set(HAVE_OPENGL 1)
    else(QT_QTOPENGL_FOUND)
        message(STATUS "Did NOT find Qt OpenGL support. Check your Qt configuration")
    endif(QT_QTOPENGL_FOUND)
else(OPENGL_FOUND)
    message(STATUS "Did NOT find OpenGL libraries")
endif(OPENGL_FOUND)

configure_file(config-opengl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-opengl.h )

macro_log_feature(HAVE_OPENGL "OpenGL" "OpenGL support" "" FALSE "" "Required by parts of Krita and optionally by flake")

macro_optional_find_package(CreateResources)
macro_log_feature(CreateResources_FOUND "Create Resources" "Create Resources (brushes, palettes etc.)" "http://create.freedesktop.org" FALSE "" "Required by KOffice for optional extra resources.")

macro_optional_find_package(KdepimLibs)
macro_log_feature(KDEPIMLIBS_FOUND "KDE PIMLibs" "KDE Personal Information Management Libraries" "http://www.kde.org/" FALSE "" "Required by KPlato and the KDE address book integration (available as a module in KDE)")

macro_optional_find_package(Boost)
macro_log_feature(Boost_FOUND "Boost" "Boost C++ Libraries" "http://www.boost.org" FALSE "" "Required by KPresenter")

macro_optional_find_package(Eigen2)
macro_log_feature(EIGEN2_FOUND "Eigen" "C++ template library for linear algebra" "http://eigen.tuxfamily.org" FALSE "2.0" "Required by KSpread and Krita (available as a module in kdesupport)")

macro_optional_find_package(Blitz)
macro_log_feature(BLITZ_FOUND "QImageBlitz" "Image effect library" "http://www.kde.org" FALSE "" "Required by Krita (available in kdesupport)")

macro_optional_find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0" "Required for encrypted OpenDocument files support (available as a module in kdesupport)")

set(EXIV2_MIN_VERSION "0.16")
macro_optional_find_package(Exiv2)
macro_log_feature(EXIV2_FOUND "Exiv2" "Image metadata library and tools" "http://www.exiv2.org" FALSE "0.16" "Required by Krita")


if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)


#Set the build of TextShape changetraker

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})

if(WIN32)
    # detect oxygen icon dir at configure time based on KDEDIRS - there may be different package installation locations
    execute_process(COMMAND "${KDE4_KDECONFIG_EXECUTABLE}" --path icon OUTPUT_VARIABLE _dir ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    file(TO_CMAKE_PATH "${_dir}" __dir)
    find_path(KDE4_ICON_DIR oxygen PATHS
        ${__dir}
    )
    message(STATUS "using oxygen application icons from ${KDE4_ICON_DIR}")

    set(LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
                        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
                        LIBRARY ${INSTALL_TARGETS_DEFAULT_ARGS}
                        ARCHIVE ${INSTALL_TARGETS_DEFAULT_ARGS} )


else(WIN32)
    set (KDE4_ICON_DIR  ${CMAKE_INSTALL_PREFIX}/share/icons)
endif(WIN32)


include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)

configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )

# Look for OpenCTL (XXX: move this inside full_pigment? Or inside pigment?)
macro_optional_find_package(OpenCTL)

macro_log_feature(OPENCTL_FOUND "OpenCTL" "Free Color Transformation Language implementation" "http://www.openctl.org" FALSE "0.9.2" "Required for High Dynamic Range Color Spaces, YCbCr and LMS support")
configure_file(config-openctl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-openctl.h )

include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)
configure_file(config-endian.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-endian.h )

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

# koaction depends on threadweaver
set(KOACTION_INCLUDES ${CMAKE_SOURCE_DIR}/libs/koaction ${CMAKE_BINARY_DIR}/libs/koaction ${KDE4_INCLUDES})

# kostore is at the bottom of stack, so it has the dependency on the kde4 includes.
set(KOSTORE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/store ${CMAKE_BINARY_DIR}/libs/store ${KDE4_INCLUDES})

# kopicture depends on kostore
set(KOPICTURE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kopicture ${CMAKE_BINARY_DIR}/libs/kopicture ${KOSTORE_INCLUDES})

# koodf depends on kostore and kopicture
set(KOODF_INCLUDES ${CMAKE_SOURCE_DIR}/libs/odf ${CMAKE_BINARY_DIR}/libs/odf ${KOPICTURE_INCLUDES})


set(FLAKE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/flake 
                   ${CMAKE_SOURCE_DIR}/libs/odf 
                   ${CMAKE_SOURCE_DIR}/libs/kobase 
                   ${CMAKE_SOURCE_DIR}/libs/flake/commands 
                   ${CMAKE_SOURCE_DIR}/libs/flake/tools 
                   ${CMAKE_BINARY_DIR}/libs/flake)

# komain depends on kostore, koplugin, flake and koodf
set(KOMAIN_INCLUDES ${KDE4_INCLUDES}
                    ${CMAKE_SOURCE_DIR}/libs/flake
                    ${CMAKE_SOURCE_DIR}/libs/flake/pics
                    ${CMAKE_SOURCE_DIR}/libs/flake/commands
                    ${CMAKE_SOURCE_DIR}/libs/flake/tools
                    ${CMAKE_SOURCE_DIR}/libs/pigment
                    ${CMAKE_SOURCE_DIR}/libs/pigment/colorspaces
                    ${CMAKE_SOURCE_DIR}/libs/pigment/compositeops
                    ${CMAKE_SOURCE_DIR}/libs/pigment/colorprofiles
                    ${CMAKE_SOURCE_DIR}/libs/koproperties
                    ${CMAKE_SOURCE_DIR}/libs/store
                    ${CMAKE_SOURCE_DIR}/libs/kotext
                    ${CMAKE_SOURCE_DIR}/libs/kotext/changetracker
                    ${CMAKE_SOURCE_DIR}/libs/kotext/styles
                    ${CMAKE_SOURCE_DIR}/libs/kotext/kohyphen
                    ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument
                    ${CMAKE_SOURCE_DIR}/libs/widgets
                    ${CMAKE_SOURCE_DIR}/libs/widgets/progress
                    ${CMAKE_SOURCE_DIR}/libs/widgets/linestyleselector
                    ${CMAKE_SOURCE_DIR}/libs/widgets/aspectbutton
                    ${CMAKE_SOURCE_DIR}/libs/widgets/openpane
                    ${CMAKE_SOURCE_DIR}/libs/widgets/colorwidgets
                    ${CMAKE_SOURCE_DIR}/libs/widgets/resources
                    ${CMAKE_SOURCE_DIR}/libs/widgets/pagelayout
                    ${CMAKE_SOURCE_DIR}/libs/widgets/characterselectiondialog
                    ${CMAKE_SOURCE_DIR}/libs/widgets/csvimportdialog
                    ${CMAKE_SOURCE_DIR}/libs/widgets/documentsectionbox
                    ${CMAKE_SOURCE_DIR}/libs/widgets/zoom
                    ${CMAKE_SOURCE_DIR}/libs/widgets/input
                    ${CMAKE_SOURCE_DIR}/libs/kopicture
                    ${CMAKE_SOURCE_DIR}/libs/kobase
                    ${CMAKE_SOURCE_DIR}/libs/koaction
                    ${CMAKE_SOURCE_DIR}/libs/main
                    ${CMAKE_SOURCE_DIR}/libs/resources
                    ${CMAKE_SOURCE_DIR}/libs/odf
                    ${CMAKE_SOURCE_DIR}/libs/koplugin
 )                   
 
 # pigment depends on koplugin and lcms
 set(PIGMENT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/koplugin ${CMAKE_SOURCE_DIR}/libs/pigment ${CMAKE_SOURCE_DIR}/libs/pigment/colorspaces ${CMAKE_SOURCE_DIR}/libs/pigment/colorprofiles ${LCMS_INCLUDE_DIR} ${KDE4_INCLUDES} )
 
 # koresources depends on pigment
 set(KORESOURCES_INCLUDES ${CMAKE_SOURCE_DIR}/libs/resources
 			 ${CMAKE_BINARY_DIR}/libs/resources 
 			 ${PIGMENT_INCLUDES} 
			 ${KDE4_INCLUDES})

# KoText depends on store, odf
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext/styles
                    ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument
                    ${CMAKE_SOURCE_DIR}/libs/kotext
                    ${CMAKE_SOURCE_DIR}/libs/kotext/changetracker
                    ${CMAKE_BINARY_DIR}/libs/kotext)

set(KFORMULA_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kformula
		      ${CMAKE_BINARY_DIR}/libs/kformula)

set(KOKROSS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kokross ${CMAKE_BINARY_DIR}/libs/kokross)

# kopageapp
set(KOPAGEAPP_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kopageapp ${CMAKE_SOURCE_DIR}/libs/kopageapp/commands ${CMAKE_BINARY_DIR}/libs/kopageapp )

set(KOPROPERTIES_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kobase ${CMAKE_BINARY_DIR}/libs/kobase)

add_subdirectory(libs)
add_subdirectory(interfaces)
add_subdirectory(cmake)

set(SHOULD_BUILD_KARBON TRUE)
if (NO_PIGMENT)
    set(SHOULD_BUILD_KARBON FALSE)
endif (NO_PIGMENT)
macro_optional_add_subdirectory(karbon)

if(EIGEN2_FOUND)
    macro_optional_add_subdirectory(kspread)
    if(BUILD_kspread)
        set(SHOULD_BUILD_KSPREAD TRUE)
    else(BUILD_kspread)
        set(SHOULD_BUILD_KSPREAD FALSE)
    endif(BUILD_kspread)
else(EIGEN2_FOUND)
    set(SHOULD_BUILD_KSPREAD FALSE)
endif(EIGEN2_FOUND)

#Check if build kpresenter here to be able to test if we can build filters/kpresenter
if(Boost_FOUND)
    set(SHOULD_BUILD_KPRESENTER TRUE)
    macro_optional_add_subdirectory(kpresenter)
else(Boost_FOUND)
    set(SHOULD_BUILD_KPRESENTER FALSE)
endif(Boost_FOUND)

FIND_PROGRAM(BZIP2_EXECUTABLE NAMES bzip2 )
macro_log_feature(BZIP2_EXECUTABLE "bzip2" "High-quality data compressor" "http://www.bzip.org" FALSE "" "Required by Kexi")

if(BUILD_KEXI)
    if( BZIP2_EXECUTABLE )
        macro_optional_add_subdirectory(kexi)
    endif( BZIP2_EXECUTABLE )
endif(BUILD_KEXI)

if(KdepimLibs_FOUND)
    macro_optional_add_subdirectory(kplato)
endif(KdepimLibs_FOUND)


if(WIN32 OR SMALL_PIGMENT OR NO_PIGMENT OR NOT BLITZ_FOUND OR NOT EIGEN2_FOUND OR NOT EXIV2_FOUND)
   set(SHOULD_BUILD_KRITA FALSE)
else(WIN32 OR SMALL_PIGMENT OR NO_PIGMENT OR NOT BLITZ_FOUND OR NOT EIGEN2_FOUND OR NOT EXIV2_FOUND)
    set(SHOULD_BUILD_KRITA TRUE)
    macro_optional_add_subdirectory(krita)
endif(WIN32 OR SMALL_PIGMENT OR NO_PIGMENT OR NOT BLITZ_FOUND OR NOT EIGEN2_FOUND OR NOT EXIV2_FOUND)


macro_optional_add_subdirectory(kword)
macro_optional_add_subdirectory(kformula)
macro_optional_add_subdirectory(kivio)
macro_optional_add_subdirectory(kounavail)
macro_optional_add_subdirectory(kdgantt)
macro_optional_add_subdirectory(kchart)
macro_optional_add_subdirectory(doc)

add_subdirectory(pics)
add_subdirectory(plugins)
add_subdirectory(servicetypes)
add_subdirectory(templates)
add_subdirectory(tools)
add_subdirectory(filters)

MACRO_DISPLAY_FEATURE_LOG()

ADD_CUSTOM_TARGET(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

