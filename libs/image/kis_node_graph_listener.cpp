/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_node_graph_listener.h"

#include "kis_time_range.h"
#include <QRect>
#include <QtGlobal>


struct Q_DECL_HIDDEN KisNodeGraphListener::Private
{
    Private() : sequenceNumber(0) {}
    int sequenceNumber;
};

KisNodeGraphListener::KisNodeGraphListener()
    : m_d(new Private())
{
}

KisNodeGraphListener::~KisNodeGraphListener()
{
}

void KisNodeGraphListener::aboutToAddANode(KisNode */*parent*/, int /*index*/)
{
    m_d->sequenceNumber++;
}

void KisNodeGraphListener::nodeHasBeenAdded(KisNode */*parent*/, int /*index*/)
{
    m_d->sequenceNumber++;
}

void KisNodeGraphListener::aboutToRemoveANode(KisNode */*parent*/, int /*index*/)
{
    m_d->sequenceNumber++;
}

void KisNodeGraphListener::nodeHasBeenRemoved(KisNode */*parent*/, int /*index*/)
{
    m_d->sequenceNumber++;
}

void KisNodeGraphListener::aboutToMoveNode(KisNode * /*node*/, int /*oldIndex*/, int /*newIndex*/)
{
    m_d->sequenceNumber++;
}

void KisNodeGraphListener::nodeHasBeenMoved(KisNode * /*node*/, int /*oldIndex*/, int /*newIndex*/)
{
    m_d->sequenceNumber++;
}

int KisNodeGraphListener::graphSequenceNumber() const
{
    return m_d->sequenceNumber;
}

void KisNodeGraphListener::nodeChanged(KisNode * /*node*/)
{
}

void KisNodeGraphListener::invalidateAllFrames()
{
}

void KisNodeGraphListener::notifySelectionChanged()
{
}

void KisNodeGraphListener::requestProjectionUpdate(KisNode * /*node*/, const QVector<QRect> &/*rects*/, bool /*resetAnimationCache*/)
{
}

void KisNodeGraphListener::invalidateFrames(const KisFrameSet &range, const QRect &rect)
{
    Q_UNUSED(range);
    Q_UNUSED(rect);
}

void KisNodeGraphListener::requestTimeSwitch(int time)
{
    Q_UNUSED(time);
}

KisNode *KisNodeGraphListener::graphOverlayNode() const
{
    return 0;
}
