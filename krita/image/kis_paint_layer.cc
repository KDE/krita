/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoCompositeOp.h>

#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_node_visitor.h"

class KisPaintLayer::Private
{
public:
    KisPaintDeviceSP paintDevice;
    bool alphaLocked;
};

KisPaintLayer::KisPaintLayer(KisImageWSP img, const QString& name, quint8 opacity, KisPaintDeviceSP dev)
        : KisLayer(img, name, opacity)
        , m_d(new Private())
{
    Q_ASSERT(img);
    Q_ASSERT(dev);
    m_d->paintDevice = dev;
    m_d->alphaLocked = false;

}


KisPaintLayer::KisPaintLayer(KisImageWSP img, const QString& name, quint8 opacity)
        : KisLayer(img, name, opacity)
        , m_d(new Private())
{
    Q_ASSERT(img);
    m_d->paintDevice = new KisPaintDevice(this, img->colorSpace());
    m_d->alphaLocked = false;
}

KisPaintLayer::KisPaintLayer(KisImageWSP img, const QString& name, quint8 opacity, const KoColorSpace * colorSpace)
        : KisLayer(img, name, opacity)
        , m_d(new Private())
{
    if (!colorSpace) {
        Q_ASSERT(img);
        colorSpace = img->colorSpace();
    }
    Q_ASSERT(colorSpace);
    m_d->paintDevice = new KisPaintDevice(this, colorSpace);
    m_d->alphaLocked = false;
}

KisPaintLayer::KisPaintLayer(const KisPaintLayer& rhs)
        : KisLayer(rhs)
        , KisIndirectPaintingSupport(rhs)
        , m_d(new Private)
{
    m_d->alphaLocked = rhs.m_d->alphaLocked;
    m_d->paintDevice = new KisPaintDevice(*rhs.m_d->paintDevice.data());
    // FIXME: check this. repeat in other constructors?
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

QRect KisPaintLayer::repaintOriginal(KisPaintDeviceSP original,
                                     const QRect& rect)
{
    Q_UNUSED(original);
    return rect;
}

bool KisPaintLayer::needProjection() const
{
    return hasTemporaryTarget();
}

void KisPaintLayer::copyOriginalToProjection(const KisPaintDeviceSP original,
        KisPaintDeviceSP projection,
        const QRect& rect) const
{
    KisPainter gc(projection);
    gc.setCompositeOp(colorSpace()->compositeOp(COMPOSITE_COPY));
    gc.bitBlt(rect.topLeft(), original, rect);

    if (hasTemporaryTarget()) {
        gc.setOpacity(temporaryOpacity());
        gc.setCompositeOp(temporaryCompositeOp());
        gc.bitBlt(rect.topLeft(), temporaryTarget(), rect);
    }
}

QIcon KisPaintLayer::icon() const
{
    return QIcon();
}

KoDocumentSectionModel::PropertyList KisPaintLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    // XXX: get right icons
    l << KoDocumentSectionModel::Property(i18n("Alpha Locked"), KIcon("object-locked-unverified"), KIcon("object-locked-verified"), alphaLocked());
    return l;
}

void KisPaintLayer::setSectionModelProperties(const KoDocumentSectionModel::PropertyList &properties)
{
    foreach (KoDocumentSectionModel::Property property, properties) {
        if (property.name == i18n("Alpha Locked")) {
            setAlphaLocked(property.state.toBool());
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
    return m_d->alphaLocked;
}

void KisPaintLayer::setAlphaLocked(bool l)
{
    m_d->alphaLocked = l;
}


#include "kis_paint_layer.moc"
