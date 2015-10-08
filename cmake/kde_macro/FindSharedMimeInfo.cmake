# - Try to find the shared-mime-info package
#
# Once done this will define
#
#  SHAREDMIMEINFO_FOUND - system has the shared-mime-info package
#  UPDATE_MIME_DATABASE_EXECUTABLE - the update-mime-database executable
#
# The minimum required version of SharedMimeInfo can be specified using the
# standard syntax, e.g. find_package(SharedMimeInfo 0.20)
#
# For backward compatibility, the following two variables are also supported:
#  SHARED_MIME_INFO_FOUND - same as SHAREDMIMEINFO_FOUND
#  SHARED_MIME_INFO_MINIMUM_VERSION - set to the minimum version you need, default is 0.18.
#    When both are used, i.e. the version is set in the find_package() call and
#   SHARED_MIME_INFO_MINIMUM_VERSION is set, the version specified in the find_package()
#   call takes precedence.


# Copyright (c) 2007, Pino Toscano, <toscano.pino@tiscali.it>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Support SHARED_MIME_INFO_MINIMUM_VERSION for compatibility:
if(NOT SharedMimeInfo_FIND_VERSION)
  set(SharedMimeInfo_FIND_VERSION "${SHARED_MIME_INFO_MINIMUM_VERSION}")
endif(NOT SharedMimeInfo_FIND_VERSION)

# the minimum version of shared-mime-database we require
if(NOT SharedMimeInfo_FIND_VERSION)
  set(SharedMimeInfo_FIND_VERSION "0.18")
endif(NOT SharedMimeInfo_FIND_VERSION)

find_program (UPDATE_MIME_DATABASE_EXECUTABLE NAMES update-mime-database)

# Store the version number in the cache, so we don't have to search the next time again:
if (UPDATE_MIME_DATABASE_EXECUTABLE  AND NOT  SHAREDMIMEINFO_VERSION)

    exec_program (${UPDATE_MIME_DATABASE_EXECUTABLE} ARGS -v RETURN_VALUE _null OUTPUT_VARIABLE _smiVersionRaw)

    string(REGEX REPLACE "update-mime-database \\([a-zA-Z\\-]+\\) ([0-9]\\.[0-9]+).*"
           "\\1" smiVersion "${_smiVersionRaw}")

    set(SHAREDMIMEINFO_VERSION "${smiVersion}" CACHE STRING "Version number of SharedMimeInfo" FORCE)
endif (UPDATE_MIME_DATABASE_EXECUTABLE  AND NOT  SHAREDMIMEINFO_VERSION)

# Use the new FPHSA() syntax:
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SharedMimeInfo REQUIRED_VARS UPDATE_MIME_DATABASE_EXECUTABLE
                                                 VERSION_VAR SHAREDMIMEINFO_VERSION )

# For backward compatibility:
set(SHARED_MIME_INFO_FOUND ${SHAREDMIMEINFO_FOUND} )

# This should go into MacroLogFeature/FeatureSummary:
#            message(FATAL_ERROR "Could NOT find shared-mime-info. See http://freedesktop.org/wiki/Software/shared-mime-info.")


mark_as_advanced(UPDATE_MIME_DATABASE_EXECUTABLE)


macro(UPDATE_XDG_MIMETYPES _path)
   get_filename_component(_xdgmimeDir "${_path}" NAME)
   if("${_xdgmimeDir}" STREQUAL packages )
      get_filename_component(_xdgmimeDir "${_path}" PATH)
   else("${_xdgmimeDir}" STREQUAL packages )
      set(_xdgmimeDir "${_path}")
   endif("${_xdgmimeDir}" STREQUAL packages )

   install(CODE "
set(DESTDIR_VALUE \"\$ENV{DESTDIR}\")
if (NOT DESTDIR_VALUE)
  # under Windows relative paths are used, that's why it runs from CMAKE_INSTALL_PREFIX
  execute_process(COMMAND ${UPDATE_MIME_DATABASE_EXECUTABLE} ${_xdgmimeDir}
                  WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\")
endif (NOT DESTDIR_VALUE)
")
endmacro (UPDATE_XDG_MIMETYPES)
