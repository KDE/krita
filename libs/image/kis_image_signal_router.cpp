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

#define CONNECT_TO_IMAGE_QUEUED(signal)                                 \
    connect(this, SIGNAL(signal), m_image, SIGNAL(signal), Qt::QueuedConnection)


struct ImageSignalsStaticRegistrar {
    ImageSignalsStaticRegistrar() {
        qRegisterMetaType<KisImageSignalType>("KisImageSignalType");
    }
};
static ImageSignalsStaticRegistrar __registrar;


KisImageSignalRouter::KisImageSignalRouter(KisImageWSP image)
    : QObject(image.data()),
      m_image(image)
{
    connect(this, SIGNAL(sigNotification(KisImageSignalType)),
            SLOT(slotNotification(KisImageSignalType)));

    CONNECT_TO_IMAGE(sigImageModified());
    CONNECT_TO_IMAGE(sigSizeChanged(const QPointF&, const QPointF&));
    CONNECT_TO_IMAGE(sigResolutionChanged(double, double));
    CONNECT_TO_IMAGE(sigRequestNodeReselection(KisNodeSP, const KisNodeList&));

    CONNECT_TO_IMAGE(sigNodeChanged(KisNodeSP));
    CONNECT_TO_IMAGE(sigNodeAddedAsync(KisNodeSP));
    CONNECT_TO_IMAGE(sigRemoveNodeAsync(KisNodeSP));
    CONNECT_TO_IMAGE(sigLayersChangedAsync());

    /**
     * Color space and profile conversion functions run without storkes,
     * therefore they are executed in GUI hread under the global lock held.
     *
     * To ensure that the receiver of the signal will not deadlock by
     * barrier-locking the image, we should make these signals queued.
     */

    CONNECT_TO_IMAGE_QUEUED(sigProfileChanged(const KoColorProfile*));
    CONNECT_TO_IMAGE_QUEUED(sigColorSpaceChanged(const KoColorSpace*));
}

KisImageSignalRouter::~KisImageSignalRouter()
{
}

void KisImageSignalRouter::emitNotifications(KisImageSignalVector notifications)
{
    Q_FOREACH (const KisImageSignalType &type, notifications) {
        emitNotification(type);
    }
}

void KisImageSignalRouter::emitNotification(KisImageSignalType type)
{
    /**
     * All the notifications except LayersChangedSignal should go in a
     * queued way. And LayersChangedSignal should be delivered to the
     * recipients in a non-reordered way
     */

    if (type.id == LayersChangedSignal) {
        slotNotification(type);
    } else {
        Q_EMIT sigNotification(type);
    }
}

void KisImageSignalRouter::emitNodeChanged(KisNodeSP node)
{
    Q_EMIT sigNodeChanged(node);
}

void KisImageSignalRouter::emitNodeHasBeenAdded(KisNode *parent, int index)
{
    KisNodeSP newNode = parent->at(index);

    // overlay selection masks reset frames themselves
    if (!newNode->inherits("KisSelectionMask")) {
        KisImageSP image = m_image.toStrongRef();
        if (image) {
            image->invalidateAllFrames();
        }
    }

    Q_EMIT sigNodeAddedAsync(newNode);
}

void KisImageSignalRouter::emitAboutToRemoveANode(KisNode *parent, int index)
{
    KisNodeSP removedNode = parent->at(index);

    // overlay selection masks reset frames themselves
    if (!removedNode->inherits("KisSelectionMask")) {
        KisImageSP image = m_image.toStrongRef();
        if (image) {
            image->invalidateAllFrames();
        }
    }

    Q_EMIT sigRemoveNodeAsync(removedNode);
}

void KisImageSignalRouter::emitRequestLodPlanesSyncBlocked(bool value)
{
    Q_EMIT sigRequestLodPlanesSyncBlocked(value);
}

void KisImageSignalRouter::emitNotifyBatchUpdateStarted()
{
    Q_EMIT sigNotifyBatchUpdateStarted();
}

void KisImageSignalRouter::emitNotifyBatchUpdateEnded()
{
    Q_EMIT sigNotifyBatchUpdateEnded();
}

void KisImageSignalRouter::slotNotification(KisImageSignalType type)
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }

    switch(type.id) {
    case LayersChangedSignal:
        image->invalidateAllFrames();
        Q_EMIT sigLayersChangedAsync();
        break;
    case ModifiedSignal:
        Q_EMIT sigImageModified();
        break;
    case SizeChangedSignal:
        image->invalidateAllFrames();
        Q_EMIT sigSizeChanged(type.sizeChangedSignal.oldStillPoint,
                            type.sizeChangedSignal.newStillPoint);
        break;
    case ProfileChangedSignal:
        image->invalidateAllFrames();
        Q_EMIT sigProfileChanged(image->profile());
        break;
    case ColorSpaceChangedSignal:
        image->invalidateAllFrames();
        Q_EMIT sigColorSpaceChanged(image->colorSpace());
        break;
    case ResolutionChangedSignal:
        image->invalidateAllFrames();
        Q_EMIT sigResolutionChanged(image->xRes(), image->yRes());
        break;
    case NodeReselectionRequestSignal:
        if (type.nodeReselectionSignal.newActiveNode ||
            !type.nodeReselectionSignal.newSelectedNodes.isEmpty()) {

            Q_EMIT sigRequestNodeReselection(type.nodeReselectionSignal.newActiveNode,
                                           type.nodeReselectionSignal.newSelectedNodes);
        }
        break;
    }
}
