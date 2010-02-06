/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#include "kis_group_layer.h"

#include <KoColorSpace.h>

#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_paint_device.h"

class KisGroupLayer::Private
{
public:
    Private()
            : paintDevice(0)
            , x(0)
            , y(0) {
    }

    KisPaintDeviceSP paintDevice;
    qint32 x;
    qint32 y;
};

KisGroupLayer::KisGroupLayer(KisImageWSP image, const QString &name, quint8 opacity) :
        KisLayer(image, name, opacity),
        m_d(new Private())
{
    m_d->paintDevice = new KisPaintDevice(this, image->colorSpace());
}

KisGroupLayer::KisGroupLayer(const KisGroupLayer &rhs) :
        KisLayer(rhs),
        m_d(new Private())
{
    m_d->paintDevice = new KisPaintDevice(*rhs.m_d->paintDevice.data());
    m_d->paintDevice->setDefaultPixel(const_cast<KisGroupLayer*>(&rhs)->m_d->paintDevice->defaultPixel());
}

KisGroupLayer::~KisGroupLayer()
{
    delete m_d;
}

bool KisGroupLayer::allowAsChild(KisNodeSP node) const
{
    Q_UNUSED(node);
    return true;
}

const KoColorSpace * KisGroupLayer::colorSpace() const
{
    return m_d->paintDevice->colorSpace();
}

QIcon KisGroupLayer::icon() const
{
    return KIcon("folder");
}

void KisGroupLayer::resetCache(const KoColorSpace *colorSpace)
{
    if (!colorSpace)
        colorSpace = image()->colorSpace();

    if (!m_d->paintDevice) {

        m_d->paintDevice = new KisPaintDevice(colorSpace);
    }
    else if(!(*m_d->paintDevice->colorSpace() == *colorSpace)) {

        KisPaintDeviceSP dev = new KisPaintDevice(colorSpace);
        quint8* defaultPixel = colorSpace->allocPixelBuffer(1);
        colorSpace->convertPixelsTo(m_d->paintDevice->defaultPixel(), defaultPixel, colorSpace, 1);
        dev->setDefaultPixel(defaultPixel);
        delete[] defaultPixel;
        m_d->paintDevice = dev;
    } else {

        m_d->paintDevice->clear();
    }
}

KisPaintDeviceSP KisGroupLayer::tryObligeChild() const
{
    if (/*parent().isNull() &&*/ childCount() == 1) {
        const KisLayer *child = dynamic_cast<KisLayer*>(firstChild().data());

        if (child &&
                child->channelFlags().isEmpty() &&
                child->projection() &&
                child->visible() &&
                child->opacity() == OPACITY_OPAQUE &&
                *child->projection()->colorSpace() == *colorSpace()) {

            return child->projection();
        }
    }

    return 0;
}

KisPaintDeviceSP KisGroupLayer::original() const
{
    /**
     * We are too lazy! Let's our children work for us.
     * Try to use children's paintDevice if it's the only
     * one in stack and meets some conditions
     */
    KisPaintDeviceSP childOriginal = tryObligeChild();
    return childOriginal ? childOriginal : m_d->paintDevice;
}

bool KisGroupLayer::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

qint32 KisGroupLayer::x() const
{
    return m_d->x;
}

qint32 KisGroupLayer::y() const
{
    return m_d->y;
}

void KisGroupLayer::setX(qint32 x)
{
    qint32 numChildren = childCount();

    qint32 delta = x - m_d->x;
    for (qint32 i = 0; i < numChildren; ++i) {
        KisNodeSP layer = at(i);
        layer->setX(layer->x() + delta);
    }
    m_d->x = x;
}

void KisGroupLayer::setY(qint32 y)
{
    qint32 numChildren = childCount();

    qint32 delta = y - m_d->y;
    for (qint32 i = 0; i < numChildren; ++i) {
        KisNodeSP layer = at(i);
        layer->setY(layer->y() + delta);
    }
    m_d->y = y;
}

#include "kis_group_layer.moc"
