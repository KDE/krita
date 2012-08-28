/*
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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

#include <KoIcon.h>
#include <KoCompositeOp.h>
#include <KoColorSpace.h>

#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_default_bounds.h"
#include "kis_clone_layer.h"
#include "kis_selection_mask.h"

struct KisGroupLayer::Private
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
    resetCache();
}

KisGroupLayer::KisGroupLayer(const KisGroupLayer &rhs) :
    KisLayer(rhs),
    m_d(new Private())
{
    m_d->paintDevice = new KisPaintDevice(*rhs.m_d->paintDevice.data());
    m_d->x = rhs.m_d->x;
    m_d->y = rhs.m_d->y;
    m_d->paintDevice->setDefaultPixel(const_cast<KisGroupLayer*>(&rhs)->m_d->paintDevice->defaultPixel());
}

KisGroupLayer::~KisGroupLayer()
{
    delete m_d;
}

bool KisGroupLayer::checkCloneLayer(KisCloneLayerSP clone) const
{
    KisNodeSP source = clone->copyFrom();
    if (source) {
        if(!allowAsChild(source)) return false;

        if (source->inherits("KisGroupLayer")) {
            KisNodeSP newParent = const_cast<KisGroupLayer*>(this);
            while (newParent) {
                if (newParent == source) {
                    return false;
                }
                newParent = newParent->parent();
            }
        }
    }

    return true;
}

bool KisGroupLayer::checkNodeRecursively(KisNodeSP node) const
{
    KisCloneLayerSP cloneLayer = dynamic_cast<KisCloneLayer*>(node.data());
    if(cloneLayer) {
        return checkCloneLayer(cloneLayer);
    }
    else if (node->inherits("KisGroupLayer")) {
        KisNodeSP child = node->firstChild();
        while (child) {
            if (!checkNodeRecursively(child)) {
                return false;
            }
            child = child->nextSibling();
        }
    }

    return true;
}

bool KisGroupLayer::allowAsChild(KisNodeSP node) const
{
    return checkNodeRecursively(node) &&
            (parent() ||
             (node->inherits("KisSelectionMask") && !selectionMask()) ||
             !node->inherits("KisMask"));
}

const KoColorSpace * KisGroupLayer::colorSpace() const
{
    return m_d->paintDevice->colorSpace();
}

QIcon KisGroupLayer::icon() const
{
    return koIcon("folder");
}

void KisGroupLayer::setImage(KisImageWSP image)
{
    m_d->paintDevice->setDefaultBounds(new KisDefaultBounds(image));
    KisLayer::setImage(image);
}

void KisGroupLayer::resetCache(const KoColorSpace *colorSpace)
{
    if (!colorSpace)
        colorSpace = image()->colorSpace();

    Q_ASSERT(colorSpace);

    if (!m_d->paintDevice) {

        m_d->paintDevice = new KisPaintDevice(this, colorSpace, new KisDefaultBounds(image()));
        m_d->paintDevice->setX(m_d->x);
        m_d->paintDevice->setY(m_d->y);
    }
    else if(!(*m_d->paintDevice->colorSpace() == *colorSpace)) {

        KisPaintDeviceSP dev = new KisPaintDevice(this, colorSpace, new KisDefaultBounds(image()));
        dev->setX(m_d->x);
        dev->setY(m_d->y);
        quint8* defaultPixel = colorSpace->allocPixelBuffer(1);
        colorSpace->convertPixelsTo(m_d->paintDevice->defaultPixel(), defaultPixel, colorSpace, 1, KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::BlackpointCompensation);
        dev->setDefaultPixel(defaultPixel);
        delete[] defaultPixel;
        m_d->paintDevice = dev;
    } else {

        m_d->paintDevice->clear();
    }
}

KisLayer* KisGroupLayer::onlyMeaningfulChild() const
{
    KisNode *child = firstChild().data();
    KisLayer *onlyLayer = 0;

    while (child) {
        KisLayer *layer = dynamic_cast<KisLayer*>(child);
        if (layer) {
            if (onlyLayer) return 0;
            onlyLayer = layer;
        }
        child = child->nextSibling().data();
    }

    return onlyLayer;
}

KisPaintDeviceSP KisGroupLayer::tryObligeChild() const
{
    const KisLayer *child = onlyMeaningfulChild();

    if (child &&
            child->channelFlags().isEmpty() &&
            child->projection() &&
            child->visible() &&
            (child->compositeOpId() == COMPOSITE_OVER ||
             child->compositeOpId() == COMPOSITE_ALPHA_DARKEN ||
             child->compositeOpId() == COMPOSITE_COPY) &&
            child->opacity() == OPACITY_OPAQUE_U8 &&
            *child->projection()->colorSpace() == *colorSpace()) {

        return child->projection();
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

void KisGroupLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
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
    qint32 delta = x - m_d->x;
    m_d->x = x;
    if(m_d->paintDevice) {
        m_d->paintDevice->setX(m_d->paintDevice->x() + delta);
        Q_ASSERT(m_d->paintDevice->x() == m_d->x);
    }
}

void KisGroupLayer::setY(qint32 y)
{
    qint32 delta = y - m_d->y;
    m_d->y = y;
    if(m_d->paintDevice) {
        m_d->paintDevice->setY(m_d->paintDevice->y() + delta);
        Q_ASSERT(m_d->paintDevice->y() == m_d->y);
    }
}

#include "kis_group_layer.moc"
