############################################################################
# FindVPX.txt
# Copyright (C) 2014  Belledonne Communications, Grenoble France
#
############################################################################
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
############################################################################
#
# - Find the VPX include file and library
#
#  VPX_FOUND - system has VPX
#  VPX_INCLUDE_DIRS - the VPX include directory
#  VPX_LIBRARIES - The libraries needed to use VPX

set(_VPX_ROOT_PATHS
	${CMAKE_INSTALL_PREFIX}
)

find_path(VPX_INCLUDE_DIRS
	NAMES vpx/vpx_encoder.h
	HINTS _VPX_ROOT_PATHS
	PATH_SUFFIXES include
)
if(VPX_INCLUDE_DIRS)
	set(HAVE_VPX_VPX_ENCODER_H 1)
endif()

find_library(VPX_LIBRARIES
	NAMES vpx vpxmd
	HINTS _VPX_ROOT_PATHS
	PATH_SUFFIXES bin lib lib/Win32
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VPX
	DEFAULT_MSG
	VPX_INCLUDE_DIRS VPX_LIBRARIES HAVE_VPX_VPX_ENCODER_H
)

mark_as_advanced(VPX_INCLUDE_DIRS VPX_LIBRARIES HAVE_VPX_VPX_ENCODER_H)