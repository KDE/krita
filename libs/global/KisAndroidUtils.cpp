/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisAndroidUtils.h"
#include "KisAndroidLogHandler.h"
#include <QtAndroid>

namespace KisAndroidUtils
{

void performInitialSetup()
{
    KisAndroidLogHandler::handler_init();

    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity",
                                                                           "()Landroid/app/Activity;");
    if (activity.isValid()) {
        activity.callMethod<void>("copyAssets", "()V");
    } else {
        qWarning("performInitialSetup: activity not valid");
    }
}

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
