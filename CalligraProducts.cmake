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

# Calligra-independent utility libs
calligra_define_product(LIB_KOVECTORIMAGE "libkovectorimage")

# calligra libs
calligra_define_product(LIB_CALLIGRA "Calligra core libs"  REQUIRES )

# features

# plugins
calligra_define_product(PLUGIN_TEXTSHAPE "Text shape plugin"  REQUIRES LIB_CALLIGRA)

# parts

# apps
calligra_define_product(APP_KRITA "Krita app (for Desktop)" REQUIRES LIB_CALLIGRA)

# extras

# more plugins
calligra_define_product(PLUGIN_COLORENGINES "Colorengine plugins"  REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_ARTISTICTEXTSHAPE "Artistic shape plugin" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_DOCKERS "Default dockers plugin" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_DEFAULTTOOLS "Default Flake tools plugin" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_PATHSHAPES "Path shape plugins" REQUIRES LIB_CALLIGRA)
calligra_define_product(PLUGIN_VECTORSHAPE "Vectorgraphic shape plugin"  REQUIRES LIB_CALLIGRA LIB_KOVECTORIMAGE)

# staging plugins

#############################################
####      Product set definitions        ####
#############################################

# For defining new productsets see end of this file,
# "How to add another productset?"

calligra_define_productset(KRITA "Full Krita"
    REQUIRES
        APP_KRITA
    OPTIONAL
        # plugins
        PLUGIN_ARTISTICTEXTSHAPE
        PLUGIN_COLORENGINES
        PLUGIN_DEFAULTTOOLS
        PLUGIN_DOCKERS
        PLUGIN_PATHSHAPES
        PLUGIN_TEXTSHAPE
        PLUGIN_VECTORSHAPE
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
