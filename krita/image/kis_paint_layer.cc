/*
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2006 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_paint_layer.h"

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <KoIcon.h>
#include <kis_icon.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoCompositeOpRegistry.h>
#include <KoProperties.h>

#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_default_bounds.h"

#include "kis_onion_skin_compositor.h"
#include "kis_raster_keyframe_channel.h"

#include "kis_signal_auto_connection.h"
#include "kis_layer_properties_icons.h"

struct Q_DECL_HIDDEN KisPaintLayer::Private
{
public:
    Private() : contentChannel(0) {}

    KisPaintDeviceSP paintDevice;
    QBitArray        paintChannelFlags;

    // the real pointer is owned by the paint device
    KisRasterKeyframeChannel *contentChannel;

    KisSignalAutoConnectionsStore onionSkinConnection;
};

KisPaintLayer::KisPaintLayer(KisImageWSP image, const QString& name, quint8 opacity, KisPaintDeviceSP dev)
        : KisLayer(image, name, opacity)
        , m_d(new Private())
{
    Q_ASSERT(dev);

    init(dev);
    m_d->paintDevice->setDefaultBounds(new KisDefaultBounds(image));
}


KisPaintLayer::KisPaintLayer(KisImageWSP image, const QString& name, quint8 opacity)
        : KisLayer(image, name, opacity)
        , m_d(new Private())
{
    Q_ASSERT(image);

    init(new KisPaintDevice(this, image->colorSpace(), new KisDefaultBounds(image)));
}

KisPaintLayer::KisPaintLayer(KisImageWSP image, const QString& name, quint8 opacity, const KoColorSpace * colorSpace)
        : KisLayer(image, name, opacity)
        , m_d(new Private())
{
    if (!colorSpace) {
        Q_ASSERT(image);
        colorSpace = image->colorSpace();
    }
    Q_ASSERT(colorSpace);
    init(new KisPaintDevice(this, colorSpace, new KisDefaultBounds(image)));
}

KisPaintLayer::KisPaintLayer(const KisPaintLayer& rhs)
        : KisLayer(rhs)
        , KisIndirectPaintingSupport()
        , m_d(new Private)
{
    init(new KisPaintDevice(*rhs.m_d->paintDevice.data()), rhs.m_d->paintChannelFlags);
}

void KisPaintLayer::init(KisPaintDeviceSP paintDevice, const QBitArray &paintChannelFlags)
{
    m_d->paintDevice = paintDevice;
    m_d->paintDevice->setParentNode(this);

    m_d->paintChannelFlags = paintChannelFlags;
}

KisPaintLayer::~KisPaintLayer()
{
    delete m_d;
}

bool KisPaintLayer::allowAsChild(KisNodeSP node) const
{
    return node->inherits("KisMask");
}

KisPaintDeviceSP KisPaintLayer::original() const
{
    return m_d->paintDevice;
}

KisPaintDeviceSP KisPaintLayer::paintDevice() const
{
    return m_d->paintDevice;
}

bool KisPaintLayer::needProjection() const
{
    return hasTemporaryTarget() || (isAnimated() && onionSkinEnabled());
}

void KisPaintLayer::copyOriginalToProjection(const KisPaintDeviceSP original,
        KisPaintDeviceSP projection,
        const QRect& rect) const
{
    lockTemporaryTarget();

    KisPainter::copyAreaOptimized(rect.topLeft(), original, projection, rect);

    if (hasTemporaryTarget()) {
        KisPainter gc(projection);
        setupTemporaryPainter(&gc);
        gc.bitBlt(rect.topLeft(), temporaryTarget(), rect);
    }

    if (m_d->contentChannel &&
        m_d->contentChannel->keyframeCount() > 1 &&
        onionSkinEnabled() &&
        !m_d->paintDevice->defaultBounds()->externalFrameActive()) {

        KisOnionSkinCompositor *compositor = KisOnionSkinCompositor::instance();
        compositor->composite(m_d->paintDevice, projection, rect);
    }

    unlockTemporaryTarget();
}

void KisPaintLayer::setDirty(const QRect & rect)
{
    KisLayer::setDirty(rect);
}

QIcon KisPaintLayer::icon() const
{
    return QIcon();
}

void KisPaintLayer::setImage(KisImageWSP image)
{
    m_d->paintDevice->setDefaultBounds(new KisDefaultBounds(image));
    KisLayer::setImage(image);
}

KisNodeModel::PropertyList KisPaintLayer::sectionModelProperties() const
{
    KisNodeModel::PropertyList l = KisLayer::sectionModelProperties();

    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::alphaLocked, alphaLocked());

    if (isAnimated()) {
        l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::onionSkins, onionSkinEnabled());
    }

    return l;
}

void KisPaintLayer::setSectionModelProperties(const KisNodeModel::PropertyList &properties)
{
    foreach (const KisNodeModel::Property &property, properties) {
        if (property.name == i18n("Alpha Locked")) {
            setAlphaLocked(property.state.toBool());
        }
        else if (property.name == i18n("Onion skin")) {
            setOnionSkinEnabled(property.state.toBool());
        }
    }

    KisLayer::setSectionModelProperties(properties);
}

const KoColorSpace * KisPaintLayer::colorSpace() const
{
    return m_d->paintDevice->colorSpace();
}

bool KisPaintLayer::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

void KisPaintLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

void KisPaintLayer::setChannelLockFlags(const QBitArray& channelFlags)
{
    Q_ASSERT(((quint32)channelFlags.count() == colorSpace()->channelCount() || channelFlags.isEmpty()));
    m_d->paintChannelFlags = channelFlags;
}

const QBitArray& KisPaintLayer::channelLockFlags() const
{
    return m_d->paintChannelFlags;
}

QRect KisPaintLayer::extent() const
{
    QRect rect = temporaryTarget() ? temporaryTarget()->extent() : QRect();
    if (onionSkinEnabled()) rect |= KisOnionSkinCompositor::instance()->calculateExtent(m_d->paintDevice);
    return rect | KisLayer::extent();
}

QRect KisPaintLayer::exactBounds() const
{
    QRect rect = temporaryTarget() ? temporaryTarget()->exactBounds() : QRect();
    if (onionSkinEnabled()) rect |= KisOnionSkinCompositor::instance()->calculateExtent(m_d->paintDevice);
    return rect | KisLayer::exactBounds();
}

bool KisPaintLayer::alphaLocked() const
{
    QBitArray flags = colorSpace()->channelFlags(false, true) & m_d->paintChannelFlags;
    return flags.count(true) == 0 && !m_d->paintChannelFlags.isEmpty();
}

void KisPaintLayer::setAlphaLocked(bool lock)
{
    if(m_d->paintChannelFlags.isEmpty())
        m_d->paintChannelFlags = colorSpace()->channelFlags(true, true);
    
    if(lock)
        m_d->paintChannelFlags &= colorSpace()->channelFlags(true, false);
    else
        m_d->paintChannelFlags |= colorSpace()->channelFlags(false, true);

    baseNodeChangedCallback();
}

bool KisPaintLayer::onionSkinEnabled() const
{
    return nodeProperties().boolProperty("onionskin", false);
}

void KisPaintLayer::setOnionSkinEnabled(bool state)
{
    int oldState = onionSkinEnabled();
    if (oldState == state) return;

    if (state == false && oldState) {
        // FIXME: change ordering! race condition possible!

        // Turning off onionskins shrinks our extent. Let's clean up the onion skins first
        setDirty(KisOnionSkinCompositor::instance()->calculateExtent(m_d->paintDevice));
    }

    if (state) {
        m_d->onionSkinConnection.addConnection(KisOnionSkinCompositor::instance(),
                                               SIGNAL(sigOnionSkinChanged()),
                                               this,
                                               SLOT(slotExternalUpdateOnionSkins()));
    } else {
        m_d->onionSkinConnection.clear();
    }

    nodeProperties().setProperty("onionskin", state);
    m_d->contentChannel->setOnionSkinsEnabled(state);

    baseNodeChangedCallback();
}

void KisPaintLayer::slotExternalUpdateOnionSkins()
{
    if (!onionSkinEnabled()) return;

    const QRect dirtyRect =
        KisOnionSkinCompositor::instance()->calculateFullExtent(m_d->paintDevice);

    setDirty(dirtyRect);
}

void KisPaintLayer::enableAnimation()
{
    m_d->contentChannel = m_d->paintDevice->createKeyframeChannel(KisKeyframeChannel::Content, this);
    addKeyframeChannel(m_d->contentChannel);

    KisLayer::enableAnimation();
}
