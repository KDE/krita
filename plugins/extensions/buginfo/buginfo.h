/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BUGINFO_H
#define BUGINFO_H

#include <QVariant>
#include <KisActionPlugin.h>
#include <config-use-surface-color-management-api.h>

class KUndo2MagicString;

class BugInfo : public KisActionPlugin
{
    Q_OBJECT
public:
    BugInfo(QObject *parent, const QVariantList &);
    ~BugInfo() override;

public Q_SLOTS:

    void slotKritaLog();
    void slotSysInfo();

#ifdef Q_OS_ANDROID
    void slotDumpLogcat();
#endif

#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
    void slotCrashLog();
#endif

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    void slotColorManagement();
#endif
};

#endif // BUGINFO_H
