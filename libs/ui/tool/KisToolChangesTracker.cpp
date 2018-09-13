/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisToolChangesTracker.h"

#include "kis_global.h"
#include <QSharedPointer>

struct KisToolChangesTracker::Private {
    QList<KisToolChangesTrackerDataSP> undoStack;
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
}

void KisToolChangesTracker::requestUndo()
{
    if (m_d->undoStack.size() > 1) {
        m_d->undoStack.removeLast();
        emit sigConfigChanged(m_d->undoStack.last());
    }
}

KisToolChangesTrackerDataSP KisToolChangesTracker::lastState() const
{
    return !m_d->undoStack.isEmpty() ? m_d->undoStack.last() : 0;
}

void KisToolChangesTracker::reset()
{
    m_d->undoStack.clear();
}

bool KisToolChangesTracker::isEmpty() const
{
    return m_d->undoStack.isEmpty();
}
