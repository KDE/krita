# Copyright (c) 2013-2014 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Global variables
# CALLIGRA_SHOULD_BUILD_PRODUCTS - list of requested products by the user
# CALLIGRA_NEEDED_PRODUCTS - list of internal needed products
# CALLIGRA_WANTED_PRODUCTS - list of internal wanted products
# CALLIGRA_STAGING_PRODUCTS - list of products only in staging mode
# SHOULD_BUILD_${product_id} - boolean if product should be build


macro(calligra_disable_product _product_id _reason)
  if (NOT DEFINED SHOULD_BUILD_${_product_id})
    message(FATAL_ERROR "Unknown product: ${_product_id}")
  endif ()

  set(SHOULD_BUILD_${_product_id} FALSE)
  if (DEFINED BUILD_${_product_id}_DISABLE_REASON)
    set(BUILD_${_product_id}_DISABLE_REASON "${BUILD_${_product_id}_DISABLE_REASON} / ")
  endif ()
  set(BUILD_${_product_id}_DISABLE_REASON "${BUILD_${_product_id}_DISABLE_REASON}${_reason}")
endmacro()

# Usage:
#   calligra_drop_product_on_bad_condition(<product_id>
#         NAME_OF_BOOL_VAR1 REASON_TEXT_FOR_DROPPING_ON_FALSE1
#         NAME_OF_BOOL_VAR2 REASON_TEXT_FOR_DROPPING_ON_FALSE2
#         ...
#       )
macro(calligra_drop_product_on_bad_condition _product_id)
  if (NOT DEFINED SHOULD_BUILD_${_product_id})
    message(FATAL_ERROR "Unknown product: ${_product_id}")
  endif ()

  set(_current_flag)
  foreach(_arg ${ARGN})
    if(DEFINED _current_flag)
      if(NOT ${_current_flag})
        calligra_disable_product(${_product_id} ${_arg})
      endif()
      set(_current_flag)
    else()
      set(_current_flag ${_arg})
    endif()
  endforeach(_arg)
    if(DEFINED _current_flag)
    message(FATAL_ERROR "Bad number of arguments for calligra_drop_product_on_bad_condition(${_product_id} ...)")
  endif()
endmacro()

macro(calligra_set_shouldbuild_dependentproduct _product_id _dep_product_id)
  # if not already enabled, enable
  if (NOT SHOULD_BUILD_${_dep_product_id})
    list(APPEND CALLIGRA_NEEDED_PRODUCTS ${_dep_product_id})
    set(SHOULD_BUILD_${_dep_product_id} TRUE)
    if (DEFINED CALLIGRA_PRODUCT_${_dep_product_id}_needed_dependencies OR
        DEFINED CALLIGRA_PRODUCT_${_dep_product_id}_wanted_dependencies)
        calligra_set_shouldbuild_productdependencies(${_dep_product_id}
            "${CALLIGRA_PRODUCT_${_dep_product_id}_needed_dependencies}"
            "${CALLIGRA_PRODUCT_${_dep_product_id}_wanted_dependencies}")
    endif (DEFINED CALLIGRA_PRODUCT_${_dep_product_id}_needed_dependencies OR
           DEFINED CALLIGRA_PRODUCT_${_dep_product_id}_wanted_dependencies)
  endif ()
endmacro()


macro(calligra_set_shouldbuild_productdependencies _product_id _productset_id_needed_dependencies  _productset_id_wanted_dependencies)
  # activate all needed products and note the dependency
  foreach(_dep_product_id ${_productset_id_needed_dependencies})
    list(APPEND CALLIGRA_PRODUCT_${_dep_product_id}_dependents ${_product_id})
    calligra_set_shouldbuild_dependentproduct(${_product_id} ${_dep_product_id})
  endforeach(_dep_product_id)

  # activate all wanted products
  foreach(_dep_product_id ${_productset_id_wanted_dependencies})
    calligra_set_shouldbuild_dependentproduct(${_product_id} ${_dep_product_id})
  endforeach(_dep_product_id)
endmacro()


macro(calligra_drop_unbuildable_products)
  # first drop all staging products if not enabled
  if(NOT CALLIGRA_SHOULD_BUILD_STAGING)
    foreach(_product_id ${CALLIGRA_STAGING_PRODUCTS})
      calligra_disable_product(${_product_id} "Not ready for release")
    endforeach(_product_id)
  endif()

  # can assume calligra_all_products has products in down-up order
  # 1. check all wanted products and see if they will be built,
  #    if not then drop their required products
  # TODO!
  # 2. check all products if they can be built, if not disable build of depending
  foreach(_product_id ${CALLIGRA_ALL_PRODUCTS})
    if(NOT SHOULD_BUILD_${_product_id})
      if(DEFINED CALLIGRA_PRODUCT_${_product_id}_dependents)
        foreach(_dependent_product_id ${CALLIGRA_PRODUCT_${_product_id}_dependents})
          calligra_disable_product(${_dependent_product_id} "Required internal dependency missing: ${_product_id}")
        endforeach(_dependent_product_id ${CALLIGRA_PRODUCT_${_dep_product_id}_dependents})
      endif()
    endif()
  endforeach(_product_id)
endmacro()

# backward compatibility: BUILD_app as option or passed as cmake parameter can overwrite product set
# and disable a product if set to FALSE (TRUE was ignored)
macro(calligra_drop_products_on_old_flag _build_id)
  string(TOLOWER "${_build_id}" lowercase_build_id)
  if (DEFINED BUILD_${lowercase_build_id} AND NOT BUILD_${lowercase_build_id})
    foreach(_product_id ${ARGN})
      if (NOT DEFINED SHOULD_BUILD_${_product_id})
        message(FATAL_ERROR "Unknown product: ${_product_id}")
      endif ()
      calligra_disable_product(${_product_id} "Disabled by cmake parameter")
    endforeach(_product_id)
  endif ()
endmacro()

macro(calligra_set_productset _productset_string)
  set(CALLIGRA_SHOULD_BUILD_PRODUCTS "")
  # turn into list, _productset_string is not yet one
  set(_productset_ids ${_productset_string})
  separate_arguments(_productset_ids)
  set(_productset_list "")
  # _product_id could be a product, feature or product set (predefined or from file)
  foreach(_product_id ${_productset_ids})
    # be gracefull and compare the productset id case insensitive
    string(TOUPPER "${_product_id}" _uppercase_product_id)

    if(_uppercase_product_id STREQUAL "ALL")
      list( APPEND CALLIGRA_SHOULD_BUILD_PRODUCTS ${CALLIGRA_ALL_PRODUCTS})
    elseif(DEFINED SHOULD_BUILD_${_uppercase_product_id})
      list( APPEND CALLIGRA_SHOULD_BUILD_PRODUCTS ${_uppercase_product_id})
    else()
      # also expects a productset definition filename in all lowercase
      string(TOLOWER "${_product_id}" _lowercase_productset_id)
      set(_productset_filename "${CMAKE_SOURCE_DIR}/cmake/productsets/${_lowercase_productset_id}.cmake")

      if (NOT EXISTS "${_productset_filename}")
        message(FATAL_ERROR "Unknown product, feature or product set: ${_product_id}")
      endif ()

      # include the productset definition
      include(${_productset_filename})
      if(NOT CALLIGRA_PRODUCTSET_${_uppercase_product_id})
        message(FATAL_ERROR "Product set file \"${_productset_filename}\" did not define a productset named \"${_uppercase_product_id}\".")
      endif()

      list( APPEND CALLIGRA_SHOULD_BUILD_PRODUCTS ${_uppercase_product_id})
    endif()
    list( APPEND _productset_list ${_uppercase_product_id})
  endforeach(_product_id)

  string(REPLACE ";" " " _productset_list "${_productset_list}")
  set(CALLIGRA_NEEDED_PRODUCTS "")

  message(STATUS "-------------------------------------------------------------------" )
  message(STATUS "Configured with product set \"${_productset_list}\"")
  message(STATUS "-------------------------------------------------------------------" )

  # backward compatibility: BUILD_app as option or passed as cmake parameter can overwrite product set
  # and disable a product if set to FALSE (TRUE was ignored)
  foreach(_product_id ${CALLIGRA_ALL_PRODUCTS})
    string(TOLOWER "${_product_id}" lowercase_product_id)
    if (DEFINED BUILD_${lowercase_product_id})
      if(NOT BUILD_${lowercase_product_id})
        list(FIND CALLIGRA_SHOULD_BUILD_PRODUCTS ${_product_id} _index)
        # remove from product set, if part
        if(NOT _index EQUAL -1)
          list(REMOVE_AT CALLIGRA_SHOULD_BUILD_PRODUCTS ${_index})
        endif()
      endif()
    endif ()
  endforeach(_product_id ${CALLIGRA_ALL_PRODUCTS})

  # mark all products of the set as SHOULD_BUILD
  foreach(_product_id ${CALLIGRA_SHOULD_BUILD_PRODUCTS})
    # check that this product is actually existing
    if (NOT DEFINED SHOULD_BUILD_${_product_id})
      message(FATAL_ERROR "Unknown product: ${_product_id}")
    endif ()

    # mark product as should build, also all dependencies
    set(SHOULD_BUILD_${_product_id} TRUE)
    if (DEFINED CALLIGRA_PRODUCT_${_product_id}_needed_dependencies OR
        DEFINED CALLIGRA_PRODUCT_${_product_id}_wanted_dependencies)
        calligra_set_shouldbuild_productdependencies(${_product_id}
            "${CALLIGRA_PRODUCT_${_product_id}_needed_dependencies}"
            "${CALLIGRA_PRODUCT_${_product_id}_wanted_dependencies}")
    endif (DEFINED CALLIGRA_PRODUCT_${_product_id}_needed_dependencies OR
           DEFINED CALLIGRA_PRODUCT_${_product_id}_wanted_dependencies)
  endforeach(_product_id)
endmacro()


# Usage:
#   calligra_define_product(<product_id>
#         [NAME] <product_name>
#         [STAGING]
#         [REQUIRES <product_id1> <feature_id1> ...]
#       )
macro(calligra_define_product _product_id)
  # default product name to id, empty deps
  set(_product_name "${_product_id}")
  set(_needed_dep_product_ids)

  if(DEFINED SHOULD_BUILD_${_product_id})
    message(FATAL_ERROR "Product \"${_product_id}\" already defined, as \"${CALLIGRA_PRODUCT_${_product_id}_name}\".")
  endif()

  # parse arguments: two states, "name" or "requires"
  set(_current_arg_type "name")
  foreach(_arg ${ARGN})
    if(${_arg} STREQUAL "NAME")
      set(_current_arg_type "name")
    elseif(${_arg} STREQUAL "REQUIRES")
      set(_current_arg_type "requires")
    else()
      if(${_current_arg_type} STREQUAL "name")
        if(${_arg} STREQUAL "STAGING")
          list(APPEND CALLIGRA_STAGING_PRODUCTS ${_product_id})
        else()
          set(_product_name "${_arg}")
        endif()
      elseif(${_current_arg_type} STREQUAL "requires")
        # check that the dependency is actually existing
        if(NOT DEFINED SHOULD_BUILD_${_arg})
          message(FATAL_ERROR "Unknown product/feature listed as dependency for \"${_product_id}\": \"${_arg}\"")
        elseif(CALLIGRA_PRODUCTSET_${_arg})
          message(FATAL_ERROR "Productset cannot be a dependency for \"${_product_id}\": \"${_arg}\"")
        endif()
        list(APPEND _needed_dep_product_ids "${_arg}")
      endif()
    endif()
  endforeach(_arg)

  # set product vars
  set(SHOULD_BUILD_${_product_id} FALSE)
  set(CALLIGRA_PRODUCT_${_product_id}_name "${_product_name}")
  set(CALLIGRA_PRODUCT_${_product_id}_needed_dependencies ${_needed_dep_product_ids})
  list(APPEND CALLIGRA_ALL_PRODUCTS ${_product_id})
endmacro(calligra_define_product)


# Usage:
#   calligra_define_feature(<feature_id>
#         [NAME] <feature_name>
#         [STAGING]
#         [REQUIRES <product_id1> <feature_id1> ...]
#       )
macro(calligra_define_feature _product_id)
  # default product name to id, empty deps
  set(_product_name "${_product_id}")
  set(_needed_dep_product_ids)

  if(DEFINED SHOULD_BUILD_${_product_id})
    message(FATAL_ERROR "Feature \"${_product_id}\" already defined, as \"${CALLIGRA_PRODUCT_${_product_id}_name}\".")
  endif()

  # parse arguments: two states, "name" or "requires"
  set(_current_arg_type "name")
  foreach(_arg ${ARGN})
    if(${_arg} STREQUAL "NAME")
      set(_current_arg_type "name")
    elseif(${_arg} STREQUAL "REQUIRES")
      set(_current_arg_type "requires")
    else()
      if(${_current_arg_type} STREQUAL "name")
        if(${_arg} STREQUAL "STAGING")
          list(APPEND CALLIGRA_STAGING_PRODUCTS ${_product_id})
        else()
          set(_product_name "${_arg}")
        endif()
      elseif(${_current_arg_type} STREQUAL "requires")
        # check that the dependency is actually existing
        if(NOT DEFINED SHOULD_BUILD_${_arg})
          message(FATAL_ERROR "Unknown product/feature listed as dependency for \"${_product_id}\": \"${_arg}\"")
        elseif(CALLIGRA_PRODUCTSET_${_arg})
          message(FATAL_ERROR "Productset cannot be a dependency for \"${_product_id}\": \"${_arg}\"")
        endif()
        list(APPEND _needed_dep_product_ids "${_arg}")
      endif()
    endif()
  endforeach(_arg)

  # set product vars
  set(SHOULD_BUILD_${_product_id} FALSE)
  set(CALLIGRA_PRODUCT_${_product_id}_name "${_product_name}")
  set(CALLIGRA_PRODUCT_${_product_id}_needed_dependencies ${_needed_dep_product_ids})
  list(APPEND CALLIGRA_ALL_PRODUCTS ${_product_id})
endmacro(calligra_define_feature)


# Usage:
#   calligra_define_productset(<productset_id>
#         [NAME] <productset_name>
#         [REQUIRES <product_id1> ... <productset_id1> ...]
#         [OPTIONAL <product_id2> ... <productset_id2> ...]
#       )
macro(calligra_define_productset _product_id)
  # default product name to id, empty deps
  set(_product_name "${_product_id}")
  set(_needed_dep_product_ids)
  set(_wanted_dep_product_ids)

  if(DEFINED SHOULD_BUILD_${_product_id})
    message(FATAL_ERROR "Productset \"${_product_id}\" already defined, as \"${CALLIGRA_PRODUCT_${_product_id}_name}\".")
  endif()

  # parse arguments: three states, "name", "requires" or "optional"
  set(_current_arg_type "name")
  foreach(_arg ${ARGN})
    if(${_arg} STREQUAL "NAME")
      set(_current_arg_type "name")
    elseif(${_arg} STREQUAL "REQUIRES")
      set(_current_arg_type "requires")
    elseif(${_arg} STREQUAL "OPTIONAL")
      set(_current_arg_type "optional")
    else()
      if(${_current_arg_type} STREQUAL "name")
        set(_product_name "${_arg}")
      elseif(${_current_arg_type} STREQUAL "requires")
        # check that the dependency is actually existing
        if(NOT DEFINED SHOULD_BUILD_${_arg})
          message(FATAL_ERROR "Unknown product(set)/feature listed as dependency for \"${_product_id}\": \"${_arg}\"")
        endif()
        list(APPEND _needed_dep_product_ids "${_arg}")
      elseif(${_current_arg_type} STREQUAL "optional")
        # check that the dependency is actually existing
        if(NOT DEFINED SHOULD_BUILD_${_arg})
          message(FATAL_ERROR "Unknown product(set)/feature listed as dependency for \"${_product_id}\": \"${_arg}\"")
        endif()
        list(APPEND _wanted_dep_product_ids "${_arg}")
      endif()
    endif()
  endforeach(_arg)

  # set product(set) vars
  set(SHOULD_BUILD_${_product_id} FALSE)
  set(CALLIGRA_PRODUCT_${_product_id}_name "${_product_name}")
  set(CALLIGRA_PRODUCTSET_${_product_id} TRUE)
  set(CALLIGRA_PRODUCT_${_product_id}_needed_dependencies ${_needed_dep_product_ids})
  set(CALLIGRA_PRODUCT_${_product_id}_wanted_dependencies ${_wanted_dep_product_ids})
  list(APPEND CALLIGRA_ALL_PRODUCTS ${_product_id})
endmacro(calligra_define_productset)


macro(calligra_log_should_build)
  # find what products will be built and which not
  set(_built_product_ids "")
  set(_not_built_product_ids "")
  set(_built_dependency_product_ids "")
  set(_not_built_dependency_product_ids "")

  foreach(_product_id ${CALLIGRA_ALL_PRODUCTS})
    list(FIND CALLIGRA_SHOULD_BUILD_PRODUCTS ${_product_id} _index)
    if(NOT _index EQUAL -1)
      if(SHOULD_BUILD_${_product_id})
        list(APPEND _built_product_ids ${_product_id})
      else()
        list(APPEND _not_built_product_ids ${_product_id})
      endif()
    else()
      if(SHOULD_BUILD_${_product_id})
        list(APPEND _built_dependency_product_ids ${_product_id})
      else()
        list(FIND CALLIGRA_NEEDED_PRODUCTS ${_product_id} _index2)
        if(NOT _index2 EQUAL -1)
          list(APPEND _not_built_dependency_product_ids ${_product_id})
        endif()
      endif()
    endif()
  endforeach(_product_id)

  if(NOT _built_dependency_product_ids STREQUAL "")
    message(STATUS "------ The following required product(set)s/features will be built -------")
    foreach(_product_id ${_built_dependency_product_ids})
      if (DEFINED CALLIGRA_PRODUCT_${_product_id}_dependents)
        set(dependents "   [[needed by: ${CALLIGRA_PRODUCT_${_product_id}_dependents}]]")
      else ()
        set(dependents "")
      endif ()

      message(STATUS "${_product_id}:  ${CALLIGRA_PRODUCT_${_product_id}_name}${dependents}" )
    endforeach(_product_id)
    message(STATUS "")
  endif()
  if(NOT _not_built_dependency_product_ids STREQUAL "")
    message(STATUS "---- The following required product(set)s/features can NOT be built ------")
    foreach(_product_id ${_not_built_dependency_product_ids})
      if (DEFINED CALLIGRA_PRODUCT_${_product_id}_dependents)
        set(dependents "   [[needed by: ${CALLIGRA_PRODUCT_${_product_id}_dependents}]]")
      else ()
        set(dependents "")
      endif ()

      message(STATUS "${_product_id}:  ${CALLIGRA_PRODUCT_${_product_id}_name}${dependents}  |  ${BUILD_${_product_id}_DISABLE_REASON}" )
    endforeach(_product_id)
    message(STATUS "")
  endif()
  message(STATUS "------ The following product(set)s/features will be built ---------")
  foreach(_product_id ${_built_product_ids})
      message(STATUS "${_product_id}:  ${CALLIGRA_PRODUCT_${_product_id}_name}" )
  endforeach(_product_id)
  if(NOT _not_built_product_ids STREQUAL "")
    message(STATUS "\n------ The following product(set)s/features can NOT be built ------")
    foreach(_product_id ${_not_built_product_ids})
        message(STATUS "${_product_id}:  ${CALLIGRA_PRODUCT_${_product_id}_name}  |  ${BUILD_${_product_id}_DISABLE_REASON}" )
    endforeach(_product_id)
  endif()
  message(STATUS "-------------------------------------------------------------------" )
endmacro(calligra_log_should_build)


macro(calligra_product_deps_report_stylebybuild _output _product_id)
  if(SHOULD_BUILD_${_product_id})
    set(${_output} "filled")
  else()
    set(${_output} "solid")
  endif()
endmacro(calligra_product_deps_report_stylebybuild)

# Usage:
#   calligra_product_deps_report(<filename_without_extension>)
macro(calligra_product_deps_report _filename)
  set(_dot "${_dot}# This is a graphviz file. It shows the internal product dependencies of Calligra.\n")
  set(_dot "${_dot}# dot -Tsvg ${_filename}.dot > ${_filename}.svg\n")
  set(_dot "${_dot}# dot -Tpng ${_filename}.dot > ${_filename}.png\n")
  set(_dot "${_dot}digraph calligra {\n")
  set(_dot "${_dot}node [colorscheme=set312]\;\n") # pastel19, set312 or accent8
  set(_dot "${_dot}rankdir=LR\;\n")

  foreach(_product_id ${CALLIGRA_ALL_PRODUCTS})
    set(_color 11)
    set(_shape "box")
    set(_style "")

    if(CALLIGRA_PRODUCTSET_${_product_id})
      set(_color 1)
      set(_shape "folder")
    elseif(_product_id MATCHES "^LIB_")
      set(_color 2)
      set(_shape "box")
    elseif(_product_id MATCHES "^FILTER_")
      set(_color 3)
      set(_shape "component")
    elseif(_product_id MATCHES "^PLUGIN_")
      set(_color 4)
      set(_shape "component")
    elseif(_product_id MATCHES "^PART_")
      set(_color 5)
      set(_shape "component")
    elseif(_product_id MATCHES "^APP_")
      set(_color 6)
      set(_shape "box")
      set(_style "rounded,")
    elseif(_product_id MATCHES "^BUILDUTIL_")
      set(_color 7)
      set(_shape "diamond")
      set(_style "rounded,")
    elseif(_product_id MATCHES "^FEATURE_")
      set(_color 8)
    elseif(_product_id MATCHES "^OKULAR_")
      set(_color 9)
      set(_shape "component")
    elseif(_product_id MATCHES "^FILEMANAGER_")
      set(_color 10)
      set(_shape "box")
    endif()

    calligra_product_deps_report_stylebybuild(_stylebybuild ${_product_id})

    set(_dot "${_dot}\"${_product_id}\" [fillcolor=${_color}, shape=${_shape}, style=\"${_style}${_stylebybuild}\"];\n")

    foreach(_dependent_product_id ${CALLIGRA_PRODUCT_${_product_id}_needed_dependencies})
      set(_edges "${_edges}\"${_product_id}\" -> \"${_dependent_product_id}\"\;\n")
    endforeach(_dependent_product_id)
    foreach(_dependent_product_id ${CALLIGRA_PRODUCT_${_product_id}_wanted_dependencies})
      set(_edges "${_edges}\"${_product_id}\" -> \"${_dependent_product_id}\" [style=dashed]\;\n")
    endforeach(_dependent_product_id)
  endforeach(_product_id)

  set(_dot "${_dot}${_edges}")
  set(_dot "${_dot}}\n")
  file(WRITE ${CMAKE_BINARY_DIR}/${_filename}.dot ${_dot})
endmacro(calligra_product_deps_report)
