# Precompiled Header creation for C++
#
# Copyright (c) CMakePCHCompiler Authors. All rights reserved.
# This code is licensed under the MIT License, see LICENSE.
#
# Defines compiler command templates based on existing C++ compiler.

include(CMakePCHCompiler)

__define_pch_compiler(CXX)

mark_as_advanced(
	CMAKE_CXXPCH_FLAGS
	CMAKE_CXXPCH_FLAGS_DEBUG
	CMAKE_CXXPCH_FLAGS_MINSIZEREL
	CMAKE_CXXPCH_FLAGS_RELEASE
	CMAKE_CXXPCH_FLAGS_RELWITHDEBINFO
	)
