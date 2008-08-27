/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_bottom_up_update_strategy.h"

#include <QPainterPath>
#include <QRegion>
#include <QRect>


#include <KoCompositeOp.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_node_visitor.h"
#include "kis_painter.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "generator/kis_generator_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_paint_layer.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_transaction.h"
#include "kis_iterators_pixel.h"
#include "kis_clone_layer.h"
#include "kis_processing_information.h"
#include "kis_node.h"
#include "kis_projection.h"

namespace
{
/**
 * The class merge visitor works using a bottom-up recomposition strategy.
 */
class KisMergeVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    /**
     * Don't even _think_ of creating a merge visitor without a projection; without a projection,
     * the adjustmentlayers won't work.
     */
    KisMergeVisitor(KisPaintDeviceSP projection, const QRect& rc) :
            KisNodeVisitor() {
        Q_ASSERT(projection);
        m_projection = projection;
        m_rc = rc;
    }

public:

    bool visit(KisExternalLayer * layer) {
        if (m_projection.isNull()) {
            return false;
        }
        if (!layer->visible())
            return true;

        layer->updateProjection(m_rc);
        KisPaintDeviceSP dev = layer->projection();
        if (!dev)
            return true;

        qint32 sx, sy, dx, dy, w, h;

        QRect rc = dev->extent() & m_rc;

        sx = rc.left();
        sy = rc.top();
        w = rc.width();
        h = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);
        gc.setChannelFlags(layer->channelFlags());
        gc.bitBlt(dx, dy, layer->compositeOp() , dev, layer->opacity(), sx, sy, w, h);

        static_cast<KisBottomUpUpdateStrategy*>(layer->updateStrategy())->setClean(rc);

        return true;
    }


    bool visit(KisGeneratorLayer * layer) {
        if (m_projection.isNull()) {
            return false;
        }
        if (!layer->visible())
            return true;

        layer->updateProjection(m_rc);
        KisPaintDeviceSP dev = layer->projection();
        if (!dev)
            return true;

        qint32 sx, sy, dx, dy, w, h;

        QRect rc = dev->extent() & m_rc;

        sx = rc.left();
        sy = rc.top();
        w = rc.width();
        h = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);
        gc.setChannelFlags(layer->channelFlags());
        gc.bitBlt(dx, dy, layer->compositeOp() , dev, layer->opacity(), sx, sy, w, h);

        static_cast<KisBottomUpUpdateStrategy*>(layer->updateStrategy())->setClean(rc);

        return true;
    }

    bool visit(KisPaintLayer *layer) {

        if (m_projection.isNull()) {
            return false;
        }

        bool first = false;
        if (layer->prevSibling() == 0 && layer->parent() == layer->image()->root())
            first = true;

        if (!layer->visible()) {
            if (first) {
                m_projection->clear();
            }
            return true;
        }

        qint32 sx, sy, dx, dy, w, h;

        QRect rc = layer->paintDevice()->extent() & m_rc;

        // Indirect painting?
        KisPaintDeviceSP tempTarget = layer->temporaryTarget();
        if (tempTarget) {
            rc = (layer->projection()->extent() | tempTarget->extent()) & m_rc;
        }

        sx = rc.left();
        sy = rc.top();
        w  = rc.width();
        h  = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);
        gc.setChannelFlags(layer->channelFlags());

        KisBottomUpUpdateStrategy * updateStrategy =
            static_cast<KisBottomUpUpdateStrategy*>(layer->updateStrategy());

        if (updateStrategy->isDirty(rc)) {
            QRegion dirty = updateStrategy->dirtyRegion(m_rc);
            foreach(const QRect & rect, dirty.rects()) {
                layer->updateProjection(rect);
            }
        }

        KisPaintDeviceSP source = layer->projection();

        if (tempTarget) {
            KisPaintDeviceSP temp = new KisPaintDevice(source->colorSpace());
            source = paintIndirect(source, temp, layer, sx, sy, dx, dy, w, h);
        }

        if (first)
            gc.bitBlt(dx, dy, m_projection->colorSpace()->compositeOp(COMPOSITE_COPY), source, layer->opacity(), sx, sy, w, h);
        else
            gc.bitBlt(dx, dy, layer->compositeOp(), source, layer->opacity(), sx, sy, w, h);


        updateStrategy->setClean(rc);

        return true;
    }

    bool visit(KisGroupLayer *layer) {

        if (m_projection.isNull()) {
            return false;
        }

        if (!layer->visible())
            return true;

        qint32 sx, sy, dx, dy, w, h;

        layer->updateProjection(m_rc);
        KisPaintDeviceSP dev = layer->projection();

        QRect rc = dev->extent() & m_rc;

        sx = rc.left();
        sy = rc.top();
        w  = rc.width();
        h  = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);
        gc.setChannelFlags(layer->channelFlags());
        gc.bitBlt(dx, dy, layer->compositeOp(), dev, layer->opacity(), sx, sy, w, h);

        static_cast<KisBottomUpUpdateStrategy*>(layer->updateStrategy())->setClean(rc);

        return true;
    }

    bool visit(KisAdjustmentLayer* layer) {
        if (m_projection.isNull()) {
            return true;
        }

        if (!layer->visible())
            return true;

        QRect tmpRc = m_rc;

        KisPaintDeviceSP tempTarget = layer->temporaryTarget();
        dbgImage << "tempTarget: " << tempTarget;
        if (tempTarget) {
            tmpRc = (layer->extent() | tempTarget->extent()) & tmpRc;
        }

        if (tmpRc.width() == 0 || tmpRc.height() == 0) // Don't even try
            return true;

        dbgImage << "Filtering on " << tmpRc;

        KisFilterConfiguration * cfg = layer->filter();
        if (!cfg) return false;

        KisFilterSP f = KisFilterRegistry::instance()->value(cfg->name());
        if (!f) return false;

        // Possibly enlarge the rect that changed (like for convolution filters)
        tmpRc = f->enlargeRect(tmpRc, cfg);

        KisSelectionSP selection = layer->selection();
        KisPaintDeviceSP layerProjection = layer->projection();

        // It's necessary to copy the unselected pixels to the projection cache inside
        // the adjustment layer to make the merge optimization in the grouplayer work.
        KisPainter gc1(layerProjection);
        gc1.setCompositeOp(layerProjection->colorSpace()->compositeOp(COMPOSITE_COPY));
        gc1.bitBlt(m_rc.topLeft(), m_projection, m_rc);
        gc1.end();

        KisConstProcessingInformation srcCfg(m_projection, tmpRc .topLeft(), 0);
        KisProcessingInformation dstCfg(layerProjection, tmpRc .topLeft(), 0);

        // Some filters will require usage of oldRawData, which is not available without
        // a transaction!
        KisTransaction* cmd = new KisTransaction("", layerProjection);
        f->process(srcCfg, dstCfg, tmpRc.size(), cfg);
        delete cmd;

        // Copy the filtered bits onto the projection
        KisPainter gc(m_projection);
        if (selection)
            gc.bltSelection(tmpRc.left(), tmpRc.top(),
                            layer->compositeOp(), layerProjection, selection, layer->opacity(),
                            tmpRc.left(), tmpRc.top(), tmpRc.width(), tmpRc.height());
        else
            gc.bitBlt(tmpRc.left(), tmpRc.top(),
                      layer->compositeOp(), layerProjection, layer->opacity(),
                      tmpRc.left(), tmpRc.top(), tmpRc.width(), tmpRc.height());
        gc.end();

        static_cast<KisBottomUpUpdateStrategy*>(layer->updateStrategy())->setClean(tmpRc);

        return true;
    }


    bool visit(KisCloneLayer * layer) {

        if (m_projection.isNull()) {
            return false;
        }

        if (!layer->visible())
            return true;

        qint32 sx, sy, dx, dy, w, h;
        if (!static_cast<KisBottomUpUpdateStrategy*>(layer->updateStrategy())->isDirty(m_rc)) {
            layer->updateProjection(m_rc);
        }
        KisPaintDeviceSP dev = layer->projection();

        if (!dev) return false;

        QRect rc = dev->extent() & m_rc;

        sx = rc.left();
        sy = rc.top();
        w  = rc.width();
        h  = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);
        gc.setCompositeOp(layer->compositeOp());
        gc.setOpacity(layer->opacity());
        gc.setChannelFlags(layer->channelFlags());

        gc.bitBlt(rc.topLeft(), dev, rc);

        static_cast<KisBottomUpUpdateStrategy*>(layer->updateStrategy())->setClean(rc);

        return true;

    }

private:
    // Helper for the indirect painting
    template<class Target>
    KisSharedPtr<Target> paintIndirect(KisPaintDeviceSP source,
                                       KisSharedPtr<Target> target,
                                       KisIndirectPaintingSupport* layer,
                                       qint32 sx, qint32 sy, qint32 dx, qint32 dy,
                                       qint32 w, qint32 h) {
        KisPainter gc2(target.data());
        gc2.bitBlt(dx, dy, COMPOSITE_COPY, source,
                   OPACITY_OPAQUE, sx, sy, w, h);
        gc2.bitBlt(dx, dy, layer->temporaryCompositeOp(), layer->temporaryTarget(),
                   layer->temporaryOpacity(), sx, sy, w, h);
        gc2.end();
        return target;
    }
    KisPaintDeviceSP m_projection;
    QRect m_rc;
};
}


class KisBottomUpUpdateStrategy::Private
{
public:

    Private()
            : node(0)
            , image(0)
            , projection(0)
            , regionLock(QMutex::Recursive) {
    }

    KisNodeWSP node;
    KisImageSP image;
    KisProjection * projection;

#ifdef USE_PAINTERPATH
    QPainterPath dirtyArea;
#else
    QRegion dirtyRegion;
#endif
    QMutex regionLock;
};

KisBottomUpUpdateStrategy::KisBottomUpUpdateStrategy(KisNodeWSP node)
        : KisProjectionUpdateStrategy()
        , m_d(new Private())
{
    m_d->node = node;
}


KisBottomUpUpdateStrategy::~KisBottomUpUpdateStrategy()
{
    if (m_d->projection)
        delete m_d->projection;
    delete m_d;
}

void KisBottomUpUpdateStrategy::setDirty(const QRect & rc)
{
    // If we're dirty, our parent is dirty, if we've got a parent
    if (m_d->node->parent()) {
        m_d->node->parent()->setDirty(rc);
    }

    QMutexLocker(&m_d->regionLock);
#ifdef USE_PAINTERPATH
    m_d->dirtyArea.addRect(rc)
#else
    m_d->dirtyRegion += QRegion(rc);
#endif

    if (m_d->projection) {
        emit rectDirtied(rc);
    }
}

void KisBottomUpUpdateStrategy::lock()
{
    if (m_d->projection)
        m_d->projection->lock();
}

void KisBottomUpUpdateStrategy::unlock()
{
    if (m_d->projection)
        m_d->projection->unlock();
}

void KisBottomUpUpdateStrategy::setImage(KisImageSP image)
{
    // Hey, we're the root node!
    if (image && m_d->node->inherits("KisGroupLayer") && !m_d->node->parent()) {

        m_d->image = image;
        m_d->projection = new KisProjection(image);
        m_d->projection->setRootLayer(static_cast<KisGroupLayer*>(m_d->node.data()));

        connect(m_d->projection, SIGNAL(sigProjectionUpdated(const QRect &)),
                m_d->image.data(), SLOT(slotProjectionUpdated(const QRect &)));
        connect(this, SIGNAL(rectDirtied(const QRect &)),
                m_d->projection, SLOT(addDirtyRect(const QRect &)));
    }
}


bool KisBottomUpUpdateStrategy::isDirty() const
{
    QMutexLocker(&m_d->regionLock);
#ifdef USE_PAINTERPATH
    return !m_d->dirtyArea.isEmpty();
#else
    return !m_d->dirtyRegion.isEmpty();
#endif

}

bool KisBottomUpUpdateStrategy::isDirty(const QRect & rect) const
{
    QMutexLocker(&m_d->regionLock);
    return m_d->dirtyRegion.intersects(rect);
}

void KisBottomUpUpdateStrategy::setClean(const QRect & rc)
{
    QMutexLocker(&m_d->regionLock);
#ifdef USE_PAINTERPATH
    QPainterPath p;
    p.addRect(QRectF(rc));
    m_d->dirtyArea = m_d->dirtyArea.subtracted(p);
#else
    m_d->dirtyRegion -= QRegion(rc);
#endif
}

void KisBottomUpUpdateStrategy::setClean()
{
    QMutexLocker(&m_d->regionLock);
#ifdef USE_PAINTERPATH
    m_d->dirtyArea = QPainterPath();
#else
    m_d->dirtyRegion = QRegion();
#endif
}

QRegion KisBottomUpUpdateStrategy::dirtyRegion(const QRect & rc)
{
    QMutexLocker(&m_d->regionLock);
    return m_d->dirtyRegion.intersected(QRegion(rc));
}

KisPaintDeviceSP KisBottomUpUpdateStrategy::updateGroupLayerProjection(const QRect & rc, KisPaintDeviceSP projection)
{

    if (!rc.isValid()) return projection;
    if (!isDirty(rc)) return projection;

    // Get the first layer in this group to start compositing with
    KisLayerSP child = dynamic_cast<KisLayer*>(m_d->node->lastChild().data());

    // No child -- clear the projection. Without children, a group layer is empty.
    if (!child) {
        projection->clear();
        return projection;
    } else {
        projection->clear(rc);
    }

    KisLayerSP startWith = KisLayerSP(0);

    KisAdjustmentLayerSP adjLayer = KisAdjustmentLayerSP(0);
    KisLayerSP tmpPaintLayer = KisLayerSP(0);

    // If this is the rootlayer, don't do anything with adj. layers that are below the
    // first paintlayer
    bool gotPaintLayer = (!m_d->node->parent().isNull());

    // Look through all the child layers, searching for the first dirty layer
    // if it's found, and if we have found an adj. layer before the the dirty layer,
    // composite from the first adjustment layer searching back from the first dirty layer
    while (child) {
        KisAdjustmentLayerSP tmpAdjLayer = KisAdjustmentLayerSP(dynamic_cast<KisAdjustmentLayer*>(child.data()));
        if (tmpAdjLayer) {
            if (gotPaintLayer) {
                // If this adjustment layer is dirty, start compositing with the
                // previous layer, if there's one.
                if (static_cast<KisBottomUpUpdateStrategy*>(tmpAdjLayer->updateStrategy())->isDirty(rc)
                        && !adjLayer.isNull()
                        && adjLayer->visible()) {
                    startWith = dynamic_cast<KisLayer*>(adjLayer->prevSibling().data());
                    break;
                } else if (tmpAdjLayer->visible()
                           && !static_cast<KisBottomUpUpdateStrategy*>(tmpAdjLayer->updateStrategy())->isDirty(rc)) {
                    // This is the first adj. layer that is not dirty -- the perfect starting point
                    adjLayer = tmpAdjLayer;
                } else {
                    startWith = tmpPaintLayer;
                }
            }
        } else {
            tmpPaintLayer = child;
            gotPaintLayer = true;
            // A non-adjustmentlayer that's dirty; if there's an adjustmentlayer
            // with a cache, we'll start from there.
            if (static_cast<KisBottomUpUpdateStrategy*>(child->updateStrategy())->isDirty(rc)) {
                if (!adjLayer.isNull() && adjLayer->visible()) {
                    // the first layer on top of the adj. layer
                    startWith = dynamic_cast<KisLayer*>(adjLayer->prevSibling().data());
                } else {
                    startWith = child;
                }
                // break here: if there's no adj layer, we'll start with the layer->lastChild
                break;
            }
        }
        child = dynamic_cast<KisLayer*>(child->prevSibling().data());
    }

    if (!adjLayer.isNull() && startWith.isNull() && gotPaintLayer && adjLayer->prevSibling()) {
        startWith = dynamic_cast<KisLayer*>(adjLayer->prevSibling().data());
    }

    // No adj layer -- all layers inside the group must be recomposited
    if (adjLayer.isNull()) {
        startWith = dynamic_cast<KisLayer*>(m_d->node->firstChild().data());
    }

    if (startWith.isNull()) {
        return projection;
    }

    bool first = true; // The first layer in a stack needs special compositing

    if (!adjLayer.isNull()) {
        KisPainter gc(projection);
        gc.bitBlt(rc.left(), rc.top(),
                  COMPOSITE_COPY, adjLayer->cachedPaintDevice(), OPACITY_OPAQUE,
                  rc.left(), rc.top(), rc.width(), rc.height());
        gc.end();
        first = false;
    } else {
        first = true;
    }

    KisMergeVisitor visitor(projection, rc);

    child = startWith;

    while (child) {
        child->accept(visitor);
        child = dynamic_cast<KisLayer*>(child->nextSibling().data());
    }

    return projection;
}

#include "kis_bottom_up_update_strategy.moc"
