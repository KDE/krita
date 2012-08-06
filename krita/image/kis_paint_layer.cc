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
#include <klocale.h>

#include <KoIcon.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoCompositeOp.h>

#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_default_bounds.h"

struct KisPaintLayer::Private
{
public:
    KisPaintDeviceSP paintDevice;
    QBitArray        paintChannelFlags;
};

KisPaintLayer::KisPaintLayer(KisImageWSP image, const QString& name, quint8 opacity, KisPaintDeviceSP dev)
        : KisLayer(image, name, opacity)
        , m_d(new Private())
{

    Q_ASSERT(dev);
    m_d->paintDevice = dev;
    m_d->paintDevice->setParentNode(this);

    // fixme: overwriting the default bounds is unexpected behaviour.
    // maybe something like if(dynamic_cast<KisDefaultBounds*>(dev.defaultBounds())) {..} is better.
    m_d->paintDevice->setDefaultBounds(new KisDefaultBounds(image));
}


KisPaintLayer::KisPaintLayer(KisImageWSP image, const QString& name, quint8 opacity)
        : KisLayer(image, name, opacity)
        , m_d(new Private())
{
    Q_ASSERT(image);
    m_d->paintDevice = new KisPaintDevice(this, image->colorSpace(), new KisDefaultBounds(image));
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
    m_d->paintDevice = new KisPaintDevice(this, colorSpace, new KisDefaultBounds(image));
}

KisPaintLayer::KisPaintLayer(const KisPaintLayer& rhs)
        : KisLayer(rhs)
        , KisIndirectPaintingSupport()
        , m_d(new Private)
{
    m_d->paintChannelFlags = rhs.m_d->paintChannelFlags;
    m_d->paintDevice = new KisPaintDevice(*rhs.m_d->paintDevice.data());
    m_d->paintDevice->setParentNode(this);
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
    return hasTemporaryTarget();
}

void KisPaintLayer::copyOriginalToProjection(const KisPaintDeviceSP original,
        KisPaintDeviceSP projection,
        const QRect& rect) const
{
    lockTemporaryTarget();

    KisPainter gc(projection);
    gc.setCompositeOp(projection->colorSpace()->compositeOp(COMPOSITE_COPY));
    gc.bitBlt(rect.topLeft(), original, rect);

    if (hasTemporaryTarget()) {
        gc.setOpacity(temporaryOpacity());
        gc.setCompositeOp(temporaryCompositeOp());
        gc.setChannelFlags(temporaryChannelFlags());
        gc.bitBlt(rect.topLeft(), temporaryTarget(), rect);
    }

    unlockTemporaryTarget();
}

void KisPaintLayer::setDirty(const QRect & rect)
{
    KisIndirectPaintingSupport::setDirty(rect);
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

KoDocumentSectionModel::PropertyList KisPaintLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    
    // XXX: get right icons
    l << KoDocumentSectionModel::Property(i18n("Alpha Channel Locked"), koIcon("transparency-locked"), koIcon("transparency-unlocked"), alphaLocked());
    l << KoDocumentSectionModel::Property(i18n("Alpha Channel Disabled"), koIcon("transparency-disabled"), koIcon("transparency-enabled"), alphaChannelDisabled());
    
    return l;
}

void KisPaintLayer::setSectionModelProperties(const KoDocumentSectionModel::PropertyList &properties)
{
    foreach (KoDocumentSectionModel::Property property, properties) {
        if (property.name == i18n("Alpha Channel Locked")) {
            setAlphaLocked(property.state.toBool());
        }
        else if (property.name == i18n("Alpha Channel Disabled")) {
            disableAlphaChannel(property.state.toBool());
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
    return rect | KisLayer::extent();
}

QRect KisPaintLayer::exactBounds() const
{
    QRect rect = temporaryTarget() ? temporaryTarget()->exactBounds() : QRect();
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
}


#include "kis_paint_layer.moc"
