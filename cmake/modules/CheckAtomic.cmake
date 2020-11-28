# ==============================================================================
# LLVM Release License
# ==============================================================================
# University of Illinois/NCSA
# Open Source License
#
# SPDX-FileCopyrightText: 2003-2018 University of Illinois at Urbana-Champaign.
# All rights reserved.
#
# Developed by:
#
# LLVM Team
#
# University of Illinois at Urbana-Champaign
#
# http://llvm.org
#
# SPDX-License-Identifier: BSD-3-Clause

INCLUDE(CheckCXXSourceCompiles)
INCLUDE(CheckLibraryExists)

# Sometimes linking against libatomic is required for atomic ops, if
# the platform doesn't support lock-free atomics.

function(check_working_cxx_atomics varname)
set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++11")
CHECK_CXX_SOURCE_COMPILES("
#include <atomic>
std::atomic<int> x;
int main() {
return std::atomic_is_lock_free(&x);
}
" ${varname})
set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endfunction(check_working_cxx_atomics)

function(check_working_cxx_atomics64 varname)
set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_FLAGS "-std=c++11 ${CMAKE_REQUIRED_FLAGS}")
CHECK_CXX_SOURCE_COMPILES("
#include <atomic>
#include <cstdint>
std::atomic<uint64_t> x (0);
int main() {
uint64_t i = x.load(std::memory_order_relaxed);
return std::atomic_is_lock_free(&x);
}
" ${varname})
set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endfunction(check_working_cxx_atomics64)


# This isn't necessary on MSVC, so avoid command-line switch annoyance
# by only running on GCC-like hosts.
if (LLVM_COMPILER_IS_GCC_COMPATIBLE)
	# First check if atomics work without the library.
	check_working_cxx_atomics(HAVE_CXX_ATOMICS_WITHOUT_LIB)
	# If not, check if the library exists, and atomics work with it.
	if(NOT HAVE_CXX_ATOMICS_WITHOUT_LIB)
		check_library_exists(atomic __atomic_fetch_add_4 "" HAVE_LIBATOMIC)
		if( HAVE_LIBATOMIC )
			list(APPEND CMAKE_REQUIRED_LIBRARIES "atomic")
			check_working_cxx_atomics(HAVE_CXX_ATOMICS_WITH_LIB)
			if (NOT HAVE_CXX_ATOMICS_WITH_LIB)
				message(FATAL_ERROR "Host compiler must support std::atomic!")
			endif()
		else()
			message(FATAL_ERROR "Host compiler appears to require libatomic, but cannot find it.")
		endif()
	endif()
endif()
# Check for 64 bit atomic operations.
if(MSVC)
	set(HAVE_CXX_ATOMICS64_WITHOUT_LIB True)
else()
	check_working_cxx_atomics64(HAVE_CXX_ATOMICS64_WITHOUT_LIB)
endif()

# If not, check if the library exists, and atomics work with it.
if(NOT HAVE_CXX_ATOMICS64_WITHOUT_LIB)
	check_library_exists(atomic __atomic_load_8 "" HAVE_CXX_LIBATOMICS64)
	if(HAVE_CXX_LIBATOMICS64)
		list(APPEND CMAKE_REQUIRED_LIBRARIES "atomic")
		check_working_cxx_atomics64(HAVE_CXX_ATOMICS64_WITH_LIB)
		if (NOT HAVE_CXX_ATOMICS64_WITH_LIB)
			message(FATAL_ERROR "Host compiler must support std::atomic!")
		endif()
	else()
		message(FATAL_ERROR "Host compiler appears to require libatomic, but cannot find it.")
	endif()
endif()
