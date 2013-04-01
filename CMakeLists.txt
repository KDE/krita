project(calligra)

cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

cmake_policy(SET CMP0002 OLD)

#cmake_policy CMP0017 was introduced in version 2.8.4
if(${CMAKE_VERSION} VERSION_GREATER 2.8.3)
    cmake_policy(SET CMP0017 NEW)
endif()

 ############
#############
## Options ##
#############
############

 ######################
#######################
## Constants defines ##
#######################
######################

# define the generic version of the Calligra libraries here
# this makes it easy to advance it when the next Calligra release comes
set(GENERIC_CALLIGRA_LIB_VERSION "12.0.0")
set(GENERIC_CALLIGRA_LIB_SOVERSION "12")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )


 ####################
#####################
## Product defines ##
#####################
####################

# There are currently 4 product sets predefined:
# "ACTIVE", "CREATIVE", "DESKTOP", "ALL" (default).
# One can define her/his own set, see cmake/productsts/README

# Finding out the products to build is done in x steps:
# 1. have the user define the products wanted
# 2. estimate all additional required products
# 3. estimate which of the products can be build by external deps
# 4. estimate which of the products can be build by internal deps

include(CalligraProductSetMacros)

option(GHNS "support Get Hot New Stuff" OFF)
option(NEPOMUK "support NEPOMUK Tagging" ON)
# TODO: orthogonal setting, results in minimal features, yet needs to be defined
# option(TINY "compile a tiny Calligra" OFF)
option(QT3SUPPORT "Build the parts of Calligra that still depend on Qt3" ON)
option(PACKAGERS_BUILD "Build support of multiple CPU architectures in one binary. Should be used by packagers only." ON)

# TODO: remove option and migration code below before 3.0 release
option(CREATIVEONLY "compile only Karbon and Krita" OFF)

#
# WARNING: don't change the SHOULD_BUILD_app if an application
# doesn't build, but rather add the following line:
# option(BUILD_app "Build directory app" OFF)
#
#eg:
#option(BUILD_kexi "Build directory kexi" OFF) # kexi isn't buildable at the moment

# temporary migration support
set(PRODUCTSET_DEFAULT "ALL")
if (CREATIVEONLY)
    set(WARN_ABOUT_CREATIVEONLY TRUE)
    set(PRODUCTSET_DEFAULT "CREATIVE")
endif (CREATIVEONLY)

if(NOT PRODUCTSET)
    set(PRODUCTSET ${PRODUCTSET_DEFAULT} CACHE STRING "Set of products to build" FORCE)
endif(NOT PRODUCTSET)

# TODO: also add headers/sdk for all the libs ("_DEVEL"?)
# TODO: note external deps for products, so they are only checked if needed
# There can be required or optional external deps, required will also result
# in automatic disabling of product building
# TODO: some products have multiple optional requirements, but need at least one.
# See CONVERTER, EXTRA_*

# 3rd-party libs
calligra_define_product(LIB_KDGANTT "libkdgantt")
calligra_define_product(LIB_KDCHART "libkdchart")
calligra_define_product(LIB_WRITERPERFECT "libwriterperfect")
# Calligra-independent utility libs
calligra_define_product(LIB_VECTORIMAGE "libvectorimage")
calligra_define_product(LIB_KOPROPERTY "libkoproperty")
calligra_define_product(LIB_KOREPORT "libkoreport" NEEDS LIB_KOPROPERTY)
calligra_define_product(LIB_CALLIGRADB "libcalligradb")
# calligra libs
calligra_define_product(CALLIGRA_LIBS "Calligra core libs")
calligra_define_product(LIB_KOPAGEAPP "Lib for paged documents"  NEEDS CALLIGRA_LIBS)
calligra_define_product(LIB_ODFTRAVERSE "libodftraverse"  NEEDS CALLIGRA_LIBS)
calligra_define_product(LIB_MSO "libmso"  NEEDS CALLIGRA_LIBS)
calligra_define_product(LIB_MSOOXML "libmsooxml"  NEEDS CALLIGRA_LIBS)
calligra_define_product(SCRIPTING "") # TODO
# apps
calligra_define_product(WORDS_PART "Words engine" NEEDS CALLIGRA_LIBS)
calligra_define_product(WORDS_APP "Words executable (for Desktop)"  NEEDS WORDS_PART)
calligra_define_product(STAGE_PART "Stage engine"  NEEDS CALLIGRA_LIBS LIB_KOPAGEAPP)
calligra_define_product(STAGE_APP "Stage executable (for Desktop)"  NEEDS STAGE_PART)
calligra_define_product(SHEETS_PART "Sheets engine"  NEEDS CALLIGRA_LIBS)
calligra_define_product(SHEETS_APP "Sheets executable (for Desktop)"  NEEDS SHEETS_PART)
calligra_define_product(AUTHOR_APP "Full Author (for Desktop)"  NEEDS WORDS_PART)
calligra_define_product(KARBON_APP "Full Karbon (for Desktop)"  NEEDS CALLIGRA_LIBS)
calligra_define_product(KEXI_APP "Full Kexi (for Desktop)"  NEEDS CALLIGRA_LIBS LIB_KOREPORT LIB_CALLIGRADB)
calligra_define_product(FLOW_APP "Full Flow (for Desktop)"  NEEDS CALLIGRA_LIBS LIB_KOPAGEAPP)
calligra_define_product(PLAN_APP "Full Plan (for Desktop)"  NEEDS CALLIGRA_LIBS LIB_KOREPORT LIB_KDCHART LIB_KDGANTT)
calligra_define_product(BRAINDUMP_APP "Full Braindump (for Desktop)"  NEEDS CALLIGRA_LIBS)
calligra_define_product(DOC "Calligra Documentations")
# extras
calligra_define_product(CONVERTER "Format converter for commandline"  NEEDS CALLIGRA_LIBS)
calligra_define_product(EXTRA_PROPERTIES "Plugin for the KDE file properties dialog"  NEEDS CALLIGRA_LIBS)
calligra_define_product(EXTRA_THUMBNAIL "Plugins for KDE filesystem thumbnailing"  NEEDS CALLIGRA_LIBS)
calligra_define_product(EXTRA_QUICKPRINT "Plugin for the KDE filemanager adding a \"Print\" action")
calligra_define_product(OKULARODPGENERATOR "Plugin for Okular adding support for ODP and MS PPT/PPTX" NEEDS STAGE_PART)
# plugins
calligra_define_product(PLUGIN_COLORENGINES ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_MUSICSHAPE ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_SPACENAVIGATOR ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_ARTISTICTEXTSHAPE ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_DOCKERS ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_TEXTEDITING ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_KEXI_SPREADSHEETMIGRATION ""  NEEDS KEXI_APP SHEETS_PART)
calligra_define_product(PLUGIN_DEFAULTTOOLS ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_PATHSHAPES ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_VARIABLES ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_CHARTSHAPE "" NEEDS CALLIGRA_LIBS LIB_KDCHART)
calligra_define_product(PLUGIN_PICTURESHAPE ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_TEXTSHAPE ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_PLUGINSHAPE ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_FORMULASHAPE ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_VIDEOSHAPE "Plugin for handling videos in Calligra"  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_VECTORSHAPE "" NEEDS CALLIGRA_LIBS LIB_VECTORIMAGE)
calligra_define_product(PLUGIN_REPORTING ""  NEEDS LIB_KOREPORT LIB_KDCHART)
# staging plugins
calligra_define_product(PLUGIN_GOOGLEDOCS ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_THREEDSHAPE ""  NEEDS CALLIGRA_LIBS)
calligra_define_product(PLUGIN_SHEETS_PIVOTTABLES ""  NEEDS SHEETS_PART)
calligra_define_product(PLUGIN_STAGING "All plugins in staging status"  NEEDS PLUGIN_GOOGLEDOCS PLUGIN_THREEDSHAPE PLUGIN_SHEETS_PIVOTTABLES)
# Sheets filters
calligra_define_product(FILTER_XLSX_TO_ODS "XLSX to ODS filter" NEEDS LIB_MSOOXML SHEETS_PART)
calligra_define_product(FILTER_XLS_TO_SHEETS "Sheets XLS import filter" NEEDS LIB_MSO LIB_MSOOXML SHEETS_PART)
calligra_define_product(FILTER_SHEETS_TO_XLS "Sheets XLS export filter" NEEDS LIB_MSO LIB_MSOOXML SHEETS_PART)
calligra_define_product(FILTER_CSV_TO_SHEETS "Sheets CSV import filter" NEEDS SHEETS_PART)
calligra_define_product(FILTER_SHEETS_TO_CSV "Sheets CSV export filter" NEEDS SHEETS_PART)
calligra_define_product(FILTER_APPLIXSPREAD_TO_KSPREAD "Applix Spreadsheet to KSpread filter" NEEDS SHEETS_PART)
calligra_define_product(FILTER_DBASE_TO_KSPREAD "dBASE to KSpread filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_GNUMERIC_TO_SHEETS "Sheets GNUMERIC import filter" NEEDS SHEETS_PART)
calligra_define_product(FILTER_SHEETS_TO_GNUMERIC "Sheets GNUMERIC import filter" NEEDS SHEETS_PART)
calligra_define_product(FILTER_OPENCALC_TO_SHEETS "Sheets OpenOffice.org Calc import filter" NEEDS SHEETS_PART)
calligra_define_product(FILTER_SHEETS_TO_OPENCALC "Sheets OpenOffice.org Calc export filter" NEEDS SHEETS_PART)
calligra_define_product(FILTER_QUATTROPRO_TO_SHEETS "Sheets Quattro Pro import filter" NEEDS SHEETS_PART)
calligra_define_product(FILTER_HTML_TO_ODS "HTML to ODS filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_SHEETS_TO_HTML "Sheets HTML export filter"  NEEDS SHEETS_PART)
calligra_define_product(FILTER_KSPREAD_TO_LATEX "KSpread to LaTeX filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_SHEETS_IMPORT "All Sheets import filter"  WANTS FILTER_XLSX_TO_ODS FILTER_XLS_TO_SHEETS FILTER_CSV_TO_SHEETS FILTER_APPLIXSPREAD_TO_KSPREAD FILTER_DBASE_TO_KSPREAD FILTER_GNUMERIC_TO_SHEETS FILTER_OPENCALC_TO_SHEETS FILTER_QUATTROPRO_TO_SHEETS FILTER_HTML_TO_ODS)
calligra_define_product(FILTER_SHEETS_EXPORT "All Sheets export filter"  WANTS FILTER_SHEETS_TO_XLS FILTER_SHEETS_TO_CSV FILTER_SHEETS_TO_GNUMERIC FILTER_SHEETS_TO_OPENCALC FILTER_SHEETS_TO_HTML FILTER_KSPREAD_TO_LATEX)
calligra_define_product(FILTER_SHEETS "All Sheets filter"  WANTS FILTER_SHEETS_IMPORT FILTER_SHEETS_EXPORT)
# Flow filters
calligra_define_product(FILTER_VISIO_TO_ODG "Visio to ODG filter"  NEEDS LIB_WRITERPERFECT CALLIGRA_LIBS)
calligra_define_product(FILTER_FLOW_IMPORT "All Flow import filter"  WANTS FILTER_VISIO_TO_ODG)
calligra_define_product(FILTER_FLOW_EXPORT "All Flow export filter"  WANTS )
calligra_define_product(FILTER_FLOW "All Flow filter"  WANTS FILTER_FLOW_IMPORT FILTER_FLOW_EXPORT)
# Stage filters
calligra_define_product(FILTER_KPR_TO_OPD "KPresenter to ODP filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_PPT_TO_ODP "PPT to OPD filter" NEEDS LIB_MSO CALLIGRA_LIBS)
calligra_define_product(FILTER_PPTX_TO_ODP "PPTX to ODP filter" NEEDS LIB_MSOOXML CALLIGRA_LIBS)
calligra_define_product(FILTER_STAGE_IMPORT "All Stage import filter"  WANTS FILTER_KPR_TO_OPD FILTER_PPT_TO_ODP FILTER_PPTX_TO_ODP)
calligra_define_product(FILTER_STAGE_EXPORT "All Stage export filter"  WANTS )
calligra_define_product(FILTER_STAGE "All Stage filter"  WANTS FILTER_STAGE_IMPORT FILTER_STAGE_EXPORT)
# Words filters
calligra_define_product(FILTER_DOC_TO_ODT "DOC to ODT filter" NEEDS LIB_MSO LIB_MSOOXML CALLIGRA_LIBS)
calligra_define_product(FILTER_DOCX_TO_ODT "DOCX to ODT filter" NEEDS LIB_MSOOXML CALLIGRA_LIBS)
calligra_define_product(FILTER_WORDPERFECT_TO_ODT "Word Perfect to ODT filter"  NEEDS LIB_WRITERPERFECT CALLIGRA_LIBS)
calligra_define_product(FILTER_WORKS_TO_ODT "MS Works to ODT filter"  NEEDS LIB_WRITERPERFECT CALLIGRA_LIBS)
calligra_define_product(FILTER_APPLIXWORD_TO_ODT "Applixword to ODT filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_ASCII_TO_WORDS "Words ASCII import filter"  NEEDS WORDS_PART)
calligra_define_product(FILTER_ODT_TO_ASCII "ODT to ASCII filter"  NEEDS LIB_ODFTRAVERSE CALLIGRA_LIBS)
calligra_define_product(FILTER_RTF_TO_ODT "RTF to ODT filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_ODT_TO_MOBI "Mobi export filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_ODT_TO_EPUB2 "ODT Epub2 export filter"  NEEDS LIB_VECTORIMAGE CALLIGRA_LIBS)
calligra_define_product(FILTER_ODT_TO_HTML "ODT HTML export filter"  NEEDS LIB_VECTORIMAGE CALLIGRA_LIBS)
calligra_define_product(FILTER_WORDS_IMPORT "All Words import filter"  WANTS FILTER_DOC_TO_ODT FILTER_DOCX_TO_ODT FILTER_WORDPERFECT_TO_ODT FILTER_WORKS_TO_ODT FILTER_ASCII_TO_WORDS FILTER_RTF_TO_ODT)
calligra_define_product(FILTER_WORDS_EXPORT "All Words export filter"  WANTS FILTER_ODT_TO_ASCII FILTER_ODT_TO_MOBI FILTER_ODT_TO_EPUB2 FILTER_ODT_TO_HTML)
calligra_define_product(FILTER_WORDS "All Words filter"  WANTS FILTER_WORDS_IMPORT FILTER_WORDS_EXPORT)
# Plan filters
calligra_define_product(FILTER_MPXJ_IMPORT "MS Project import filter"  NEEDS PLAN_APP)
calligra_define_product(FILTER_PLAN "All Plan filter" WANTS FILTER_MPXJ_IMPORT)
# Karbon filters
calligra_define_product(FILTER_EPS_TO_SVG_AI "EPS to SVG/AI filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_XFIG_TO_ODG "XFig to ODG filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_PDF_TO_SVG "PDF to SVG filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_WPG_TO_SVG "WPG to SVG filter"  NEEDS CALLIGRA_LIBS)
calligra_define_product(FILTER_KARBON_TO_IMAGE "Karbon image export filter"  NEEDS KARBON_APP)
calligra_define_product(FILTER_KARBON_TO_SVG "Karbon SVG export filter"  NEEDS KARBON_APP)
calligra_define_product(FILTER_SVG_TO_KARBON "Karbon SVG import filter"  NEEDS KARBON_APP)
calligra_define_product(FILTER_KARBON_TO_WMF "Karbon WMF export filter"  NEEDS KARBON_APP)
calligra_define_product(FILTER_WMF_TO_SVG "WMF to SVG filter"  NEEDS LIB_VECTORIMAGE CALLIGRA_LIBS)
calligra_define_product(FILTER_KARBON1X_TO_KARBON "Karbon 1.x import filter"  NEEDS KARBON_APP)
calligra_define_product(FILTER_KARBON_IMPORT "All Karbon import filter"  WANTS FILTER_EPS_TO_SVG_AI FILTER_PDF_TO_SVG FILTER_WPG_TO_SVG FILTER_SVG_TO_KARBON FILTER_WMF_TO_SVG FILTER_KARBON1X_TO_KARBON)
calligra_define_product(FILTER_KARBON_EXPORT "All Karbon export filter"  WANTS FILTER_KARBON_TO_IMAGE FILTER_KARBON_TO_SVG FILTER_KARBON_TO_WMF)
calligra_define_product(FILTER_KARBON "All Karbon filter"  WANTS FILTER_KARBON_IMPORT FILTER_KARBON_EXPORT)
# meta apps
calligra_define_product(WORDS "Full Words (for Desktop)"  NEEDS WORDS_APP  WANTS FILTER_WORDS)
calligra_define_product(STAGE "Full Stage (for Desktop)"  NEEDS STAGE_APP  WANTS FILTER_STAGE)
calligra_define_product(SHEETS "Full Sheets (for Desktop)"  NEEDS SHEETS_APP  WANTS FILTER_SHEETS)
calligra_define_product(AUTHOR "Full Author (for Desktop)"  NEEDS AUTHOR_APP  WANTS FILTER_WORDS)
calligra_define_product(KARBON "Full Karbon (for Desktop)"  NEEDS KARBON_APP  WANTS FILTER_KARBON)
calligra_define_product(KRITA "Full Krita (for Desktop)"  NEEDS CALLIGRA_LIBS)
calligra_define_product(KEXI "Full Kexi (for Desktop)"  NEEDS KEXI_APP  WANTS PLUGIN_KEXI_SPREADSHEETMIGRATION)
calligra_define_product(FLOW "Full Flow (for Desktop)"  NEEDS FLOW_APP  WANTS FILTER_FLOW)
calligra_define_product(PLAN "Full Plan (for Desktop)"  NEEDS PLAN_APP  WANTS FILTER_PLAN)
calligra_define_product(BRAINDUMP "Full Braindump (for Desktop)"  NEEDS BRAINDUMP_APP)
calligra_define_product(ACTIVE "Calligra Active"  NEEDS SHEETS_PART STAGE_PART WORDS_PART)
calligra_define_product(CALLIGRA ""  NEEDS CALLIGRA_LIBS)
# developer utils
calligra_define_product(DEVTOOLS "Tools for developers")
calligra_define_product(CSTESTER "" NEEDS SHEETS_PART STAGE_PART WORDS_PART)

calligra_set_productset(${PRODUCTSET})

 ########################
#########################
## Look for KDE and Qt ##
#########################
########################
set(KDE_MIN_VERSION "4.3.0")
find_package(KDE4 4.3.0 REQUIRED)

if (QT3SUPPORT)
    find_package(Qt4 4.6.0 REQUIRED QtCore QtGui QtXml QtScript QtSvg QtTest QtWebKit QtDBus QtOpenGL Qt3Support)
else (QT3SUPPORT)
    find_package(Qt4 4.6.0 REQUIRED QtCore QtGui QtXml QtScript QtSvg QtTest QtWebKit QtDBus QtOpenGL)
endif (QT3SUPPORT)

include (KDE4Defaults)
include (MacroLibrary)
include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)

if ((${QTVERSION} VERSION_EQUAL 4.8.0) OR (${QTVERSION} VERSION_EQUAL 4.8.1))
    if (NOT IHAVEPATCHEDQT)
        set(SHOULD_BUILD_WORDS_PART FALSE)
        set(SHOULD_BUILD_STAGE_PART FALSE)
        set(SHOULD_BUILD_PLUGIN_TEXTSHAPE FALSE)
        set(SHOULD_BUILD_PLUGIN_TEXTEDITING FALSE)
        MESSAGE(STATUS "WARNING: You are using a version of Qt that causes crashes. As a result Words and Stage will not be built. Please upgrade to 4.8.2. You can also patch Qt and when building Calligra set IHAVEPATCHEDQT. Patch against Qt can be found in qt48setx.patch")
    endif (NOT IHAVEPATCHEDQT)
endif ((${QTVERSION} VERSION_EQUAL 4.8.0) OR (${QTVERSION} VERSION_EQUAL 4.8.1))

if (GHNS)
    if(NOT ${KDE_VERSION} VERSION_GREATER 4.4.0)
        set(GHNS FALSE)
    endif(NOT ${KDE_VERSION} VERSION_GREATER 4.4.0)

    macro_optional_find_package(LibAttica)
    macro_log_feature(LIBATTICA_FOUND "LibAttica" "Attica is used for Get Hot New Stuff." "https://projects.kde.org/projects/kdesupport/attica" FALSE "" "You need at least version 3.0 for uploading of resources to work.")

    if (NOT LIBATTICA_FOUND)
        set(GHNS FALSE)
    else (NOT LIBATTICA_FOUND)
        MESSAGE(STATUS "WARNING: You are compiling with Get Hot New Stuff enabled. Do not do that when building distribution packages. GHNS is unusable these days until someone starts maintaining it again.")
    endif (NOT LIBATTICA_FOUND)
endif (GHNS)

if(NOT ${KDE_VERSION} VERSION_GREATER 4.6.4)
  set(NEPOMUK FALSE)
endif(NOT ${KDE_VERSION} VERSION_GREATER 4.6.4)

macro_ensure_out_of_source_build("Compiling Calligra inside the source directory is not possible. Please refer to the build instruction http://community.kde.org/Calligra/Building/Building_Calligra")

if(KDE4_BUILD_TESTS)
    # only with this definition will the FOO_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

 ###########################
############################
## Required dependencies  ##
############################
###########################

find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
if (APPLE)
    # this is not added correctly on OSX -- see http://forum.kde.org/viewtopic.php?f=139&t=101867&p=221242#p221242
    include_directories(${PNG_INCLUDE_DIR})
endif(APPLE)
find_package(Boost REQUIRED) # for pigment and stage
if (NOT Boost_FOUND)
    MESSAGE(FATAL_ERROR "Did not find Boost. Boost is required for the core libraries, stage, sheets and krita.")
endif (NOT Boost_FOUND)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

 ###########################
############################
## Optional dependencies  ##
############################
###########################

##
## Test for sqlite
##
set(SQLITE_MIN_VERSION 3.6.16)
set(SQLITE_RECOMMENDED_VERSION 3.7.15)
set(SQLITE_LOAD_EXTENSION_REQUIRED ON)
macro_optional_find_package(CalligraSqlite 3.6.16)
macro_bool_to_01(SQLITE_FOUND HAVE_SQLITE)
macro_log_feature(SQLITE_FOUND "SQLite" "Embedded SQL database engine with enabled extensions loading"
                  "http://www.sqlite.org" FALSE "${SQLITE_MIN_VERSION}"
                  "Required by Calligra's default database handler for Kexi and bibliography in Words.")

##
## Test for ICU
##
macro_optional_find_package(ICU)
macro_bool_to_01(ICU_FOUND HAVE_ICU)
macro_log_feature(ICU_FOUND "ICU" "International Components for Unicode (ICU) Library"
                  "http://icu-project.org" FALSE ""
                  "Required by Calligra's default database handler for Kexi and Words.")

##
## Check for OpenEXR 
##

macro_optional_find_package(OpenEXR)
macro_bool_to_01(OPENEXR_FOUND HAVE_OPENEXR)

##
## Look for OpenGL
##
set(HAVE_OPENGL 0)
if (!WIN32 AND !APPLE)

    macro_optional_find_package(OpenGL)

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
endif (!WIN32 AND !APPLE)

macro_log_feature(HAVE_OPENGL "OpenGL" "OpenGL support" "" FALSE "" "Required by parts of Krita and optionally by flake")

##
## Test for kdepimlibs
##
macro_optional_find_package(KdepimLibs)
macro_log_feature(KDEPIMLIBS_FOUND "KDE PIMLibs" "KDE Personal Information Management Libraries" "http://www.kde.org/" FALSE "" "Required by Plan and the KDE address book integration (available as a module in KDE)")


##
## Test for eigen2
##
macro_optional_find_package(Eigen2)
macro_log_feature(EIGEN2_FOUND "Eigen" "C++ template library for linear algebra" "http://eigen.tuxfamily.org" FALSE "2.0" "Required by Calligra Sheets and Krita")

##
## Test for QCA2
##
macro_optional_find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0" "Required for encrypted OpenDocument files and encrypted xls files support (available as a module in kdesupport)")

##
## Test for exiv2
##
set(EXIV2_MIN_VERSION "0.16")
macro_optional_find_package(Exiv2)
macro_log_feature(EXIV2_FOUND "Exiv2" "Image metadata library and tools" "http://www.exiv2.org" FALSE "0.16" "Required by Krita")

##
## Test for soprano
##
macro_optional_find_package(Soprano)
macro_log_feature(Soprano_FOUND "Soprano" "KDE4 RDF handling library" "http://soprano.sourceforge.net/" FALSE "" "Required to handle RDF metadata in ODF")
if(NOT Soprano_FOUND)
    set(SHOULD_BUILD_RDF FALSE)
    set(SOPRANO_INCLUDE_DIR "")
else(NOT Soprano_FOUND)
    set(SHOULD_BUILD_RDF TRUE)
   add_definitions( -DSHOULD_BUILD_RDF )
endif(NOT Soprano_FOUND)


##
## Test for marble
##
macro_optional_find_package(Marble)
macro_log_feature(MARBLE_FOUND "Marble" "KDE4 World Globe Widget library" "http://techbase.kde.org/Projects/Marble/" FALSE "" "Required by RDF, Kexi Forms and Reports to show locations on a map")
if(NOT MARBLE_FOUND)
    set(CAN_USE_MARBLE FALSE)
    set(MARBLE_INCLUDE_DIR "")
else(NOT MARBLE_FOUND)
    set(CAN_USE_MARBLE TRUE)
endif(NOT MARBLE_FOUND)
if( MARBLE_FOUND )
    add_definitions( -DCAN_USE_MARBLE )
    ##
    ## Marble changed addMarbleWidget to setMarbleWidget in MarbleControlBox.h
    ## with commit ea177ca. This is for compatibility with older versions.
    ##
    find_file(MARBLECONTROLBOX_H MarbleControlBox.h ${MARBLE_INCLUDE_DIR} PATH_SUFFIXES marble)
    if( MARBLECONTROLBOX_H )
    file(READ ${MARBLECONTROLBOX_H} MARBLECONTROLBOX_H_CONTENT)
    string(REGEX MATCH "setMarbleWidget" SETMARBLEWIDGET ${MARBLECONTROLBOX_H_CONTENT})
        if( SETMARBLEWIDGET )
            add_definitions(-DHAVE_SETMARBLEWIDGET)
        endif( SETMARBLEWIDGET)
    else( MARBLECONTROLBOX_H )
        message( WARNING "MarbleControlBox.h not found, could not properly set the SETMARBLEWIDGET define." )
    endif( MARBLECONTROLBOX_H )
endif( MARBLE_FOUND )

##
## Test for lcms
##
macro_optional_find_package(LCMS2)
macro_log_feature(LCMS2_FOUND "LittleCMS" "Color management engine" "http://www.littlecms.com" FALSE "2.4" "Will be used for color management and is necesary for Krita")
if(LCMS2_FOUND)
    if(NOT ${LCMS2_VERSION} VERSION_LESS 2040 )
        set(HAVE_LCMS24 TRUE)
    endif(NOT ${LCMS2_VERSION} VERSION_LESS 2040 )
    set(HAVE_REQUIRED_LCMS_VERSION TRUE)
    set(HAVE_LCMS2 TRUE)
endif(LCMS2_FOUND)

##
## Test for Vc
##

set(OLD_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} )
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )
macro_optional_find_package(Vc 0.6.70)
macro_log_feature(Vc_FOUND "Vc" "Portable, zero-overhead SIMD library for C++" "http://code.compeng.uni-frankfurt.de/projects/vc" FALSE "" "Required by the Krita for vectorization")
macro_bool_to_01(Vc_FOUND HAVE_VC)
macro_bool_to_01(PACKAGERS_BUILD DO_PACKAGERS_BUILD)

configure_file(config-vc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-vc.h )

if(HAVE_VC)
    message(STATUS "Vc found!")

    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Vc_CMAKE_MODULES_DIR}")
    include (VcMacros)

    macro(ko_compile_for_all_implementations_no_scalar _objs _src)
      if(PACKAGERS_BUILD)
        vc_compile_for_all_implementations(${_objs} ${_src} FLAGS -fPIC ONLY SSE2 SSSE3 SSE4_1 AVX)
      else(PACKAGERS_BUILD)
        set(${_objs} ${_src})
      endif(PACKAGERS_BUILD)
    endmacro()

    macro(ko_compile_for_all_implementations _objs _src)
      if(PACKAGERS_BUILD)
        vc_compile_for_all_implementations(${_objs} ${_src} FLAGS -fPIC ONLY Scalar SSE2 SSSE3 SSE4_1 AVX)
      else(PACKAGERS_BUILD)
        set(${_objs} ${_src})
      endif(PACKAGERS_BUILD)
    endmacro()

    if (NOT PACKAGERS_BUILD)
      # Optimize the whole Calligra for current architecture
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Vc_DEFINITIONS}")
    endif (NOT PACKAGERS_BUILD)
endif(HAVE_VC)
set(CMAKE_MODULE_PATH ${OLD_CMAKE_MODULE_PATH} )

##
## Test for Nepomuk
##
if(NEPOMUK)
    macro_optional_find_package(Nepomuk)
    if(NOT NEPOMUK_FOUND)
        set(NEPOMUK FALSE)
    endif(NOT NEPOMUK_FOUND)
    add_definitions(-DDISABLE_NEPOMUK_LEGACY)
    macro_log_feature(NEPOMUK_FOUND "Nepomuk" "Handles metadata of resources" FALSE  "Required to handle metadata of resources")
endif(NEPOMUK)

##
## Test for KActivities
##
macro_optional_find_package(KActivities 6.1.0)
macro_log_feature(KActivities_FOUND "KActivities" "Activities interface library" "https://projects.kde.org/projects/kde/kdelibs/kactivities" FALSE "" "Required for Activities integration.")
if(KActivities_FOUND)
    set(HAVE_KACTIVITIES TRUE)
endif(KActivities_FOUND)

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

##
## Test for Fontconfig
##
## Only test if on non-Windows system
if(!WIN32)
    macro_optional_find_package(Fontconfig)
    macro_log_feature(FONTCONFIG_FOUND "Fontconfig" "Library for configuring and customizing font access" "http://fontconfig.org" FALSE "" "Required to handle exact font size")
endif(!WIN32)

##
## Test for Freetype
##
## Only test if on non-Windows system
if(!WIN32)
    macro_optional_find_package(Freetype)
    macro_log_feature(FREETYPE_FOUND "Freetype" "A Free, High-Quality, and Portable Font Engine" "http://www.freetype.org/" FALSE "" "Required to handle exact font size")
endif(!WIN32)

if(NOT FONTCONFIG_FOUND OR NOT FREETYPE_FOUND)
    set(FONTCONFIG_INCLUDE_DIR "")
    set(FREETYPE_INCLUDE_DIRS "")
else(NOT FONTCONFIG_FOUND OR NOT FREETYPE_FOUND)
    add_definitions( -DSHOULD_BUILD_FONT_CONVERSION )
endif(NOT FONTCONFIG_FOUND OR NOT FREETYPE_FOUND)

##
## Test for Qt Webkit
##
macro_log_feature(QT_QTWEBKIT_FOUND "Qt Webkit" "Qt binding for Webkit, the HTML engine." "http://qt.nokia.com" FALSE "" "Required for the web shape, web Kexi widget and web report element")
if(QT_QTWEBKIT_FOUND)
    add_definitions( -DCAN_USE_QTWEBKIT )
endif(QT_QTWEBKIT_FOUND)

##
## QImageBlitz (for Kexi)
##
#macro_optional_find_package(QImageBlitz)
#macro_log_feature(QIMAGEBLITZ_FOUND "QImageBlitz" "Fast, smooth scaling of QImages" "sourceforge.net/projects/qimageblitz/" FALSE "" "Required for Kexi")

##
## Test for KDeclarative
##
macro_optional_find_package(KDeclarative)

##
## Test endianess
##
include (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

##
## Test SharedMimeInfo
##
macro_optional_find_package(SharedMimeInfo)
macro_log_feature(SHARED_MIME_INFO_FOUND "SharedMimeInfo" "Shared Mime Info" "http://freedesktop.org/wiki/Software/shared-mime-info" FALSE "" "Required to determine file types OpenRaster (Krita default format), SVM or all of MSOOXML.")

##
## Test for Okular
##
macro_optional_find_package(Okular)
macro_log_feature(OKULAR_FOUND "Okular" "Okular ODP Plugin" "http://okular.kde.org/" FALSE "" "Required to build the Okular OpenDocument Presenter plugin")

##
## Test for WordPerfect Document Lib
##
macro_optional_find_package(WPD)
macro_log_feature(WPD_FOUND "WPD" "WordPerfect Document Library" "http://libwpd.sourceforge.net/" FALSE "" "Required by the Words WPD import filter")

##
## Test for WordPerfect Graphics Lib
##
macro_optional_find_package(WPG)
macro_log_feature(LIBWPG_FOUND "WPG" "WordPerfect Graphics Library" "http://libwpg.sourceforge.net/" FALSE "" "Required by the Karbon WPG import filter")

##
## Test for Microsoft Works Document Library
##
macro_optional_find_package(LibWPS)
macro_log_feature(LIBWPS_FOUND "LibWPS" "Microsoft Works Document Library" "http://libwps.sourceforge.net/" FALSE "" "Required by the Words WPS import filter")

##
## Test for libvisio
##
macro_optional_find_package(LibVisio)
macro_log_feature(LIBVISIO_FOUND "LibVisio" "Visio Import Filter Library" "http://www.freedesktop.org/wiki/Software/libvisio" FALSE "" "Required by the Flow visio import filter")

##
## Test for qt-poppler
##
macro_optional_find_package(Poppler)
macro_log_feature( POPPLER_FOUND "poppler-qt4" "The Poppler Qt4 interface library" "http://poppler.freedesktop.org" FALSE "" "Required by the Krita PDF filter, Karbon PDF import filter and CSTester PDF feature")

## The Karbon pdf-importer needs the not-officially-supported XPDF Headers
## Installing these is off by default in poppler sources, so lets make
## sure they're really there before trying to build the pdf import
if(POPPLER_FOUND)
    find_path(POPPLER_XPDF_HEADERS poppler-config.h
    HINTS ${POPPLER_INCLUDE_DIR} )
    if(POPPLER_XPDF_HEADERS)
        set(POPPLER_XPDF_HEADERS_FOUND TRUE)
    endif(POPPLER_XPDF_HEADERS)
    macro_log_feature( POPPLER_XPDF_HEADERS_FOUND "poppler-qt4-xpdf-headers" "XPDF headers in the Poppler Qt4 interface library" "http://poppler.freedesktop.org" FALSE "" "Required by the Karbon PDF import filter")
endif(POPPLER_FOUND)


##
## Generate a file for prefix information
##

 ###############################
################################
## Add Calligra helper macros ##
################################
###############################

include(MacroCalligraAddBenchmark)
include(MacroCalligraBuildTest)

 ####################
#####################
## Define includes ##
#####################
####################

# WARNING: make sure that QT_INCLUDES is the first directory to be added to include_directory before
#          any other include directory

# for config.h and <toplevel/foo.h> includes (if any?)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/interfaces )

# koplugin is at the bottom of the stack
set(KOPLUGIN_INCLUDES ${CMAKE_SOURCE_DIR}/libs/koplugin)

# koodf is at the bottom of the stack
set(KOODF_INCLUDES ${CMAKE_SOURCE_DIR}/libs/odf
                   ${CMAKE_BINARY_DIR}/libs/odf
                   ${KDE4_INCLUDES})

set(KUNDO2_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kundo2)

# pigment depends on koplugin and lcms
set(PIGMENT_INCLUDES ${KOPLUGIN_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/pigment
                   ${CMAKE_SOURCE_DIR}/libs/pigment/compositeops
                   ${CMAKE_SOURCE_DIR}/libs/pigment/resources
                   ${QT_INCLUDES}
                   ${KDE4_INCLUDES})

# flake depends on koodf and pigment
set(FLAKE_INCLUDES ${CMAKE_SOURCE_DIR}/libs/flake
                   ${KOODF_INCLUDES}
                   ${KOPLUGIN_INCLUDES}
                   ${PIGMENT_INCLUDES}
                   ${KUNDO2_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/widgetutils
                   ${CMAKE_SOURCE_DIR}/libs/flake/commands
                   ${CMAKE_SOURCE_DIR}/libs/flake/tools
                   ${CMAKE_SOURCE_DIR}/libs/flake/svg
                   ${CMAKE_BINARY_DIR}/libs/flake)

# vectorimage
set(VECTORIMAGE_INCLUDES 
                   ${CMAKE_SOURCE_DIR}/libs/vectorimage
                   ${CMAKE_SOURCE_DIR}/libs/vectorimage/libemf
                   ${CMAKE_SOURCE_DIR}/libs/vectorimage/libsvm
                   ${CMAKE_SOURCE_DIR}/libs/vectorimage/libwmf)
                   
# KoText depends on koplugin, odf
set(KOTEXT_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kotext
                   ${CMAKE_BINARY_DIR}/libs/kotext
                   ${CMAKE_SOURCE_DIR}/libs/kotext/changetracker
                   ${CMAKE_SOURCE_DIR}/libs/kotext/styles
                   ${CMAKE_SOURCE_DIR}/libs/kotext/opendocument
                   ${SOPRANO_INCLUDE_DIR}
                   ${FLAKE_INCLUDES}
                   ${KOODF_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/main)

# TextLayout depends on kotext
set(TEXTLAYOUT_INCLUDES ${KOTEXT_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/textlayout)

# BasicFlakes depends on flake, widgets
set(BASICFLAKES_INCLUDES ${KOWIDGETS_INCLUDES} ${FLAKE_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/basicflakes
                   ${CMAKE_SOURCE_DIR}/libs/basicflakes/tools)

# komain depends on kotext & flake
set(KOMAIN_INCLUDES ${KDE4_INCLUDES}
                    ${TEXTLAYOUT_INCLUDES}
                    ${CMAKE_SOURCE_DIR}/libs/widgets
                    ${CMAKE_SOURCE_DIR}/libs/widgetutils
                    ${CMAKE_SOURCE_DIR}/libs/main
                    ${CMAKE_SOURCE_DIR}/libs/main/config)

set(KORDF_INCLUDES ${KOMAIN_INCLUDES}
                   ${CMAKE_SOURCE_DIR}/libs/rdf
)

if(SHOULD_BUILD_SCRIPTING)
  set(KOKROSS_INCLUDES ${CMAKE_SOURCE_DIR}/libs/kokross ${CMAKE_BINARY_DIR}/libs/kokross)
endif(SHOULD_BUILD_SCRIPTING)

# kopageapp
set(KOPAGEAPP_INCLUDES ${TEXTLAYOUT_INCLUDES}
                    ${PIGMENT_INCLUDES}
                    ${KOMAIN_INCLUDES}
                    ${CMAKE_SOURCE_DIR}/libs/widgets
                    ${CMAKE_SOURCE_DIR}/libs/kopageapp ${CMAKE_SOURCE_DIR}/libs/kopageapp/commands ${CMAKE_BINARY_DIR}/libs/kopageapp )

# koproperty
set(KOPROPERTY_INCLUDES ${KOODF_INCLUDES}
                        ${CMAKE_SOURCE_DIR}/libs
                        ${CMAKE_SOURCE_DIR}/libs/widgets)
set(KOPROPERTY_LIBS koproperty)

# koreport
set(KOREPORT_INCLUDES
    ${CMAKE_SOURCE_DIR}/libs/
    ${CMAKE_SOURCE_DIR}/libs/koreport
    ${CMAKE_SOURCE_DIR}/libs/koreport/common
    ${CMAKE_SOURCE_DIR}/libs/koreport/renderer
    ${CMAKE_SOURCE_DIR}/libs/koreport/wrtembed)
set(KOREPORT_LIBS koreport)

# calligradb
# TODO: remove when Predicate lib arrives
if (HAVE_ICU AND HAVE_SQLITE) # These are hard deps for calligradb
   set(CALLIGRADB_LIBS calligradb)                    # TODO remove when Predicate lib arrives
   set(CALLIGRADB_INCLUDES ${CMAKE_SOURCE_DIR}/libs   # TODO remove when Predicate lib arrives
                           ${CMAKE_BINARY_DIR}/libs/db)
else (HAVE_ICU AND HAVE_SQLITE)
   set(SHOULD_BUILD_LIB_CALLIGRADB FALSE)
endif (HAVE_ICU AND HAVE_SQLITE)

#############################################
####          3rd party libraries        ####
#############################################

set(KDGANTT_INCLUDES
    ${CMAKE_SOURCE_DIR}/3rdparty/kdgantt
)

set(KDCHART_INCLUDES
    ${CMAKE_SOURCE_DIR}/3rdparty/kdchart/include
)

set(WRITERPERFECT_INCLUDES
    ${CMAKE_SOURCE_DIR}/filters/writerperfect
)

 #########################################
##########################################
## Detect which products can be compile ##
##########################################
#########################################

#
# WARNING: when adding a dependency if there is already a line
# containing SHOULD_BUILD_app, don't add a new one, just add
# the test to the if construct
#

# libooxml needs to install own mimetypes
if(NOT SHARED_MIME_INFO_FOUND)
    set(SHOULD_BUILD_LIB_MSOOXML FALSE)
endif(NOT SHARED_MIME_INFO_FOUND)

### Calligra Sheets: eigen2, boost
if(NOT EIGEN2_FOUND)
    set(SHOULD_BUILD_SHEETS_PART FALSE)
endif(NOT EIGEN2_FOUND)

### Krita: pigment, eigen2, exiv2, sharedmimeinfo
if(NOT EIGEN2_FOUND OR NOT EXIV2_FOUND OR NOT HAVE_REQUIRED_LCMS_VERSION OR NOT SHARED_MIME_INFO_FOUND)
   set(SHOULD_BUILD_KRITA FALSE)
endif(NOT EIGEN2_FOUND OR NOT EXIV2_FOUND OR NOT HAVE_REQUIRED_LCMS_VERSION OR NOT SHARED_MIME_INFO_FOUND)


### Active: only build with Qt 4.7.0 or better
if (NOT ${QTVERSION} VERSION_GREATER 4.7.0)
    set(SHOULD_BUILD_ACTIVE FALSE)
endif (NOT ${QTVERSION} VERSION_GREATER 4.7.0)

if (NOT QT_QTDECLARATIVE_FOUND OR NOT KDECLARATIVE_FOUND)
    set(SHOULD_BUILD_ACTIVE FALSE)
endif(NOT QT_QTDECLARATIVE_FOUND OR NOT KDECLARATIVE_FOUND)

#if(NOT QIMAGEBLITZ_FOUND)
#    set (SHOULD_BUILD_KEXI FALSE)
#endif(NOT QIMAGEBLITZ_FOUND)

if (NOT QT3SUPPORT)
    set (SHOULD_BUILD_KEXI FALSE)
endif (NOT QT3SUPPORT)

if(NOT OKULAR_FOUND)
    set(SHOULD_BUILD_OKULARODPGENERATOR FALSE)
endif(NOT OKULAR_FOUND)

if (NOT WPD_FOUND OR NOT LIBWPG_FOUND OR NOT LIBVISIO_FOUND)
    set(SHOULD_BUILD_FILTER_VISIO_TO_ODG FALSE)
endif (NOT WPD_FOUND OR NOT LIBWPG_FOUND OR NOT LIBVISIO_FOUND)

if(NOT WPD_FOUND OR NOT LIBWPG_FOUND)
    set(SHOULD_BUILD_FILTER_WORDPERFECT_TO_ODT FALSE)
endif(NOT WPD_FOUND OR NOT LIBWPG_FOUND)

if(NOT WPD_FOUND OR NOT LIBWPS_FOUND)
    set(SHOULD_BUILD_FILTER_WORKS_TO_ODT FALSE)
endif(NOT WPD_FOUND OR NOT LIBWPS_FOUND)

if (NOT WPD_FOUND OR NOT LIBWPG_FOUND)
    set(SHOULD_BUILD_LIB_WRITERPERFECT FALSE)
endif (NOT WPD_FOUND OR NOT LIBWPG_FOUND)

if (NOT WPD_FOUND OR NOT LIBWPG_FOUND)
    set(SHOULD_BUILD_FILTER_WPG_TO_SVG FALSE)
endif (NOT WPD_FOUND OR NOT LIBWPG_FOUND)

if (WIN32 OR NOT POPPLER_XPDF_HEADERS_FOUND)
    set(SHOULD_BUILD_FILTER_PDF_TO_SVG FALSE)
endif (WIN32 OR NOT POPPLER_XPDF_HEADERS_FOUND)

if(WIN32 OR QT_MAC_USE_COCOA)
    set(SHOULD_BUILD_FILTER_HTML_TO_ODS FALSE)
endif(WIN32 OR QT_MAC_USE_COCOA)

if(WIN32 OR QT_MAC_USE_COCOA)
    set(SHOULD_BUILD_FILTER_SHEETS_TO_HTML FALSE)
endif(WIN32 OR QT_MAC_USE_COCOA)

if(WIN32 OR QT_MAC_USE_COCOA)
    set(SHOULD_BUILD_FILTER_KSPREAD_TO_LATEX FALSE)
endif(WIN32 OR QT_MAC_USE_COCOA)

# finally disable all those which are missing internal dependencies
calligra_drop_unbuildable_products()

 ###################
####################
## Subdirectories ##
####################
###################

add_subdirectory(words)

if (SHOULD_BUILD_FLOW_APP)
    add_subdirectory(flow)
endif (SHOULD_BUILD_FLOW_APP)

add_subdirectory(stage)

if(SHOULD_BUILD_KEXI_APP)
    add_subdirectory(kexi)
endif(SHOULD_BUILD_KEXI_APP)

if(SHOULD_BUILD_PLAN_APP)
    add_subdirectory(plan)
endif(SHOULD_BUILD_PLAN_APP)

add_subdirectory(sheets)

if(SHOULD_BUILD_KRITA)
  add_subdirectory(krita)
endif(SHOULD_BUILD_KRITA)

if(SHOULD_BUILD_KARBON_APP)
  add_subdirectory(karbon)
endif(SHOULD_BUILD_KARBON_APP)

if(SHOULD_BUILD_BRAINDUMP_APP)
  add_subdirectory(braindump)
endif(SHOULD_BUILD_BRAINDUMP_APP)

if(SHOULD_BUILD_ACTIVE)
  add_subdirectory(active)
endif(SHOULD_BUILD_ACTIVE)

if(SHOULD_BUILD_DOC)
    add_subdirectory(doc)
endif(SHOULD_BUILD_DOC)

# non-app directories are moved here because they can depend on SHOULD_BUILD_{appname} variables set above
add_subdirectory(libs)
add_subdirectory(3rdparty)
add_subdirectory(interfaces)
add_subdirectory(cmake)
add_subdirectory(pics)
add_subdirectory(plugins)
add_subdirectory(servicetypes)
add_subdirectory(templates)
add_subdirectory(devtools)
add_subdirectory(extras)
add_subdirectory(filters)
add_subdirectory(data)

macro_display_feature_log()

calligra_log_should_build()

add_custom_target(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

configure_file(KoConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/KoConfig.h )
install( FILES  ${CMAKE_CURRENT_BINARY_DIR}/KoConfig.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

## temporary warning
if (WARN_ABOUT_CREATIVEONLY)
    message(STATUS "WARNING:\n   You are using a deprecated build flag,\n   switch from \"-DCREATIVEONLY=ON\" to \"-DPRODUCTSET=CREATIVE\"\n   and remove the line \"CREATIVEONLY:BOOL=ON\" from CMakeCache.txt")
    message(STATUS "--------------------------------------------------------------------------" )
endif (WARN_ABOUT_CREATIVEONLY)
