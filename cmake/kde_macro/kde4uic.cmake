
# SPDX-FileCopyrightText: 2006 Alexander Neundorf <neundorf@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(KDE_UIC_EXECUTABLE uic)
  EXECUTE_PROCESS(COMMAND ${KDE_UIC_EXECUTABLE}
    -tr tr2i18n
    ${KDE_UIC_FILE}
    OUTPUT_VARIABLE _uic_CONTENTS
  )

   set(KDE_UIC_CPP_FILE ${KDE_UIC_H_FILE})


IF (_uic_CONTENTS)
  #replace tr218n("") with QString::null to avoid warning from KLocale
  STRING(REGEX REPLACE "tr2i18n\\(\"\"\\)" "QString::null" _uic_CONTENTS "${_uic_CONTENTS}" )
  STRING(REGEX REPLACE "tr2i18n\\(\"\", \"\"\\)" "QString::null" _uic_CONTENTS "${_uic_CONTENTS}" )
  #replace image15_data with img15_filename to make enable_final work
  STRING(REGEX REPLACE "image([0-9]+)_data" "img\\1_${KDE_UIC_BASENAME}" _uic_CONTENTS "${_uic_CONTENTS}")
  #fixup include guard
  STRING(REGEX REPLACE "#ifndef " "#ifndef UI_" _uic_CONTENTS "${_uic_CONTENTS}")
  STRING(REGEX REPLACE "#define " "#define UI_" _uic_CONTENTS "${_uic_CONTENTS}")

  FILE(WRITE ${KDE_UIC_CPP_FILE} "#include <klocalizedstring.h>\n\n${_uic_CONTENTS}\n")
ENDIF()

