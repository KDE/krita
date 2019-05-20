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
#include <kis_icon.h>
#include <KoCompositeOpRegistry.h>
#include <KoColorSpace.h>
#include <KoColor.h>


#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_default_bounds.h"
#include "kis_clone_layer.h"
#include "kis_selection_mask.h"
#include "kis_psd_layer_style.h"
#include "kis_layer_properties_icons.h"


struct Q_DECL_HIDDEN KisGroupLayer::Private
{
public:
    Private()
        : paintDevice(0)
        , x(0)
        , y(0)
        , passThroughMode(false)
    {
    }

    KisPaintDeviceSP paintDevice;
    qint32 x;
    qint32 y;
    bool passThroughMode;
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
    m_d->paintDevice->setProjectionDevice(true);
    m_d->passThroughMode = rhs.passThroughMode();
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
    if (!checkNodeRecursively(node)) return false;

    if (!parent()) {
        // We are the root layer, so we need to check
        // whether the node that's going to be added is
        // a selection mask; that is only allowed if
        // there isn't a global selection. See
        // BUG:294905

        if (node->inherits("KisSelectionMask")) {
            return !selectionMask();
        }

        KisImageSP image = this->image();

        if (!image || !image->allowMasksOnRootNode()) {
            if (node->inherits("KisMask")) {
                return false;
            }
        }
    }

    return checkNodeRecursively(node);

}

const KoColorSpace * KisGroupLayer::colorSpace() const
{
    return m_d->paintDevice->colorSpace();
}

QIcon KisGroupLayer::icon() const
{
    return KisIconUtils::loadIcon("groupLayer");
}

void KisGroupLayer::setImage(KisImageWSP image)
{
    m_d->paintDevice->setDefaultBounds(new KisDefaultBounds(image));
    KisLayer::setImage(image);
}

KisLayerSP KisGroupLayer::createMergedLayerTemplate(KisLayerSP prevLayer)
{
    KisGroupLayer *prevGroup = dynamic_cast<KisGroupLayer*>(prevLayer.data());

    if (prevGroup && canMergeAndKeepBlendOptions(prevLayer)) {
        KisSharedPtr<KisGroupLayer> merged(new KisGroupLayer(*prevGroup));

        KisNodeSP child, cloned;

        for (child = firstChild(); child; child = child->nextSibling()) {
            cloned = child->clone();
            image()->addNode(cloned, merged);
        }

        if (!merged->passThroughMode()) {
            image()->refreshGraphAsync(merged);
        }

        return merged;
    } else
        return KisLayer::createMergedLayerTemplate(prevLayer);
}

void KisGroupLayer::fillMergedLayerTemplate(KisLayerSP dstLayer, KisLayerSP prevLayer)
{
    if (!dynamic_cast<KisGroupLayer*>(dstLayer.data())) {
        KisLayer::fillMergedLayerTemplate(dstLayer, prevLayer);
    }
}

void KisGroupLayer::resetCache(const KoColorSpace *colorSpace)
{
    if (!colorSpace)
        colorSpace = image()->colorSpace();

    Q_ASSERT(colorSpace);

    if (!m_d->paintDevice) {

        KisPaintDeviceSP dev = new KisPaintDevice(this, colorSpace, new KisDefaultBounds(image()));
        dev->setX(this->x());
        dev->setY(this->y());
        m_d->paintDevice = dev;
        m_d->paintDevice->setProjectionDevice(true);
    }
    else if(*m_d->paintDevice->colorSpace() != *colorSpace) {

        KisPaintDeviceSP dev = new KisPaintDevice(this, colorSpace, new KisDefaultBounds(image()));
        dev->setX(this->x());
        dev->setY(this->y());
        dev->setDefaultPixel(m_d->paintDevice->defaultPixel());

        m_d->paintDevice = dev;
        m_d->paintDevice->setProjectionDevice(true);
    } else {

        m_d->paintDevice->clear();
    }
}

KisLayer* KisGroupLayer::onlyMeaningfulChild() const
{
    KisNode *child = firstChild().data();
    KisLayer *onlyLayer = 0;

    while (child) {
        KisLayer *layer = qobject_cast<KisLayer*>(child);
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
        *child->projection()->colorSpace() == *colorSpace() &&
        !child->layerStyle()) {

        quint8 defaultOpacity =
            m_d->paintDevice->defaultPixel().opacityU8();

        if(defaultOpacity == OPACITY_TRANSPARENT_U8) {
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
    KisPaintDeviceSP realOriginal = tryObligeChild();

    if (!realOriginal) {
        if (!childCount() && !m_d->paintDevice->extent().isEmpty()) {
            m_d->paintDevice->clear();
        }
        realOriginal = m_d->paintDevice;
    }

    return realOriginal;
}

KisPaintDeviceSP KisGroupLayer::paintDevice() const
{
    return 0;
}

bool KisGroupLayer::projectionIsValid() const
{
    return !tryObligeChild();
}

void KisGroupLayer::setDefaultProjectionColor(KoColor color)
{
    m_d->paintDevice->setDefaultPixel(color);
}

KoColor KisGroupLayer::defaultProjectionColor() const
{
    KoColor color(m_d->paintDevice->defaultPixel(), m_d->paintDevice->colorSpace());
    return color;
}

bool KisGroupLayer::passThroughMode() const
{
    return m_d->passThroughMode;
}

void KisGroupLayer::setPassThroughMode(bool value)
{
    if (m_d->passThroughMode == value) return;

    m_d->passThroughMode = value;

    baseNodeChangedCallback();
    baseNodeInvalidateAllFramesCallback();
}

KisBaseNode::PropertyList KisGroupLayer::sectionModelProperties() const
{
    KisBaseNode::PropertyList l = KisLayer::sectionModelProperties();

    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::passThrough, passThroughMode());

    return l;
}

void KisGroupLayer::setSectionModelProperties(const KisBaseNode::PropertyList &properties)
{
    Q_FOREACH (const KisBaseNode::Property &property, properties) {
        if (property.name == i18n("Pass Through")) {
            setPassThroughMode(property.state.toBool());
        }
    }

    KisLayer::setSectionModelProperties(properties);
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
    return m_d->paintDevice ? m_d->paintDevice->x() : m_d->x;
}

qint32 KisGroupLayer::y() const
{
    return m_d->paintDevice ? m_d->paintDevice->y() : m_d->y;
}

void KisGroupLayer::setX(qint32 x)
{
    m_d->x = x;
    if(m_d->paintDevice) {
        m_d->paintDevice->setX(x);
    }
}

void KisGroupLayer::setY(qint32 y)
{
    m_d->y = y;
    if(m_d->paintDevice) {
        m_d->paintDevice->setY(y);
    }
}

struct ExtentPolicy
{
    inline QRect operator() (const KisNode *node) {
        return node->extent();
    }
};

struct ExactBoundsPolicy
{
    inline QRect operator() (const KisNode *node) {
        return node->exactBounds();
    }
};

template <class MetricPolicy>
QRect collectRects(const KisNode *node, MetricPolicy policy)
{
    QRect accumulator;

    const KisNode *child = node->firstChild();
    while (child) {
        if (!qobject_cast<const KisMask*>(child)) {
            accumulator |= policy(child);
        }
        child = child->nextSibling();
    }

    return accumulator;
}

QRect KisGroupLayer::extent() const
{
    return m_d->passThroughMode ?
        collectRects(this, ExtentPolicy()) :
        KisLayer::extent();
}

QRect KisGroupLayer::exactBounds() const
{
    return m_d->passThroughMode ?
        collectRects(this, ExactBoundsPolicy()) :
        KisLayer::exactBounds();
}
