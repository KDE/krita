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

    CONNECT_TO_IMAGE(sigImageModified());
    CONNECT_TO_IMAGE(sigSizeChanged(qint32, qint32));
    CONNECT_TO_IMAGE(sigProfileChanged(const KoColorProfile*));
    CONNECT_TO_IMAGE(sigColorSpaceChanged(const KoColorSpace*));
    CONNECT_TO_IMAGE(sigResolutionChanged(double, double));

    CONNECT_TO_IMAGE(sigNodeChanged(KisNodeSP));
    CONNECT_TO_IMAGE(sigNodeAddedAsync(KisNodeSP));
    CONNECT_TO_IMAGE(sigRemoveNodeAsync(KisNodeSP));
    CONNECT_TO_IMAGE(sigLayersChangedAsync());
}

KisImageSignalRouter::~KisImageSignalRouter()
{
}

void KisImageSignalRouter::emitNotifications(KisImageSignalVector notifications)
{
    foreach(const KisImageSignalType &type, notifications) {
        emitNotification(type);
    }
}

void KisImageSignalRouter::emitNotification(KisImageSignalType type)
{
    emit sigNotification(type);
}

void KisImageSignalRouter::emitNodeChanged(KisNodeSP node)
{
    emit sigNodeChanged(node);
}

void KisImageSignalRouter::emitNodeHasBeenAdded(KisNode *parent, int index)
{
    emit sigNodeAddedAsync(parent->at(index));
}

void KisImageSignalRouter::emitAboutToRemoveANode(KisNode *parent, int index)
{
    emit sigRemoveNodeAsync(parent->at(index));
}


void KisImageSignalRouter::slotNotification(KisImageSignalType type)
{
    switch(type) {
    case LayersChangedSignal:
        emit sigLayersChangedAsync();
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
