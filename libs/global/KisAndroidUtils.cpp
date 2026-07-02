/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisAndroidUtils.h"
#include "KisAndroidLogHandler.h"
#include <kis_debug.h>

#include <QFile>

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

bool copyFile(const QString &inputPath, const QString &outputPath, QString *outErrorMessage)
{
    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        if (outErrorMessage) {
            *outErrorMessage =
                QStringLiteral("failed to open input file '%1': %2").arg(inputPath).arg(inputFile.errorString());
        }
        return false;
    }

    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (outErrorMessage) {
            *outErrorMessage =
                QStringLiteral("failed to open output file '%1': %2").arg(outputPath).arg(outputFile.errorString());
        }
        return false;
    }

    QByteArray buffer;
    buffer.resize(BUFSIZ);
    while (true) {
        qint64 read = inputFile.read(buffer.data(), BUFSIZ);
        if (read < 0) {
            if (outErrorMessage) {
                *outErrorMessage = QStringLiteral("failed to read from input file '%1': %2")
                                       .arg(inputPath)
                                       .arg(inputFile.errorString());
            }
            return false;
        } else if (read > 0) {
            qint64 written = outputFile.write(buffer, read);
            if (written < 0) {
                if (outErrorMessage) {
                    *outErrorMessage = QStringLiteral("failed to write %1 byte(s) to output file '%2': %3")
                                           .arg(read)
                                           .arg(outputPath)
                                           .arg(outputFile.errorString());
                }
                return false;
            } else if (written != read) {
                if (outErrorMessage) {
                    *outErrorMessage =
                        QStringLiteral("tried to write %1 byte(s) to output file '%2', but only wrote %3")
                            .arg(read)
                            .arg(outputPath)
                            .arg(written);
                }
                return false;
            }
        } else {
            if (outputFile.flush()) {
                if (outErrorMessage) {
                    outErrorMessage->clear();
                }
                return true;
            } else {
                if (outErrorMessage) {
                    *outErrorMessage = QStringLiteral("failed to flush output file '%1': %2")
                                           .arg(outputPath)
                                           .arg(outputFile.errorString());
                }
                return false;
            }
        }
    }
}

} // namespace KisAndroidUtils
