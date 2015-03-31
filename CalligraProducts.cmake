### DEFINITION OF PRODUCTS, FEATURES AND PRODUCTSETS
####################################################

# When building Calligra a lot of different things are created and installed. To
# describe them and their internal dependencies the concepts of "product",
# "feature" and "product set" are used.

# A "product" is the smallest functional unit which can be created in the build
# and which is useful on its own when installed. Examples are e.g. libraries,
# plugins or executables. Products have external and internal required
# dependencies at build-time. Internal dependencies are noted in terms of other
# products or features (see below) and could be e.g. other libraries to link
# against or build tools needed to generate source files.
# A product gets defined by setting an identifier, a descriptive fullname and
# the needed internal build-time requirements. Any other product or feature
# listed as requirement must have been defined before.

# A "feature" is not a standalone product, but adds abilities to one or multiple
# given products. One examples is e.g. scriptability. Features have external and
# internal required dependencies at build-time. Internal dependencies are noted
# in terms of other products or features and could be e.g. other libraries to
# link against or build tools needed to generate source files.
# A feature gets defined by setting an identifier, a descriptive fullname and
# the needed internal build-time requirements. Any other product or feature
# listed as requirement must have been defined before.

# A "productset" is a selection of products and features which should be build
# together. The products and features can be either essential or optional to the
# set. If essential (REQUIRES), the whole productset will not be build if a
# product or feature is missing another internal or external dependency. If
# optional (OPTIONAL), the rest of the set will still be build in that case.
# The products and features to include in a set can be listed directly or
# indirectly: they can be named explicitely, but also by including other
# productsets in a set, whose products and features will then be part of the
# first set as well.
# Products, features and productsets can be listed as dependencies in multiple
# product sets. As with dependencies for products or features, they must have
# been defined before.

# Products, features and product sets are in the same namespace, so a given
# identifier can be only used either for a product or for a feature or for a
# product set.

# The ids of products and features (but not sets) are used to generate cmake
# variables SHOULD_BUILD_${ID}, which then are used to control what is build and
# how.


#############################################
####      Product definitions            ####
#############################################

# For defining new products see end of this file, "How to add another product?"

# IDEA: also add headers/sdk for all the libs ("_DEVEL"?)
# IDEA: note external deps for products, so they are only checked if needed
# There can be required or optional external deps, required will also result
# in automatic disabling of product building
# TODO: some products have multiple optional requirements, but need at least one.
# See APP_CONVERTER, FILEMANAGER_*

# building tools
calligra_define_product(BUILDTOOL_RNG2CPP "rng2cpp")

# Calligra-independent utility libs
calligra_define_product(LIB_KOVECTORIMAGE "libkovectorimage")

# calligra libs
calligra_define_product(LIB_CALLIGRA "Calligra core libs" REQUIRES BUILDTOOL_RNG2CPP)
calligra_define_product(LIB_KOPAGEAPP "Lib for paged documents"  REQUIRES LIB_CALLIGRA)
calligra_define_product(LIB_KOODF2 "libkoodf2"  REQUIRES LIB_CALLIGRA)
calligra_define_product(LIB_KOODFREADER "libkoodfreader"  REQUIRES LIB_KOODF2 LIB_CALLIGRA)
calligra_define_product(LIB_MSO "libmso"  REQUIRES LIB_CALLIGRA)
calligra_define_product(LIB_KOMSOOXML "libkomsooxml" REQUIRES LIB_KOODF2 LIB_CALLIGRA)

# features
calligra_define_feature(FEATURE_SCRIPTING "Scripting feature" UNPORTED) # TODO
calligra_define_feature(FEATURE_RDF "RDF feature" UNPORTED)

# plugins
calligra_define_product(PLUGIN_TEXTSHAPE "Text shape plugin"  REQUIRES LIB_CALLIGRA)

# parts
calligra_define_product(PART_WORDS "Words engine"  REQUIRES LIB_CALLIGRA PLUGIN_TEXTSHAPE)
calligra_define_product(PART_STAGE "Stage engine"  REQUIRES LIB_CALLIGRA LIB_KOPAGEAPP)
calligra_define_product(PART_SHEETS "Sheets engine" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(PART_QTQUICK "QtQuick Plugin that provides Calligra components" UNPORTED  REQUIRES PART_WORDS PART_STAGE)# SHEETS_PART)

# apps
calligra_define_product(APP_WORDS "Words app (for Desktop)"  REQUIRES PART_WORDS)
calligra_define_product(APP_STAGE "Stage app (for Desktop)"  REQUIRES PART_STAGE)
calligra_define_product(APP_SHEETS "Sheets app (for Desktop)" UNPORTED  REQUIRES PART_SHEETS)
calligra_define_product(APP_AUTHOR "Author app (for Desktop)" UNPORTED  REQUIRES PART_WORDS)
calligra_define_product(APP_KARBON "Karbon app (for Desktop)" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(APP_KRITA "Krita app (for Desktop)" UNPORTED REQUIRES LIB_CALLIGRA)
calligra_define_product(APP_KEXI "Kexi app (for Desktop)" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(APP_FLOW "Flow app (for Desktop)" UNPORTED  REQUIRES LIB_CALLIGRA LIB_KOPAGEAPP)
calligra_define_product(APP_PLAN "Plan app (for Desktop)" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(APP_BRAINDUMP "Braindump app (for Desktop)" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(APP_GEMINI "The Calligra Gemini application" UNPORTED  REQUIRES PART_QTQUICK)
# TODO: this needs to be split up by app products
calligra_define_product(DOC "Calligra Documentations" UNPORTED)

# extras
calligra_define_product(APP_CONVERTER "Format converter for commandline" REQUIRES LIB_CALLIGRA)
calligra_define_product(FILEMANAGER_PROPERTIES "Plugin for the KDE file properties dialog" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILEMANAGER_THUMBNAIL "Plugins for KDE filesystem thumbnailing"  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILEMANAGER_QUICKPRINT "Plugin for the filemanager adding a \"Print\" action" UNPORTED)
calligra_define_product(FILEMANAGER_TEMPLATES "File templates for filemanager" UNPORTED)
calligra_define_product(OKULAR_GENERATOR_ODP "Plugin for Okular adding support for ODP" UNPORTED  REQUIRES PART_STAGE)
calligra_define_product(OKULAR_GENERATOR_ODT "Plugin for Okular adding support for ODT" UNPORTED  REQUIRES PART_WORDS)

# more plugins
calligra_define_product(PLUGIN_COLORENGINES "Colorengine plugins"  REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_MUSICSHAPE "Music shape plugin"  REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_SPACENAVIGATOR "SpaceNavigator input plugin" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_ARTISTICTEXTSHAPE "Artistic shape plugin" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_DOCKERS "Default dockers plugin" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_TEXTEDITING "Textediting plugins"  REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_KEXI_SPREADSHEETMIGRATION "Import from ODS plugin for Kexi" UNPORTED  REQUIRES APP_KEXI PART_SHEETS)
calligra_define_product(PLUGIN_DEFAULTTOOLS "Default Flake tools plugin" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_PATHSHAPES "Path shape plugins" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_VARIABLES "Text variables plugin"  REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_CHARTSHAPE "Chart shape plugin"  REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_PICTURESHAPE "Picture shape plugin" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_PLUGINSHAPE "Plugin shape plugin" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_FORMULASHAPE "Formula shape plugin" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_VIDEOSHAPE "Plugin for handling videos in Calligra" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_VECTORSHAPE "Vectorgraphic shape plugin"  REQUIRES LIB_CALLIGRA LIB_KOVECTORIMAGE)
calligra_define_product(PLUGIN_SEMANTICITEMS "Semantic items plugins" UNPORTED  REQUIRES FEATURE_RDF LIB_CALLIGRA)
calligra_define_product(PLUGIN_CALLIGRAGEMINI_GIT "Git support plugin for Calligra Gemini" UNPORTED)

# staging plugins
calligra_define_product(PLUGIN_GOOGLEDOCS "Plugin for integration with Google Docs" STAGING UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_THREEDSHAPE "3D shape plugin"  STAGING  REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_SHEETS_PIVOTTABLES "Plugin for Sheets adding pivot tables"  STAGING UNPORTED  REQUIRES PART_SHEETS)

# Sheets filters
calligra_define_product(FILTER_XLSX_TO_ODS "XLSX to ODS filter" UNPORTED  REQUIRES LIB_KOMSOOXML PART_SHEETS)
calligra_define_product(FILTER_XLS_TO_SHEETS "Sheets XLS import filter" UNPORTED  REQUIRES LIB_MSO LIB_KOMSOOXML PART_SHEETS)
calligra_define_product(FILTER_SHEETS_TO_XLS "Sheets XLS export filter" UNPORTED  REQUIRES LIB_MSO LIB_KOMSOOXML PART_SHEETS)
calligra_define_product(FILTER_CSV_TO_SHEETS "Sheets CSV import filter" UNPORTED  REQUIRES PART_SHEETS)
calligra_define_product(FILTER_SHEETS_TO_CSV "Sheets CSV export filter" UNPORTED  REQUIRES PART_SHEETS)
calligra_define_product(FILTER_APPLIXSPREAD_TO_KSPREAD "Applix Spreadsheet to KSpread filter" UNPORTED  REQUIRES PART_SHEETS)
calligra_define_product(FILTER_DBASE_TO_KSPREAD "dBASE to KSpread filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_GNUMERIC_TO_SHEETS "Sheets GNUMERIC import filter" UNPORTED  REQUIRES PART_SHEETS)
calligra_define_product(FILTER_SHEETS_TO_GNUMERIC "Sheets GNUMERIC import filter" UNPORTED  REQUIRES PART_SHEETS)
calligra_define_product(FILTER_OPENCALC_TO_SHEETS "Sheets OpenOffice.org Calc import filter" UNPORTED  REQUIRES PART_SHEETS)
calligra_define_product(FILTER_SHEETS_TO_OPENCALC "Sheets OpenOffice.org Calc export filter" UNPORTED  REQUIRES PART_SHEETS)
calligra_define_product(FILTER_QUATTROPRO_TO_SHEETS "Sheets Quattro Pro import filter" UNPORTED  REQUIRES PART_SHEETS)
calligra_define_product(FILTER_HTML_TO_ODS "HTML to ODS filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_SHEETS_TO_HTML "Sheets HTML export filter" UNPORTED  REQUIRES PART_SHEETS)
calligra_define_product(FILTER_KSPREAD_TO_LATEX "KSpread to LaTeX filter" UNPORTED  REQUIRES LIB_CALLIGRA)

# Flow filters
calligra_define_product(FILTER_VISIO_TO_ODG "Visio to ODG filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_WPG_TO_ODG "WPG to ODG filter" UNPORTED  REQUIRES LIB_CALLIGRA)

# Stage filters
calligra_define_product(FILTER_KEY_TO_ODP "Apple Keynote to ODP filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_KPR_TO_ODP "KPresenter to ODP filter"  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_PPT_TO_ODP "PPT to OPD filter"  REQUIRES LIB_MSO LIB_CALLIGRA)
calligra_define_product(FILTER_PPTX_TO_ODP "PPTX to ODP filter" UNPORTED  REQUIRES LIB_KOMSOOXML LIB_KOODF2 LIB_CALLIGRA)

# Words filters
calligra_define_product(FILTER_DOC_TO_ODT "DOC to ODT filter" UNPORTED  REQUIRES LIB_MSO LIB_KOMSOOXML LIB_CALLIGRA)
calligra_define_product(FILTER_DOCX_TO_ODT "DOCX to ODT filter" UNPORTED  REQUIRES LIB_KOMSOOXML LIB_KOODF2 LIB_CALLIGRA)
calligra_define_product(FILTER_ODT_TO_DOCX "ODT to DOCX filter" UNPORTED  REQUIRES LIB_KOODFREADER LIB_KOODF2 LIB_CALLIGRA)
calligra_define_product(FILTER_WORDPERFECT_TO_ODT "Word Perfect to ODT filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_WORKS_TO_ODT "MS Works to ODT filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_APPLIXWORD_TO_ODT "Applixword to ODT filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_ASCII_TO_WORDS "Words ASCII import filter" UNPORTED  REQUIRES PART_WORDS LIB_KOODF2)
calligra_define_product(FILTER_ODT_TO_ASCII "ODT to ASCII filter" REQUIRES LIB_KOODFREADER LIB_CALLIGRA)
calligra_define_product(FILTER_RTF_TO_ODT "RTF to ODT filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_ODT_TO_MOBI "Mobi export filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_ODT_TO_EPUB2 "ODT Epub2 export filter" UNPORTED  REQUIRES LIB_KOVECTORIMAGE LIB_CALLIGRA)
calligra_define_product(FILTER_ODT_TO_HTML "ODT HTML export filter" UNPORTED  REQUIRES LIB_KOVECTORIMAGE LIB_CALLIGRA)
calligra_define_product(FILTER_ODT_TO_WIKI "ODT Wiki export filter" UNPORTED  REQUIRES LIB_KOODFREADER LIB_KOODF2 LIB_CALLIGRA)

# Plan filters
calligra_define_product(FILTER_MPXJ_IMPORT "MS Project import filter" UNPORTED  REQUIRES APP_PLAN)

# Karbon filters
calligra_define_product(FILTER_EPS_TO_SVG_AI "EPS to SVG/AI filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_XFIG_TO_ODG "XFig to ODG filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_PDF_TO_SVG "PDF to SVG filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_WPG_TO_SVG "WPG to SVG filter" UNPORTED  REQUIRES LIB_CALLIGRA)
calligra_define_product(FILTER_KARBON_TO_IMAGE "Karbon image export filter" UNPORTED  REQUIRES APP_KARBON)
calligra_define_product(FILTER_KARBON_TO_SVG "Karbon SVG export filter" UNPORTED  REQUIRES APP_KARBON)
calligra_define_product(FILTER_SVG_TO_KARBON "Karbon SVG import filter" UNPORTED  REQUIRES APP_KARBON)
calligra_define_product(FILTER_KARBON_TO_WMF "Karbon WMF export filter" UNPORTED  REQUIRES APP_KARBON)
calligra_define_product(FILTER_WMF_TO_SVG "WMF to SVG filter" UNPORTED  REQUIRES LIB_KOVECTORIMAGE LIB_CALLIGRA)
calligra_define_product(FILTER_KARBON1X_TO_KARBON "Karbon 1.x import filter" UNPORTED  REQUIRES APP_KARBON)

# meta apps
calligra_define_product(APP_ACTIVE "Calligra Active app" UNPORTED  REQUIRES PART_SHEETS PART_STAGE PART_WORDS)
calligra_define_product(APP_CALLIGRA "General Calligra app starter"  REQUIRES LIB_CALLIGRA)

# more extras
calligra_define_product(OKULAR_GENERATOR_PPT "Plugin for Okular extended with support for PPT" UNPORTED  REQUIRES OKULAR_GENERATOR_ODP FILTER_PPT_TO_ODP)
calligra_define_product(OKULAR_GENERATOR_PPTX "Plugin for Okular extended with support for PPTX" UNPORTED  REQUIRES OKULAR_GENERATOR_ODP FILTER_PPTX_TO_ODP)
calligra_define_product(OKULAR_GENERATOR_DOC "Plugin for Okular extended with support for DOC" UNPORTED REQUIRES OKULAR_GENERATOR_ODT FILTER_DOC_TO_ODT)
calligra_define_product(OKULAR_GENERATOR_DOCX "Plugin for Okular extended with support for DOCX" UNPORTED REQUIRES OKULAR_GENERATOR_ODT FILTER_DOCX_TO_ODT)
calligra_define_product(OKULAR_GENERATOR_WORDPERFECT "Plugin for Okular extended with support for WORDPERFECT" UNPORTED REQUIRES OKULAR_GENERATOR_ODT FILTER_WORDPERFECT_TO_ODT)

# developer utils
calligra_define_product(APP_DEVTOOLS "Tools for developers")
calligra_define_product(APP_CSTESTER "cstester" UNPORTED  REQUIRES PART_SHEETS PART_STAGE PART_WORDS)


#############################################
####      Product set definitions        ####
#############################################

# For defining new productsets see end of this file,
# "How to add another productset?"

# filter sets
calligra_define_productset(FILTERS_SHEETS_IMPORT "All Sheets import filters"
    OPTIONAL
        FILTER_XLSX_TO_ODS
        FILTER_XLS_TO_SHEETS
        FILTER_CSV_TO_SHEETS
        FILTER_APPLIXSPREAD_TO_KSPREAD
        FILTER_DBASE_TO_KSPREAD
        FILTER_GNUMERIC_TO_SHEETS
        FILTER_OPENCALC_TO_SHEETS
        FILTER_QUATTROPRO_TO_SHEETS
        FILTER_HTML_TO_ODS
)
calligra_define_productset(FILTERS_SHEETS_EXPORT "All Sheets export filters"
    OPTIONAL
        FILTER_SHEETS_TO_XLS
        FILTER_SHEETS_TO_CSV
        FILTER_SHEETS_TO_GNUMERIC
        FILTER_SHEETS_TO_OPENCALC
        FILTER_SHEETS_TO_HTML
        FILTER_KSPREAD_TO_LATEX
)
calligra_define_productset(FILTERS_SHEETS "All Sheets filters"
    OPTIONAL
        FILTERS_SHEETS_IMPORT
        FILTERS_SHEETS_EXPORT
)
calligra_define_productset(FILTERS_FLOW_IMPORT "All Flow import filters"
    OPTIONAL
        FILTER_VISIO_TO_ODG
        FILTER_WPG_TO_ODG
)
#calligra_define_productset(FILTERS_FLOW_EXPORT "All Flow export filters"  OPTIONAL ) noone currently
calligra_define_productset(FILTERS_FLOW "All Flow filters"
    OPTIONAL
        FILTERS_FLOW_IMPORT
#        FILTERS_FLOW_EXPORT
)
calligra_define_productset(FILTERS_STAGE_IMPORT "All Stage import filters"
    OPTIONAL
        FILTER_KEY_TO_ODP
        FILTER_KPR_TO_ODP
        FILTER_PPT_TO_ODP
        FILTER_PPTX_TO_ODP
)
#calligra_define_productset(FILTERS_STAGE_EXPORT "All Stage export filters"  OPTIONAL ) noone currently
calligra_define_productset(FILTERS_STAGE "All Stage filters"
    OPTIONAL
        FILTERS_STAGE_IMPORT
#         FILTERS_STAGE_EXPORT
)
calligra_define_productset(FILTERS_WORDS_IMPORT "All Words import filters"
    OPTIONAL
        FILTER_DOC_TO_ODT
        FILTER_DOCX_TO_ODT
        FILTER_WORDPERFECT_TO_ODT
        FILTER_WORKS_TO_ODT
        FILTER_APPLIXWORD_TO_ODT
        FILTER_ASCII_TO_WORDS
        FILTER_RTF_TO_ODT
)
calligra_define_productset(FILTERS_WORDS_EXPORT "All Words export filters"
    OPTIONAL
        FILTER_ODT_TO_ASCII
        FILTER_ODT_TO_MOBI
        FILTER_ODT_TO_EPUB2
        FILTER_ODT_TO_HTML
        FILTER_ODT_TO_DOCX
        FILTER_ODT_TO_WIKI
)
calligra_define_productset(FILTERS_WORDS "All Words filters"
    OPTIONAL
        FILTERS_WORDS_IMPORT
        FILTERS_WORDS_EXPORT
)
calligra_define_productset(FILTERS_PLAN "All Plan filters"
    OPTIONAL
        FILTER_MPXJ_IMPORT
)
calligra_define_productset(FILTERS_KARBON_IMPORT "All Karbon import filters"
    OPTIONAL
        FILTER_EPS_TO_SVG_AI
        FILTER_XFIG_TO_ODG
        FILTER_PDF_TO_SVG
        FILTER_WPG_TO_SVG
        FILTER_SVG_TO_KARBON
        FILTER_WMF_TO_SVG
        FILTER_KARBON1X_TO_KARBON
)
calligra_define_productset(FILTERS_KARBON_EXPORT "All Karbon export filters"
    OPTIONAL
        FILTER_KARBON_TO_IMAGE
        FILTER_KARBON_TO_SVG
        FILTER_KARBON_TO_WMF
)
calligra_define_productset(FILTERS_KARBON "All Karbon filters"
    OPTIONAL
        FILTERS_KARBON_IMPORT
        FILTERS_KARBON_EXPORT
)

# filemanager
calligra_define_productset(FILEMANAGER "Extensions for the filemanager"
    OPTIONAL
        FILEMANAGER_PROPERTIES
        FILEMANAGER_QUICKPRINT
        FILEMANAGER_TEMPLATES
        FILEMANAGER_THUMBNAIL
)

# apps
calligra_define_productset(ACTIVE "Full Calligra Active"
    REQUIRES
        APP_ACTIVE
    OPTIONAL
        # extras
        FILEMANAGER_PROPERTIES
        FILEMANAGER_THUMBNAIL
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_CHARTSHAPE
        PLUGIN_DEFAULTTOOLS
        PLUGIN_FORMULASHAPE
        PLUGIN_PATHSHAPES
        PLUGIN_PICTURESHAPE
        PLUGIN_PLUGINSHAPE
        PLUGIN_TEXTEDITING
        PLUGIN_TEXTSHAPE
        PLUGIN_VARIABLES
        PLUGIN_VECTORSHAPE
        PLUGIN_VIDEOSHAPE
        # filters
        FILTERS_SHEETS_IMPORT
        FILTERS_STAGE_IMPORT
        FILTERS_WORDS_IMPORT
)
calligra_define_productset(AUTHOR "Full Author (for Desktop)"
    REQUIRES
        APP_AUTHOR
    OPTIONAL
        # extras
        FILEMANAGER
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_CHARTSHAPE
        PLUGIN_DEFAULTTOOLS
        PLUGIN_DOCKERS
        PLUGIN_FORMULASHAPE
        PLUGIN_PATHSHAPES
        PLUGIN_PICTURESHAPE
        PLUGIN_PLUGINSHAPE
        PLUGIN_TEXTEDITING
        PLUGIN_TEXTSHAPE
        PLUGIN_SEMANTICITEMS
        PLUGIN_VARIABLES
        PLUGIN_VECTORSHAPE
        # filters
        FILTERS_WORDS
)
calligra_define_productset(BRAINDUMP "Full Braindump (for Desktop)"
    REQUIRES
        APP_BRAINDUMP
    OPTIONAL
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_CHARTSHAPE
        PLUGIN_DEFAULTTOOLS
        PLUGIN_DOCKERS
        PLUGIN_FORMULASHAPE
        PLUGIN_MUSICSHAPE
        PLUGIN_PATHSHAPES
        PLUGIN_PICTURESHAPE
        PLUGIN_PLUGINSHAPE
        PLUGIN_TEXTEDITING
        PLUGIN_TEXTSHAPE
        PLUGIN_THREEDSHAPE
        PLUGIN_VARIABLES
        PLUGIN_VECTORSHAPE
        PLUGIN_VIDEOSHAPE
)
calligra_define_productset(FLOW "Full Flow (for Desktop)"
    REQUIRES
        APP_FLOW
    OPTIONAL
        # extras
        FILEMANAGER
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_CHARTSHAPE
        PLUGIN_DEFAULTTOOLS
        PLUGIN_DOCKERS
        PLUGIN_FORMULASHAPE
        PLUGIN_PATHSHAPES
        PLUGIN_PICTURESHAPE
        PLUGIN_PLUGINSHAPE
        PLUGIN_TEXTEDITING
        PLUGIN_TEXTSHAPE
        PLUGIN_VARIABLES
        PLUGIN_VECTORSHAPE
        # filters
        FILTERS_FLOW
)
calligra_define_productset(KARBON "Full Karbon (for Desktop)"
    REQUIRES
        APP_KARBON
    OPTIONAL
        # extras
        FILEMANAGER
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_CHARTSHAPE
        PLUGIN_DEFAULTTOOLS
        PLUGIN_DOCKERS
        PLUGIN_FORMULASHAPE
        PLUGIN_PATHSHAPES
        PLUGIN_PICTURESHAPE
        PLUGIN_PLUGINSHAPE
        PLUGIN_TEXTEDITING
        PLUGIN_TEXTSHAPE
        PLUGIN_VARIABLES
        PLUGIN_VECTORSHAPE
        # filters
        FILTERS_KARBON
)
calligra_define_productset(KEXI "Full Kexi (for Desktop)"
    REQUIRES
        APP_KEXI
    OPTIONAL
        FEATURE_SCRIPTING
        PLUGIN_KEXI_SPREADSHEETMIGRATION
)
calligra_define_productset(KRITA "Full Krita"
    REQUIRES
        APP_KRITA
    OPTIONAL
        # extras
        FILEMANAGER_PROPERTIES
        FILEMANAGER_THUMBNAIL
        FILEMANAGER_QUICKPRINT
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_COLORENGINES
        PLUGIN_DEFAULTTOOLS
        PLUGIN_DOCKERS
        PLUGIN_PATHSHAPES
        PLUGIN_PICTURESHAPE
        PLUGIN_TEXTEDITING
        PLUGIN_TEXTSHAPE
        PLUGIN_SPACENAVIGATOR
        PLUGIN_VECTORSHAPE
)
calligra_define_productset(PLAN "Full Plan (for Desktop)"
    REQUIRES
        APP_PLAN
    OPTIONAL
        FEATURE_SCRIPTING
        FILTERS_PLAN
)
calligra_define_productset(SHEETS "Full Sheets (for Desktop)"
    REQUIRES
        APP_SHEETS
    OPTIONAL
        # extras
        FILEMANAGER
        # feature
        FEATURE_SCRIPTING
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_CHARTSHAPE
        PLUGIN_DEFAULTTOOLS
        PLUGIN_DOCKERS
        PLUGIN_FORMULASHAPE
        PLUGIN_PATHSHAPES
        PLUGIN_PICTURESHAPE
        PLUGIN_PLUGINSHAPE
        PLUGIN_SHEETS_PIVOTTABLES
        PLUGIN_TEXTEDITING
        PLUGIN_TEXTSHAPE
        PLUGIN_VARIABLES
        PLUGIN_VECTORSHAPE
        # filters
        FILTERS_SHEETS
)
calligra_define_productset(STAGE "Full Stage (for Desktop)"
    REQUIRES
        APP_STAGE
    OPTIONAL
        # extras
        FILEMANAGER
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_CHARTSHAPE
        PLUGIN_DEFAULTTOOLS
        PLUGIN_DOCKERS
        PLUGIN_FORMULASHAPE
        PLUGIN_PATHSHAPES
        PLUGIN_PICTURESHAPE
        PLUGIN_PLUGINSHAPE
        PLUGIN_TEXTEDITING
        PLUGIN_TEXTSHAPE
        PLUGIN_VARIABLES
        PLUGIN_VECTORSHAPE
        PLUGIN_VIDEOSHAPE
        # filters
        FILTERS_STAGE
)
calligra_define_productset(WORDS "Full Words (for Desktop)"
    REQUIRES
        APP_WORDS
    OPTIONAL
        # extras
        FILEMANAGER
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_CHARTSHAPE
        PLUGIN_DEFAULTTOOLS
        PLUGIN_DOCKERS
        PLUGIN_FORMULASHAPE
        PLUGIN_PATHSHAPES
        PLUGIN_PICTURESHAPE
        PLUGIN_PLUGINSHAPE
        PLUGIN_SEMANTICITEMS
        PLUGIN_TEXTEDITING
        PLUGIN_TEXTSHAPE
        PLUGIN_VARIABLES
        PLUGIN_VECTORSHAPE
        # filters
        FILTERS_WORDS
)
calligra_define_productset(GEMINI "Calligra for 2:1 devices"
    REQUIRES
        APP_GEMINI
    OPTIONAL
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_CALLIGRAGEMINI_GIT
        PLUGIN_CHARTSHAPE
        PLUGIN_DEFAULTTOOLS
        PLUGIN_DOCKERS
        PLUGIN_FORMULASHAPE
        PLUGIN_PATHSHAPES
        PLUGIN_PICTURESHAPE
        PLUGIN_PLUGINSHAPE
        PLUGIN_TEXTEDITING
        PLUGIN_TEXTSHAPE
        PLUGIN_VARIABLES
        PLUGIN_VECTORSHAPE
        PLUGIN_VIDEOSHAPE
        # filters
        FILTERS_WORDS
        FILTERS_STAGE
)

# okular support
calligra_define_productset(OKULAR "Okular generators"
    OPTIONAL
        OKULAR_GENERATOR_ODP
        OKULAR_GENERATOR_PPT
        OKULAR_GENERATOR_PPTX
        OKULAR_GENERATOR_ODT
        OKULAR_GENERATOR_DOC
        OKULAR_GENERATOR_DOCX
        OKULAR_GENERATOR_WORDPERFECT
)


# How to add another product?
# ===========================
#
# 1. Define the product by a call of calligra_define_product,
#    e.g.
#
#    calligra_define_product(MYPRODUCT "title of product")
#
#    For the product id use a proper prefix (LIB_, PLUGIN_, FILTER_, APP_, PART_,
#     ...), whatever is appropriate.
#
# 2. Extend that call with a REQUIRES argument section, if the product has
#    hard internal build-time dependencies on other products or features.
#    Products/features that are listed as dependencies have to be defined before
#    (see also the API doc in cmake/modules/CalligraProductSetMacros.cmake)
#    E.g.
#
#    calligra_define_product(MYPRODUCT "title of product"  REQUIRES P1 P2)
#
# 3. Add a rule when to not build the product, in the section "Detect which
#    products/features can be compiled" of the toplevel CMakeLists.txt. Each
#    product should have their own boolean expression when to set the build flag
#    to FALSE, e.g.
#
#    if (PLATFORMX OR NOT EXTERNAL_DEP_X_FOUND)
#      set(SHOULD_BUILD_MYPRODUCT FALSE)
#    endif ()
#
# 4. Wrap everything belonging to the product with the build flag of the product.
#    Ideally this is done around subdirectory inclusions, results in easier code.
#    e.g.
#
#    if (SHOULD_BUILD_MYPRODUCT)
#      add_subdirectory(myproduct)
#    endif ()
#
# 5. Tag the product as STAGING, if it is not yet ready for release, but already
#    integrated in the master branch, e.g.
#
#    calligra_define_product(MYPRODUCT "title of product" STAGING REQUIRES P1)
#
# 6. Add the product to all products, features and product sets which have this
#    product as REQUIRED or OPTIONAL dependency.
#
#
# How to add another feature?
# ===========================
#
# 1. Define the feature by a call of calligra_define_feature,
#    e.g.
#
#    calligra_define_feature(MYFEATURE "title of feature")
#
#    For the feature id use a proper prefix (FEATURE_, ...), whatever is
#    appropriate.
#
# 2. Extend that call with a REQUIRES argument section, if the feature has
#    hard internal build-time dependencies on other products or features.
#    Products or features that are listed as dependencies have to be defined
#    before
#    (see also the API doc in cmake/modules/CalligraProductSetMacros.cmake)
#    E.g.
#
#    calligra_define_feature(MYFEATURE "title of feature"  REQUIRES P1 F1)
#
# 3. Add a rule when to not build the feature, in the section "Detect which
#    products/features can be compiled" of the toplevel CMakeLists.txt. Each
#    feature should have their own boolean expression when to set the build flag
#    to FALSE, e.g.
#
#    if (PLATFORMX OR NOT EXTERNAL_DEP_X_FOUND)
#      set(SHOULD_BUILD_MYFEATURE FALSE)
#    endif ()
#
# 4. Wrap everything belonging to the feature with the build flag of the feature.
#    Ideally this is done around subdirectory inclusions, results in easier code.
#    e.g.
#
#    if (SHOULD_BUILD_MYFEATURE)
#      add_subdirectory(myproduct)
#    endif ()
#
# 5. Tag the feature as STAGING, if it is not yet ready for release, but already
#    integrated in the master branch, e.g.
#
#    calligra_define_product(MYFEATURE "title of feature" STAGING REQUIRES P1 F1)
#
# 6. Add the feature to all products, features and product sets which have this
#    product as REQUIRED or OPTIONAL dependency.
#
#
# How to add another productset?
# ==============================
#
# There are two possible places to put a productset definition. The first is to
# add it to this file, which should be done for more generic sets that are
# useful for many people. The second is a file of its own, in the directory
# "cmake/productsets", which should be done for more special ones or for those
# which should not be added to the repository.
# The file must be named with the name of the productset in lowercase and have
# the extension ".cmake".
#
# 1. Define the productset by a call of calligra_define_productset,
#    e.g.
#
#    calligra_define_productset(MYPRODUCTSET "title of productset")
#
# 2. Extend that call with REQUIRES or OPTIONAL argument sections, if the productset
#    has hard or soft internal dependencies on other products, features or
#    productsets.
#    Products, features or productsets that are listed as dependencies have to
#    be defined before
#    (see also the API doc in cmake/modules/CalligraProductSetMacros.cmake)
#    E.g.
#
#    calligra_define_productset(MYPRODUCT "title of product"
#                               REQUIRES P1 P2 F1 PS1
#                               OPTIONAL P3 F2 PS2)
#
# 3. Add the productset to all product sets which have this product set as
#     REQUIRED or OPTIONAL dependency.
#
# Example for a file-based productset definition:
# You want a productset "MYWORDS". For that you add a file named
# "mywords.cmake" into the directory "cmake/productsets", with the content:
# --- 8< ---
# calligra_define_productset(MYWORDS "My Words"
#     REQUIRES
#         APP_WORDS
#         PLUGIN_DEFAULTTOOLS
#         PLUGIN_DOCKERS
#         PLUGIN_PATHSHAPES
#         PLUGIN_VARIABLES
#         PLUGIN_TEXTSHAPE
#         PLUGIN_PLUGINSHAPE
#         PLUGIN_FORMULASHAPE
# )
# --- 8< ---
