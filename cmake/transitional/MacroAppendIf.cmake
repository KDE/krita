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

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(macro_append_if _cond _list)
  if(${_cond})
    list(APPEND ${_list} ${ARGN})
  endif(${_cond})
endmacro(macro_append_if _cond _list)
