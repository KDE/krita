# MACRO_APPEND_IF(CONDITION VAR VALUE1...VALUEN )
# This convenience macro appends the values VALUE1 up to VALUEN to the list
# given in VAR, but only if the variable CONDITION is TRUE:
#
# usage example:
# IF(SOMELIB_FOUND)
#   SET(my_sources ${my_sources} somefile.c someotherfile.c)
# ENDIF(SOMELIB_FOUND)
#
# becomes:
# MACRO_APPEND_IF(SOMELIB_FOUND  my_sources  somefile.c someotherfile.c)

# SPDX-FileCopyrightText: 2006 Alexander Neundorf <neundorf@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#

macro(macro_append_if _cond _list)
  if(${_cond})
    list(APPEND ${_list} ${ARGN})
  endif(${_cond})
endmacro(macro_append_if _cond _list)
