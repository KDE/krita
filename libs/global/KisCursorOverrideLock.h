/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCURSOROVERRIDELOCK_H
#define KISCURSOROVERRIDELOCK_H

#include <kritaglobal_export.h>
#include <QCursor>
#include <KisAdaptedLock.h>


class KRITAGLOBAL_EXPORT KisCursorOverrideLockAdapter
{
public:
    KisCursorOverrideLockAdapter(const QCursor &cursor);
    ~KisCursorOverrideLockAdapter();

    void lock();
    void unlock();

private:
    QCursor m_cursor;
};

KIS_DECLARE_ADAPTED_LOCK(KisCursorOverrideLock, KisCursorOverrideLockAdapter)

#endif // KISCURSOROVERRIDELOCK_H
