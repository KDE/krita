/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISANDROIDEXITINFO_H_
#define __KISANDROIDEXITINFO_H_

#include <kritaglobal_export.h>

#include <QString>

class KRITAGLOBAL_EXPORT KisAndroidExitInfo
{
public:
    // REASON_* constants from ApplicationExitInfo.
    enum class Reason {
        Unknown = 0,
        ExitSelf = 1,
        Signaled = 2,
        LowMemory = 3,
        Crash = 4,
        CrashNative = 5,
        Anr = 6,
        InitializationFailure = 7,
        PermissionChange = 8,
        ExcessiveResourceUsage = 9,
        UserRequested = 10,
        UserStopped = 11,
        DependencyDied = 12,
        Other = 13,
        Freezer = 14,
        PackageStateChange = 15,
        PackageUpdated = 16,
    };

    // IMPORTANCE_* constants from ActivityManager.RunningAppProcessInfo.
    enum class Importance {
        Foreground = 100,
        ForegroundService = 125,
        PerceptiblePre26 = 130,
        TopSleepingPre28 = 150,
        Visible = 200,
        Perceptible = 230,
        Service = 300,
        TopSleeping = 325,
        CantSaveState = 350,
        Cached = 400,
        Empty = 500,
        Gone = 1000,
    };

    static KisAndroidExitInfo getLast();

    bool isValid() const
    {
        return m_valid;
    }

    int reasonCode() const
    {
        return m_reasonCode;
    }

    int exitOrSignalCode() const
    {
        return m_exitOrSignalCode;
    }

    int importanceCode() const
    {
        return m_importanceCode;
    }

    const QString &description() const
    {
        return m_description;
    }

    QString buildLogString() const;

private:
    KisAndroidExitInfo();
    KisAndroidExitInfo(int reasonCode, int exitOrSignalCode, int importanceCode, const QString &description);

    const QString m_description;
    const int m_reasonCode;
    const int m_exitOrSignalCode;
    const int m_importanceCode;
    const bool m_valid;
};

#endif // __KISANDROIDEXITINFO_H_
