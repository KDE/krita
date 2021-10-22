# SPDX-FileCopyrightText: 2003-2018 University of Illinois at Urbana-Champaign.
#
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
CheckAtomic
-----------

Check if the compiler supports std:atomic out of the box or if libatomic is
needed for atomic support. If it is needed libatomicis added to
``CMAKE_REQUIRED_LIBRARIES``. So after running CheckAtomic you can use
std:atomic.

Since 5.75.0.
#]=======================================================================]

include(CheckCXXSourceCompiles)
include(CheckLibraryExists)

# Sometimes linking against libatomic is required for atomic ops, if
# the platform doesn't support lock-free atomics.

function(check_working_cxx_atomics varname)
	set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++11")
	check_cxx_source_compiles("
	#include <atomic>
	std::atomic<int> x;
	std::atomic<short> y;
	std::atomic<char> z;
	int main() {
		++z;
		++y;
		return ++x;
	}
	" ${varname})
	set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endfunction()

function(check_working_cxx_atomics64 varname)
	set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_FLAGS "-std=c++11 ${CMAKE_REQUIRED_FLAGS}")
	check_cxx_source_compiles("
	#include <atomic>
	#include <cstdint>
	std::atomic<uint64_t> x (0);
	int main() {
		uint64_t i = x.load(std::memory_order_relaxed);
		return 0;
	}
	" ${varname})
	set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endfunction()

# Check for (non-64-bit) atomic operations.
if(MSVC)
	set(HAVE_CXX_ATOMICS_WITHOUT_LIB True)
else()
	# First check if atomics work without the library.
	if (CMAKE_COMPILER_IS_GNUCXX
		OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang"
		OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"
		OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
		check_working_cxx_atomics(HAVE_CXX_ATOMICS_WITHOUT_LIB)
	endif()
	# If not, check if the library exists, and atomics work with it.
	if(NOT HAVE_CXX_ATOMICS_WITHOUT_LIB)
		check_library_exists(atomic __atomic_fetch_add_4 "" HAVE_LIBATOMIC)
		if(HAVE_LIBATOMIC)
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
	# First check if atomics work without the library.
	if (CMAKE_COMPILER_IS_GNUCXX
		OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang"
		OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"
		OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
		check_working_cxx_atomics64(HAVE_CXX_ATOMICS64_WITHOUT_LIB)
	endif()
	# If not, check if the library exists, and atomics work with it.
	if(NOT HAVE_CXX_ATOMICS64_WITHOUT_LIB)
		check_library_exists(atomic __atomic_load_8 "" HAVE_CXX_LIBATOMICS64)
		if(HAVE_CXX_LIBATOMICS64)
			list(APPEND CMAKE_REQUIRED_LIBRARIES "atomic")
			check_working_cxx_atomics64(HAVE_CXX_ATOMICS64_WITH_LIB)
			if (NOT HAVE_CXX_ATOMICS64_WITH_LIB)
				message(FATAL_ERROR "Host compiler must support 64-bit std::atomic!")
			endif()
		else()
			message(FATAL_ERROR "Host compiler appears to require libatomic for 64-bit operations, but cannot find it.")
		endif()
	endif()
endif()
