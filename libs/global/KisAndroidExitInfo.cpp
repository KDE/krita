/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisAndroidUtils.h"
#include "KisAndroidExitInfo.h"
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtAndroid>

KisAndroidExitInfo KisAndroidExitInfo::getLast()
{
    QAndroidJniEnvironment env;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity",
                                                                           "()Landroid/app/Activity;");
    if (env->ExceptionCheck()) {
        qWarning("KisAndroidExitInfo::getLast: JNI exception in activity");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return KisAndroidExitInfo();
    } else if (!activity.isValid()) {
        qWarning("KisAndroidExitInfo::getLast: activity not valid");
        return KisAndroidExitInfo();
    }

    QAndroidJniObject exitInfo =
        activity.callObjectMethod("getLastApplicationExitInfo", "()Landroid/app/ApplicationExitInfo;");
    if (env->ExceptionCheck()) {
        qWarning("KisAndroidExitInfo::getLast: JNI exception in getLastApplicationExitInfo");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return KisAndroidExitInfo();
    } else if (!exitInfo.isValid()) {
        qWarning("KisAndroidExitInfo::getLast: exit info not valid");
        return KisAndroidExitInfo();
    }

    int reasonCode = exitInfo.callMethod<jint>("getReason", "()I");
    if (env->ExceptionCheck()) {
        qWarning("KisAndroidExitInfo::getLast: JNI exception in getReason");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return KisAndroidExitInfo();
    }

    int exitOrSignalCode = exitInfo.callMethod<jint>("getStatus", "()I");
    if (env->ExceptionCheck()) {
        qWarning("KisAndroidExitInfo::getLast: JNI exception in getStatus");
        env->ExceptionDescribe();
        env->ExceptionClear();
        exitOrSignalCode = -1;
    }

    int importanceCode = exitInfo.callMethod<jint>("getImportance", "()I");
    if (env->ExceptionCheck()) {
        qWarning("KisAndroidExitInfo::getLast: JNI exception in getImportance");
        env->ExceptionDescribe();
        env->ExceptionClear();
        importanceCode = -1;
    }

    QString description;
    {
        QAndroidJniObject descriptionObject = exitInfo.callObjectMethod("getDescription", "()Ljava/lang/String;");
        if (env->ExceptionCheck()) {
            qWarning("KisAndroidExitInfo::getLast: JNI exception in getDescription");
            env->ExceptionDescribe();
            env->ExceptionClear();
        } else if (descriptionObject.isValid()) {
            description = descriptionObject.toString();
        }
    }

    return KisAndroidExitInfo(reasonCode, exitOrSignalCode, importanceCode, description);
}

KisAndroidExitInfo::KisAndroidExitInfo()
    : m_reasonCode(int(Reason::Unknown))
    , m_exitOrSignalCode(-1)
    , m_importanceCode(-1)
    , m_valid(false)
{
}

KisAndroidExitInfo::KisAndroidExitInfo(int reasonCode,
                                       int exitOrSignalCode,
                                       int importanceCode,
                                       const QString &description)
    : m_description(description)
    , m_reasonCode(reasonCode)
    , m_exitOrSignalCode(exitOrSignalCode)
    , m_importanceCode(importanceCode)
    , m_valid(true)
{
}

QString KisAndroidExitInfo::buildLogString() const
{
    if (!isValid()) {
        return QString();
    }

    QString message;

    message.append(QStringLiteral("reason %1").arg(m_reasonCode));
    switch (m_reasonCode) {
    case int(Reason::Unknown):
        message.append(QStringLiteral(" (UNKNOWN)"));
        break;
    case int(Reason::ExitSelf):
        message.append(QStringLiteral(" (EXIT_SELF)"));
        break;
    case int(Reason::Signaled):
        message.append(QStringLiteral(" (SIGNALED)"));
        break;
    case int(Reason::LowMemory):
        message.append(QStringLiteral(" (LOW_MEMORY)"));
        break;
    case int(Reason::Crash):
        message.append(QStringLiteral(" (CRASH)"));
        break;
    case int(Reason::CrashNative):
        message.append(QStringLiteral(" (CRASH_NATIVE)"));
        break;
    case int(Reason::Anr):
        message.append(QStringLiteral(" (ANR)"));
        break;
    case int(Reason::InitializationFailure):
        message.append(QStringLiteral(" (INITIALIZATION_FAILURE)"));
        break;
    case int(Reason::PermissionChange):
        message.append(QStringLiteral(" (PERMISSION_CHANGE)"));
        break;
    case int(Reason::ExcessiveResourceUsage):
        message.append(QStringLiteral(" (EXCESSIVE_RESOURCE_USAGE)"));
        break;
    case int(Reason::UserRequested):
        message.append(QStringLiteral(" (USER_REQUESTED)"));
        break;
    case int(Reason::UserStopped):
        message.append(QStringLiteral(" (USER_STOPPED)"));
        break;
    case int(Reason::DependencyDied):
        message.append(QStringLiteral(" (DEPENDENCY_DIED)"));
        break;
    case int(Reason::Other):
        message.append(QStringLiteral(" (OTHER)"));
        break;
    case int(Reason::Freezer):
        message.append(QStringLiteral(" (FREEZER)"));
        break;
    case int(Reason::PackageStateChange):
        message.append(QStringLiteral(" (PACKAGE_STATE_CHANGE)"));
        break;
    case int(Reason::PackageUpdated):
        message.append(QStringLiteral(" (PACKAGE_UPDATED)"));
        break;
    default:
        break;
    }

    message.append(QStringLiteral(", importance %1").arg(m_importanceCode));
    switch (m_importanceCode) {
    case int(Importance::Foreground):
        message.append(QStringLiteral(" (FOREGROUND)"));
        break;
    case int(Importance::ForegroundService):
        message.append(QStringLiteral(" (FOREGROUND_SERVICE)"));
        break;
    case int(Importance::PerceptiblePre26):
        message.append(QStringLiteral(" (PERCEPTIBLE_PRE_26)"));
        break;
    case int(Importance::TopSleepingPre28):
        message.append(QStringLiteral(" (TOP_SLEEPING_PRE_28)"));
        break;
    case int(Importance::Visible):
        message.append(QStringLiteral(" (VISIBLE)"));
        break;
    case int(Importance::Perceptible):
        message.append(QStringLiteral(" (PERCEPTIBLE)"));
        break;
    case int(Importance::Service):
        message.append(QStringLiteral(" (SERVICE)"));
        break;
    case int(Importance::TopSleeping):
        message.append(QStringLiteral(" (TOP_SLEEPING)"));
        break;
    case int(Importance::CantSaveState):
        message.append(QStringLiteral(" (CANT_SAVE_STATE)"));
        break;
    case int(Importance::Cached):
        message.append(QStringLiteral(" (CACHED)"));
        break;
    case int(Importance::Empty):
        message.append(QStringLiteral(" (EMPTY)"));
        break;
    case int(Importance::Gone):
        message.append(QStringLiteral(" (GONE)"));
        break;
    default:
        break;
    }

    message.append(QStringLiteral(", exit code/signal %1").arg(m_exitOrSignalCode));

    message.append(QStringLiteral(", low-memory kill report "));
    if (KisAndroidUtils::isLowMemoryKillReportSupported()) {
        message.append(QStringLiteral("supported"));
    } else {
        message.append(QStringLiteral("not supported"));
    }

    if (m_description.isEmpty()) {
        message.append(QStringLiteral(", no description"));
    } else {
        message.append(QStringLiteral(", description '%1'").arg(m_description));
    }

    return message;
}
