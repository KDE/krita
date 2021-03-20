
# SPDX-FileCopyrightText: 2006 Alexander Neundorf <neundorf@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#

if (UNIX)

file(WRITE "${_filename}"
"#!/bin/sh
# created by cmake, don't edit, changes will be lost

${_library_path_variable}=${_ld_library_path}\${${_library_path_variable}:+:\$${_library_path_variable}} \"${_executable}\" \"$@\"
")

# make it executable
# since this is only executed on UNIX, it is safe to call chmod
exec_program(chmod ARGS ug+x \"${_filename}\" OUTPUT_VARIABLE _dummy )

else (UNIX)

file(TO_NATIVE_PATH "${_ld_library_path}" win_path)

file(WRITE "${_filename}"
"
set PATH=${win_path};$ENV{PATH}
\"${_executable}\" %*
")

endif (UNIX)
