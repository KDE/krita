/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KRITAVERSIONWRAPPER_H
#define KRITAVERSIONWRAPPER_H

#include "kritaversion_export.h"
#include <QString>

namespace KritaVersionWrapper {

    KRITAVERSION_EXPORT QString versionString(bool checkGit = false);
}

#endif // KRITAVERSIONWRAPPER_H
