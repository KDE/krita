/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCursorOverrideLock.h"
#include <QGuiApplication>

KisCursorOverrideLockAdapter::KisCursorOverrideLockAdapter(const QCursor &cursor)
    : m_cursor(cursor)
{
}

KisCursorOverrideLockAdapter::~KisCursorOverrideLockAdapter() = default;

void KisCursorOverrideLockAdapter::lock()
{
    qApp->setOverrideCursor(m_cursor);
}

void KisCursorOverrideLockAdapter::unlock()
{
    qApp->restoreOverrideCursor();
}
