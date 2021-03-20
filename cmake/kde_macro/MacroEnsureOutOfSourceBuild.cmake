# - MACRO_ENSURE_OUT_OF_SOURCE_BUILD(<errorMessage>)
# MACRO_ENSURE_OUT_OF_SOURCE_BUILD(<errorMessage>)
#    Call this macro in your project if you want to enforce out-of-source builds.
#    If an in-source build is detected, it will abort with the given error message.
#    This macro works in any of the CMakeLists.txt of your project, but the recommended
#    location to call this is close to the beginning of the top level CMakeLists.txt

# SPDX-FileCopyrightText: 2006 Alexander Neundorf <neundorf@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#

MACRO (MACRO_ENSURE_OUT_OF_SOURCE_BUILD _errorMessage)

   STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
   IF(insource)
      MESSAGE(FATAL_ERROR "${_errorMessage}")
   ENDIF(insource)

ENDMACRO (MACRO_ENSURE_OUT_OF_SOURCE_BUILD)
