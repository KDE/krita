/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisToolChangesTracker.h"

#include "kis_global.h"
#include <QSharedPointer>

struct KisToolChangesTracker::Private {
    QList<KisToolChangesTrackerDataSP> undoStack;
    QList<KisToolChangesTrackerDataSP> redoStack;
};


KisToolChangesTracker::KisToolChangesTracker()
    : m_d(new Private)
{
}

KisToolChangesTracker::~KisToolChangesTracker()
{
}

void KisToolChangesTracker::commitConfig(KisToolChangesTrackerDataSP state)
{
    m_d->undoStack.append(state);
    m_d->redoStack.clear();
}

void KisToolChangesTracker::requestUndo()
{
    if (m_d->undoStack.isEmpty()) return;

    m_d->redoStack.append(m_d->undoStack.last());
    m_d->undoStack.removeLast();
    if (!m_d->undoStack.isEmpty()) {
        emit sigConfigChanged(m_d->undoStack.last());
    }
}

void KisToolChangesTracker::requestRedo()
{
    if (m_d->redoStack.isEmpty()) return;

    m_d->undoStack.append(m_d->redoStack.last());
    m_d->redoStack.removeLast();
    if (!m_d->undoStack.isEmpty()) {
        emit sigConfigChanged(m_d->undoStack.last());
    }
}

KisToolChangesTrackerDataSP KisToolChangesTracker::lastState() const
{
    return !m_d->undoStack.isEmpty() ? m_d->undoStack.last() : static_cast<QSharedPointer<KisToolChangesTrackerData>>(0);
}

void KisToolChangesTracker::reset()
{
    m_d->undoStack.clear();
    m_d->redoStack.clear();
}

bool KisToolChangesTracker::isEmpty(bool undo) const
{
    if (undo) {
        return m_d->undoStack.isEmpty();
    } else {
        return m_d->redoStack.isEmpty();
    }
}
