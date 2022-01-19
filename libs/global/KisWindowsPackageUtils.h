/*
 * SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_WINDOWS_PACKAGE_UTILS_H
#define KIS_WINDOWS_PACKAGE_UTILS_H

#include "kritaglobal_export.h"


class QString;

namespace KisWindowsPackageUtils
{

KRITAGLOBAL_EXPORT bool isRunningInPackage();
KRITAGLOBAL_EXPORT bool tryGetCurrentPackageFamilyName(QString *outName = nullptr);
KRITAGLOBAL_EXPORT bool tryGetCurrentPackageFullName(QString *outName = nullptr);

} /* namespace KisWindowsPackageUtils */

#endif /* KIS_WINDOWS_PACKAGE_UTILS_H */
