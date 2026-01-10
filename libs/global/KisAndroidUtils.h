/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISANDROIDUTILS_H_
#define __KISANDROIDUTILS_H_

#include <kritaglobal_export.h>

namespace KisAndroidUtils
{

// Check whether we seem to be running on a Xiaomi device, which requires
// enabling several workarounds by default. If we need additional workarounds
// in the future, change this to return an enum or a flag set instead, depending
// on what's actually needed.
KRITAGLOBAL_EXPORT bool looksLikeXiaomiDevice();

} // namespace KisAndroidUtils

#endif // __KISANDROIDUTILS_H_
