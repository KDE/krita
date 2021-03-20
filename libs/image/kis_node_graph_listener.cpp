/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_graph_listener.h"

#include "kis_time_span.h"
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

void KisNodeGraphListener::keyframeChannelHasBeenAdded(KisNode *node, KisKeyframeChannel *channel)
{

}

void KisNodeGraphListener::keyframeChannelAboutToBeRemoved(KisNode *node, KisKeyframeChannel *channel)
{

}

void KisNodeGraphListener::nodeChanged(KisNode * /*node*/)
{
}

void KisNodeGraphListener::nodeCollapsedChanged(KisNode * /*node*/)
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

void KisNodeGraphListener::invalidateFrames(const KisTimeSpan &range, const QRect &rect)
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
