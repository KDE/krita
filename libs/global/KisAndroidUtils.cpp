/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisAndroidUtils.h"
#include <QtAndroid>

namespace KisAndroidUtils
{

bool looksLikeXiaomiDevice()
{
    // The device isn't going to change, so let's cache the slow JNI call.
    static bool checked;
    static bool result;
    if (!checked) {
        checked = true;
        result = QAndroidJniObject::callStaticMethod<jboolean>("org/krita/android/MainActivity",
                                                               "looksLikeXiaomiDevice",
                                                               "()Z");
    }
    return result;
}

} // namespace KisAndroidUtils
