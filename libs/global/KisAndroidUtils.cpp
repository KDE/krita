/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisAndroidUtils.h"
#include "KisAndroidLogHandler.h"
#include <kis_debug.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QJniEnvironment>
#include <QJniObject>
#else
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
using QJniEnvironment = QAndroidJniEnvironment;
using QJniObject = QAndroidJniObject;
#endif

namespace KisAndroidUtils
{

void performInitialSetup()
{
    KisAndroidLogHandler::handler_init();

    QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
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
        result =
            QJniObject::callStaticMethod<jboolean>("org/krita/android/MainActivity", "looksLikeXiaomiDevice", "()Z");
    }
    return result;
}

bool isLowMemoryKillReportSupported()
{
    // The support is device-bound and will never change, so cache the JNI call.
    static bool checked;
    static bool result;
    if (!checked) {
        checked = true;
        result = QJniObject::callStaticMethod<jboolean>("org/krita/android/MainActivity",
                                                        "isLowMemoryKillReportSupported",
                                                        "()Z");
    }
    return result;
}

void clearJniException(const QString &location)
{
    QJniEnvironment env;
    if (env->ExceptionCheck()) {
        warnKrita << "JNI exception occurred" << location;
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

bool isInFullScreen()
{
    QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                             "activity",
                                                             "()Landroid/app/Activity;");
    KisAndroidUtils::clearJniException(QStringLiteral("getting activity in isInFullScreen"));
    if (activity.isValid()) {
        bool fullScreen = activity.callMethod<jboolean>("isInFullScreen", "()Z");
        KisAndroidUtils::clearJniException(QStringLiteral("calling isInFullScreen"));
        return fullScreen;
    } else {
        qWarning("isInFullScreen: activity not valid");
        return false;
    }
}

void setFullScreen(bool fullScreen)
{
    QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                             "activity",
                                                             "()Landroid/app/Activity;");
    KisAndroidUtils::clearJniException(QStringLiteral("getting activity in setFullScreen"));
    if (activity.isValid()) {
        activity.callMethod<void>("setFullScreenOnUiThread", "(Z)V", jboolean(fullScreen));
        KisAndroidUtils::clearJniException(QStringLiteral("calling setFullScreenOnUiThread"));
    } else {
        qWarning("setFullScreen: activity not valid");
    }
}

} // namespace KisAndroidUtils
