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

/**
 * Get the RoamingAppData location. If the current process is a packaged app,
 * the redirected private app location is returned. Uses `SHGetKnownFolderPath`
 * therefore the path uses native path separators and does not include a
 * trailing backslash.
 *
 * See also: https://docs.microsoft.com/en-us/windows/msix/desktop/desktop-to-uwp-behind-the-scenes#appdata-operations-on-windows-10-version-1903-and-later
 */
KRITAGLOBAL_EXPORT QString getPackageRoamingAppDataLocation();

} /* namespace KisWindowsPackageUtils */

#endif /* KIS_WINDOWS_PACKAGE_UTILS_H */
