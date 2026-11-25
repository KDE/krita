/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISANDROIDUTILS_H_
#define __KISANDROIDUTILS_H_

#include <QString>
#include <kritaglobal_export.h>

class QTemporaryFile;

namespace KisAndroidUtils
{

// Installs the Android log handler and copies assets.
KRITAGLOBAL_EXPORT void performInitialSetup();

// Check whether we seem to be running on a Xiaomi device, which requires
// enabling several workarounds by default. If we need additional workarounds
// in the future, change this to return an enum or a flag set instead, depending
// on what's actually needed.
KRITAGLOBAL_EXPORT bool looksLikeXiaomiDevice();

// Check whether we seem to be running a device with broken stylus cursors that
// show up while hovering and then disappear when pressing them down, which is
// immensely annoying and needs to be worked around. We consider this to be the
// case for Wacom and XPPEN devices.
KRITAGLOBAL_EXPORT bool looksLikeBrokenCursorDevice();

// Checks whether the below setHideTouchAndStylusCursors is supported on this
// patched version of Qt.
KRITAGLOBAL_EXPORT bool canHideTouchAndStylusCursors();

// Hides cursors on touch and stylus events made with internal devices. If the
// method doesn't exist, such as when using an outdated patched Qt, this will
// do nothing. Use the above canHideTouchAndStylusCursors to check in advance.
KRITAGLOBAL_EXPORT void setHideTouchAndStylusCursors(bool hide);

// Check whether the device supports reporting low-memory situations as an exit
// reason. Devices that don't will instead report a SIGKILL.
KRITAGLOBAL_EXPORT bool isLowMemoryKillReportSupported();

// Checks , logs and clears the pending exception in the Java Native Interface.
// If an exception occurs while one is pending, the application dies. Some Qt
// stuff, such as QDesktopServices, can throw JNI errors and don't clear them.
KRITAGLOBAL_EXPORT void clearJniException(const QString &location);

// Checks whether we are in immersive mode, which is basically full-screen.
KRITAGLOBAL_EXPORT bool isInFullScreen();

// Enters or exits immersive mode if we're not in that state already.
KRITAGLOBAL_EXPORT void setFullScreen(bool fullScreen);

// QFile::copy doesn't work on sandboxed directories, use this instead. The
// value placed in outErrorMessage is not translated, use it for logging or
// present it to the user as an internal error. On success, it will be cleared.
KRITAGLOBAL_EXPORT bool
copyFile(const QString &inputPath, const QString &outputPath, QString *outErrorMessage = nullptr);

KRITAGLOBAL_EXPORT bool
copyFileToTemporary(const QString &inputPath, QTemporaryFile &outputFile, QString *outErrorMessage = nullptr);

} // namespace KisAndroidUtils

#endif // __KISANDROIDUTILS_H_
