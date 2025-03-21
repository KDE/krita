/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISOPENGLCONTEXTSWITCHLOCK_H
#define KISOPENGLCONTEXTSWITCHLOCK_H

#include <kritaui_export.h>
#include <KisAdaptedLock.h>

class QOpenGLWidget;
class QOpenGLContext;
class QSurface;

class KRITAUI_EXPORT KisOpenGLContextSwitchLockAdapter
{
public:
    KisOpenGLContextSwitchLockAdapter(QOpenGLWidget *targetWidget);

    void lock();
    void unlock();

private:
    QOpenGLWidget *m_targetWidget {nullptr};
    QOpenGLContext *m_oldContext {nullptr};
    QSurface *m_oldSurface {nullptr};
};

class KRITAUI_EXPORT KisOpenGLContextSwitchLockAdapterSkipOnQt5 : public KisOpenGLContextSwitchLockAdapter
{
public:
    using KisOpenGLContextSwitchLockAdapter::KisOpenGLContextSwitchLockAdapter;

    void lock();
    void unlock();
};

KIS_DECLARE_ADAPTED_LOCK(KisOpenGLContextSwitchLock, KisOpenGLContextSwitchLockAdapter)
KIS_DECLARE_ADAPTED_LOCK(KisOpenGLContextSwitchLockSkipOnQt5, KisOpenGLContextSwitchLockAdapterSkipOnQt5)

#endif // KISOPENGLCONTEXTSWITCHLOCK_H
