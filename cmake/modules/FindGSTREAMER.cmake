# - Try to find GStreamer and its plugins
# Once done, this will define
#
#  GSTREAMER_FOUND - system has GStreamer
#  GSTREAMER_INCLUDE_DIRS - the GStreamer include directories
#  GSTREAMER_LIBRARIES - link these to use GStreamer
#
# Additionally, gstreamer-base is always looked for and required, and
# the following related variables are defined:
#
#  GSTREAMER_BASE_INCLUDE_DIRS - gstreamer-base's include directory
#  GSTREAMER_BASE_LIBRARIES - link to these to use gstreamer-base
#
# Optionally, the COMPONENTS keyword can be passed to find_package()
# and GStreamer plugins can be looked for.  Currently, the following
# plugins can be searched, and they define the following variables if
# found:
#
#  gstreamer-app:        GSTREAMER_APP_INCLUDE_DIRS and GSTREAMER_APP_LIBRARIES
#  gstreamer-audio:      GSTREAMER_AUDIO_INCLUDE_DIRS and GSTREAMER_AUDIO_LIBRARIES
#  gstreamer-fft:        GSTREAMER_FFT_INCLUDE_DIRS and GSTREAMER_FFT_LIBRARIES
#  gstreamer-mpegts:     GSTREAMER_MPEGTS_INCLUDE_DIRS and GSTREAMER_MPEGTS_LIBRARIES
#  gstreamer-pbutils:    GSTREAMER_PBUTILS_INCLUDE_DIRS and GSTREAMER_PBUTILS_LIBRARIES
#  gstreamer-tag:        GSTREAMER_TAG_INCLUDE_DIRS and GSTREAMER_TAG_LIBRARIES
#  gstreamer-video:      GSTREAMER_VIDEO_INCLUDE_DIRS and GSTREAMER_VIDEO_LIBRARIES
#
# Copyright (C) 2012 Raphael Kubo da Costa <rakuco@webkit.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

find_package(PkgConfig REQUIRED QUIET)

# Helper macro to find a GStreamer plugin (or GStreamer itself)
#   _component_prefix is prepended to the _INCLUDE_DIRS and _LIBRARIES variables (eg. "GSTREAMER_AUDIO")
#   _pkgconfig_name is the component's pkg-config name (eg. "gstreamer-1.0", or "gstreamer-video-1.0").
#   _header is the component's header, relative to the gstreamer-1.0 directory (eg. "gst/gst.h").
#   _library is the component's library name (eg. "gstreamer-1.0" or "gstvideo-1.0")
macro(FIND_GSTREAMER_COMPONENT _component_prefix _pkgconfig_name _header _library)
	pkg_check_modules(PC_${_component_prefix} QUIET ${_pkgconfig_name})

	find_path(${_component_prefix}_INCLUDE_DIRS
		NAMES ${_header}
		HINTS ${PC_${_component_prefix}_INCLUDE_DIRS} ${PC_${_component_prefix}_INCLUDEDIR}
		PATH_SUFFIXES gstreamer-1.0
	)

	find_library(${_component_prefix}_LIBRARIES
		NAMES ${_library}
		HINTS ${PC_${_component_prefix}_LIBRARY_DIRS} ${PC_${_component_prefix}_LIBDIR}
	)
endmacro()

# ------------------------
# 1. Find GStreamer itself
# ------------------------

# 1.1. Find headers and libraries
FIND_GSTREAMER_COMPONENT(GSTREAMER gstreamer-1.0 gst/gst.h gstreamer-1.0)
FIND_GSTREAMER_COMPONENT(GSTREAMER_CONFIG gstreamer-1.0 gst/gstconfig.h gstreamer-1.0)
FIND_GSTREAMER_COMPONENT(GSTREAMER_BASE gstreamer-base-1.0 gst/base/gstadapter.h gstbase-1.0)

# 1.2. Check GStreamer version
if (GSTREAMER_INCLUDE_DIRS)
	if (EXISTS "${GSTREAMER_INCLUDE_DIRS}/gst/gstversion.h")
		file(READ "${GSTREAMER_INCLUDE_DIRS}/gst/gstversion.h" GSTREAMER_VERSION_CONTENTS)

		string(REGEX MATCH "#define +GST_VERSION_MAJOR +\\(([0-9]+)\\)" _dummy "${GSTREAMER_VERSION_CONTENTS}")
		set(GSTREAMER_VERSION_MAJOR "${CMAKE_MATCH_1}")

		string(REGEX MATCH "#define +GST_VERSION_MINOR +\\(([0-9]+)\\)" _dummy "${GSTREAMER_VERSION_CONTENTS}")
		set(GSTREAMER_VERSION_MINOR "${CMAKE_MATCH_1}")

		string(REGEX MATCH "#define +GST_VERSION_MICRO +\\(([0-9]+)\\)" _dummy "${GSTREAMER_VERSION_CONTENTS}")
		set(GSTREAMER_VERSION_MICRO "${CMAKE_MATCH_1}")

		set(GSTREAMER_VERSION "${GSTREAMER_VERSION_MAJOR}.${GSTREAMER_VERSION_MINOR}.${GSTREAMER_VERSION_MICRO}")
	endif ()
endif ()

if ("${GStreamer_FIND_VERSION}" VERSION_GREATER "${GSTREAMER_VERSION}")
	message(FATAL_ERROR "Required version (" ${GStreamer_FIND_VERSION} ") is higher than found version (" ${GSTREAMER_VERSION} ")")
endif ()

# -------------------------
# 2. Find GStreamer plugins
# -------------------------

FIND_GSTREAMER_COMPONENT(GSTREAMER_APP gstreamer-app-1.0 gst/app/gstappsink.h gstapp-1.0)
FIND_GSTREAMER_COMPONENT(GSTREAMER_AUDIO gstreamer-audio-1.0 gst/audio/audio.h gstaudio-1.0)
FIND_GSTREAMER_COMPONENT(GSTREAMER_FFT gstreamer-fft-1.0 gst/fft/gstfft.h gstfft-1.0)
FIND_GSTREAMER_COMPONENT(GSTREAMER_MPEGTS gstreamer-mpegts-1.0>=1.4.0 gst/mpegts/mpegts.h gstmpegts-1.0)
FIND_GSTREAMER_COMPONENT(GSTREAMER_PBUTILS gstreamer-pbutils-1.0 gst/pbutils/pbutils.h gstpbutils-1.0)
FIND_GSTREAMER_COMPONENT(GSTREAMER_TAG gstreamer-tag-1.0 gst/tag/tag.h gsttag-1.0)
FIND_GSTREAMER_COMPONENT(GSTREAMER_VIDEO gstreamer-video-1.0 gst/video/video.h gstvideo-1.0)

list(APPEND GSTREAMER_INCLUDE_DIRS ${GSTREAMER_CONFIG_INCLUDE_DIRS})
list(APPEND GSTREAMER_LIBRARIES ${GSTREAMER_CONFIG_LIBRARIES})

# ------------------------------------------------
# 3. Process the COMPONENTS passed to FIND_PACKAGE
# ------------------------------------------------
set(_GSTREAMER_REQUIRED_VARS GSTREAMER_INCLUDE_DIRS GSTREAMER_LIBRARIES GSTREAMER_VERSION GSTREAMER_BASE_INCLUDE_DIRS GSTREAMER_BASE_LIBRARIES)

foreach (_component ${GStreamer_FIND_COMPONENTS})
	set(_gst_component "GSTREAMER_${_component}")
	string(TOUPPER ${_gst_component} _UPPER_NAME)

	list(APPEND _GSTREAMER_REQUIRED_VARS ${_UPPER_NAME}_INCLUDE_DIRS ${_UPPER_NAME}_LIBRARIES)
endforeach ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GStreamer REQUIRED_VARS _GSTREAMER_REQUIRED_VARS
											VERSION_VAR   GSTREAMER_VERSION)

mark_as_advanced(
	GSTREAMER_APP_INCLUDE_DIRS
	GSTREAMER_APP_LIBRARIES
	GSTREAMER_AUDIO_INCLUDE_DIRS
	GSTREAMER_AUDIO_LIBRARIES
	GSTREAMER_BASE_INCLUDE_DIRS
	GSTREAMER_BASE_LIBRARIES
	GSTREAMER_CONFIG_INCLUDE_DIRS
	GSTREAMER_CONFIG_LIBRARIES
	GSTREAMER_FFT_INCLUDE_DIRS
	GSTREAMER_FFT_LIBRARIES
	GSTREAMER_INCLUDE_DIRS
	GSTREAMER_LIBRARIES
	GSTREAMER_MPEGTS_INCLUDE_DIRS
	GSTREAMER_MPEGTS_LIBRARIES
	GSTREAMER_PBUTILS_INCLUDE_DIRS
	GSTREAMER_PBUTILS_LIBRARIES
	GSTREAMER_TAG_INCLUDE_DIRS
	GSTREAMER_TAG_LIBRARIES
	GSTREAMER_VIDEO_INCLUDE_DIRS
	GSTREAMER_VIDEO_LIBRARIES
)