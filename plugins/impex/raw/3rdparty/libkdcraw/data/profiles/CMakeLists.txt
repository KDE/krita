#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

file(GLOB icmfiles *.icm *.icc)

install(FILES  ${icmfiles}
        DESTINATION ${DATA_INSTALL_DIR}/libkdcraw/profiles)
