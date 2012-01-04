/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_image_signal_router.h"

#include <QThread>

#include "kis_image.h"


#define CONNECT_TO_IMAGE(signal)                                        \
    connect(this, SIGNAL(signal), m_image, SIGNAL(signal), Qt::DirectConnection)

#define EMIT_NONBLOCKING(signal)                \
    {                                           \
        emit signal;                            \
    }

#define EMIT_DIRECT_ASSERT_SAME_THREAD(signal)  \
    {                                           \
        Q_ASSERT(checkSameThread());            \
        emit signal;                            \
    }


struct ImageSignalsStaticRegistrar {
    ImageSignalsStaticRegistrar() {
        qRegisterMetaType<KisImageSignalType>("KisImageSignalType");
    }
};
static ImageSignalsStaticRegistrar __registrar;


KisImageSignalRouter::KisImageSignalRouter(KisImageWSP image)
    : m_image(image)
{
    connect(this, SIGNAL(sigNotification(KisImageSignalType)),
            SLOT(slotNotification(KisImageSignalType)));

    CONNECT_TO_IMAGE(sigLayersChanged(KisGroupLayerSP));
    CONNECT_TO_IMAGE(sigPostLayersChanged(KisGroupLayerSP));
    CONNECT_TO_IMAGE(sigImageModified());
    CONNECT_TO_IMAGE(sigSizeChanged(qint32, qint32));
    CONNECT_TO_IMAGE(sigProfileChanged(const KoColorProfile*));
    CONNECT_TO_IMAGE(sigColorSpaceChanged(const KoColorSpace*));
    CONNECT_TO_IMAGE(sigResolutionChanged(double, double));

    CONNECT_TO_IMAGE(sigNodeChanged(KisNode*));
    CONNECT_TO_IMAGE(sigAboutToAddANode(KisNode*, int));
    CONNECT_TO_IMAGE(sigNodeHasBeenAdded(KisNode*, int));
    CONNECT_TO_IMAGE(sigAboutToRemoveANode(KisNode*, int));
    CONNECT_TO_IMAGE(sigNodeHasBeenRemoved(KisNode*, int));
    CONNECT_TO_IMAGE(sigAboutToMoveNode(KisNode*, int, int));
    CONNECT_TO_IMAGE(sigNodeHasBeenMoved(KisNode*, int, int));
}

KisImageSignalRouter::~KisImageSignalRouter()
{
}

bool KisImageSignalRouter::checkSameThread()
{
    return QThread::currentThread() == m_image->thread();
}

void KisImageSignalRouter::emitNotifications(KisImageSignalVector notifications)
{
    foreach(const KisImageSignalType &type, notifications) {
        emitNotification(type);
    }
}

void KisImageSignalRouter::emitNotification(KisImageSignalType type)
{
    EMIT_NONBLOCKING(sigNotification(type));
}

void KisImageSignalRouter::emitNodeChanged(KisNode *node)
{
    EMIT_NONBLOCKING(sigNodeChanged(node));
}

void KisImageSignalRouter::emitAboutToAddANode(KisNode *parent, int index)
{
    /**
     * Some of the users of our signals rely on the fact that the
     * signals are emitted synchronously from the same thread. Such
     * users are KisNodeModel, KisShapeController. They request the
     * data of the signal right from the node data, so they cannot
     * be emitted asynchronously. We cannot use BlockingQueued
     * connections here, because the we'll get a deadlock when UI
     * will decide to wait for scheduler to finish it's job.
     *
     * That is why we explicitly check that no nodes are added,
     * removed or moved from the context of the scheduler thread.
     * Currently we have no other way than to assert in such a case.
     * So all the node modifications should be done using legacy
     * undo adapter, in the context of the UI thread.
     */

    EMIT_DIRECT_ASSERT_SAME_THREAD(sigAboutToAddANode(parent, index));
}

void KisImageSignalRouter::emitNodeHasBeenAdded(KisNode *parent, int index)
{
    // see comment in emitAboutToAddANode()
    EMIT_DIRECT_ASSERT_SAME_THREAD(sigNodeHasBeenAdded(parent, index));
}

void KisImageSignalRouter::emitAboutToRemoveANode(KisNode *parent, int index)
{
    // see comment in emitAboutToAddANode()
    EMIT_DIRECT_ASSERT_SAME_THREAD(sigAboutToRemoveANode(parent, index));
}

void KisImageSignalRouter::emitNodeHasBeenRemoved(KisNode *parent, int index)
{
    // see comment in emitAboutToAddANode()
    EMIT_DIRECT_ASSERT_SAME_THREAD(sigNodeHasBeenRemoved(parent, index));
}

void KisImageSignalRouter::emitAboutToMoveNode(KisNode *parent, int oldIndex, int newIndex)
{
    // see comment in emitAboutToAddANode()
    EMIT_DIRECT_ASSERT_SAME_THREAD(sigAboutToMoveNode(parent, oldIndex, newIndex));
}

void KisImageSignalRouter::emitNodeHasBeenMoved(KisNode *parent, int oldIndex, int newIndex)
{
    // see comment in emitAboutToAddANode()
    EMIT_DIRECT_ASSERT_SAME_THREAD(sigNodeHasBeenMoved(parent, oldIndex, newIndex));
}


void KisImageSignalRouter::slotNotification(KisImageSignalType type)
{
    switch(type) {
    case LayersChangedSignal:
        emit sigLayersChanged(m_image->rootLayer());
        emit sigPostLayersChanged(m_image->rootLayer());
        break;
    case ModifiedSignal:
        emit sigImageModified();
        break;
    case SizeChangedSignal:
        emit sigSizeChanged(m_image->width(), m_image->height());
        break;
    case ProfileChangedSignal:
        emit sigProfileChanged(m_image->profile());
        break;
    case ColorSpaceChangedSignal:
        emit sigColorSpaceChanged(m_image->colorSpace());
        break;
    case ResolutionChangedSignal:
        emit sigResolutionChanged(m_image->xRes(), m_image->yRes());
        break;
    }
}
