# - MACRO_ADD_COMPILE_FLAGS(<_target> "flags...")

# SPDX-FileCopyrightText: 2006 Oswald Buddenhagen <ossi@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#

MACRO (MACRO_ADD_COMPILE_FLAGS _target _flg)

   GET_TARGET_PROPERTY(_flags ${_target} COMPILE_FLAGS)
   if (_flags)
      set(_flags "${_flags} ${_flg}")
   else (_flags)
      set(_flags "${_flg}")
   endif (_flags)
   SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS "${_flags}")

ENDMACRO (MACRO_ADD_COMPILE_FLAGS)
