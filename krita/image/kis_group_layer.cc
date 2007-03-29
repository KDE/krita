/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#include <kglobal.h>
#include <kicon.h>
#include <QImage>
#include <QDateTime>

#include <KoColorSpace.h>

#include "kis_types.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_layer_visitor.h"
#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_merge_visitor.h"
#include "kis_fill_painter.h"

KisGroupLayer::KisGroupLayer(KisImageSP img, const QString &name, quint8 opacity) :
    super(img, name, opacity),
    m_x(0),
    m_y(0)
{
    m_projection = new KisPaintDevice(this, img->colorSpace(), name.toLatin1());
}

KisGroupLayer::KisGroupLayer(const KisGroupLayer &rhs) :
    super(rhs),
    m_x(rhs.m_x),
    m_y(rhs.m_y)
{
    for(vKisLayerSP_cit it = rhs.m_layers.begin(); it != rhs.m_layers.end(); ++it)
    {
        m_layers.push_back( it->data()->clone() );
    }
    m_projection = new KisPaintDevice(*rhs.m_projection.data());
    m_projection->setParentLayer(this);
}

KisLayerSP KisGroupLayer::clone() const
{
    return KisLayerSP(new KisGroupLayer(*this));
}

KisGroupLayer::~KisGroupLayer()
{
    m_layers.clear();
}

KoColorSpace * KisGroupLayer::colorSpace()
{
    // Due to virtual void resetProjection(KisPaintDeviceSP to =
    // 0), the colorspace of the group layer can be different from the
    // colorspace of the image. (XXX: is that desirable? BSAR)
    return m_projection->colorSpace();
}

QIcon KisGroupLayer::icon() const
{
    return KIcon("folder");
}


void KisGroupLayer::setDirty()
{
    QRect rc = extent();
    KisLayer::setDirty(rc);
    emit sigDirtyRegionAdded( extent() );
}

void KisGroupLayer::setDirty(const QRect & rc)
{
    KisLayer::setDirty(rc);
    emit sigDirtyRectAdded( rc );
}

void KisGroupLayer::setDirty( const QRegion & region)
{
    KisLayer::setDirty( region );
    emit sigDirtyRegionAdded( region );
}

void KisGroupLayer::resetProjection(KisPaintDeviceSP to)
{
    if (to)
        m_projection = new KisPaintDevice(*to); /// XXX ### look into Copy on Write here (CoW)
    else
        m_projection = new KisPaintDevice(this, image()->colorSpace(), name().toLatin1());
}

bool KisGroupLayer::paintLayerInducesProjectionOptimization(KisPaintLayerSP l) {
    return l && l->paintDevice()->colorSpace() == image()->colorSpace() && l->visible()
             && l->opacity() == OPACITY_OPAQUE && !l->temporaryTarget() && !l->hasMask();
}

KisPaintDeviceSP KisGroupLayer::projection()
{
    // We don't have a parent, and we've got only one child: abuse the child's
    // paint device as the projection if the child is visible
    if (parent().isNull() && childCount() == 1) {
        KisPaintLayerSP l = KisPaintLayerSP(dynamic_cast<KisPaintLayer*>(firstChild().data()));
        if (paintLayerInducesProjectionOptimization(l)) {
            return l->paintDevice();
        }
    }
    return m_projection;
}

uint KisGroupLayer::childCount() const
{
    return m_layers.count();
}

KisLayerSP KisGroupLayer::firstChild() const
{
    return at(0);
}

KisLayerSP KisGroupLayer::lastChild() const
{
    return at(childCount() - 1);
}

KisLayerSP KisGroupLayer::at(int index) const
{
    if (childCount() && index >= 0 && qBound(uint(0), uint(index), childCount() - 1) == uint(index))
        return m_layers.at(reverseIndex(index));
    return KisLayerSP(0);
}

int KisGroupLayer::index(KisLayerSP layer) const
{
    if (layer->parent().data() == this)
        return layer->index();
    return -1;
}
#if 0
void KisGroupLayer::setIndex(KisLayerSP layer, int index)
{
    if (layer->parent().data() != this)
        return;
    //TODO optimize
    removeLayer(layer);
    addLayer(layer, index);
}
#endif
bool KisGroupLayer::addLayer(KisLayerSP newLayer, int x)
{
    if (x < 0 || qBound(uint(0), uint(x), childCount()) != uint(x) ||
        newLayer->parent() || m_layers.contains(newLayer))
    {
        kWarning() << "invalid input to KisGroupLayer::addLayer(KisLayerSP newLayer, int x)!" << endl;
        return false;
    }
    notifyAboutToAdd(this, x);
    uint index(x);
    if (index == 0)
        m_layers.append(newLayer);
    else
        m_layers.insert(m_layers.begin() + reverseIndex(index) + 1, newLayer);

    for (uint i = childCount() - 1; i > index; i--) {
        KisLayerSP l = at( i );
        l->setIndexPrivate( l->index() + 1 );
    }
    newLayer->setParentPrivate( this );
    newLayer->setImage(image());
    newLayer->setIndexPrivate( index );
    newLayer->setDirty(newLayer->extent());

    notifyAdded(this, x);
    return true;
}

bool KisGroupLayer::addLayer(KisLayerSP newLayer, KisLayerSP aboveThis)
{
    kDebug() << "new layer: " << newLayer.data() << endl;
    if (aboveThis && aboveThis->parent().data() != this)
    {
        kWarning() << "invalid input to KisGroupLayer::addLayer(KisLayerSP newLayer, KisLayerSP aboveThis)!" << endl;
        return false;
    }
    return addLayer(newLayer, aboveThis ? aboveThis->index() : childCount());
}

bool KisGroupLayer::removeLayer(int x)
{
    if (x >= 0 && qBound(uint(0), uint(x), childCount() - 1) == uint(x))
    {
        uint index(x);
        for (uint i = childCount() - 1; i > index; i--) {
            KisLayerSP l = at( i );
            l->setIndexPrivate( l->index() -1 );
        }
        KisLayerSP removedLayer = at(index);

        removedLayer->setParentPrivate( 0 );
        removedLayer->setIndexPrivate( -1 );
        notifyAboutToRemove(this, x);
        m_layers.erase(m_layers.begin() + reverseIndex(index));
        setDirty(removedLayer->extent());
        if (childCount() < 1) {
            // No children, nothing to show for it.
            m_projection->clear();
            setDirty();
        }
        notifyRemoved(this, x);
        return true;
    }
    kWarning() << "invalid input to KisGroupLayer::removeLayer()!" << endl;
    return false;
}

bool KisGroupLayer::removeLayer(KisLayerSP layer)
{
    if (layer->parent().data() != this)
    {
        kWarning() << "invalid input to KisGroupLayer::removeLayer()!" << endl;
        return false;
    }

    return removeLayer(layer->index());
}

void KisGroupLayer::setImage(KisImageSP image)
{
    super::setImage(image);
    for (vKisLayerSP_it it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        (*it)->setImage(image);
    }
}

QRect KisGroupLayer::extent() const
{
    QRect groupExtent;

    for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        groupExtent |= (*it)->extent();
    }

    return groupExtent;
}

QRect KisGroupLayer::exactBounds() const
{

    QRect groupExactBounds;

    for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        groupExactBounds |= (*it)->exactBounds();
    }

    return groupExactBounds;
}

qint32 KisGroupLayer::x() const
{
    return m_x;
}

void KisGroupLayer::setX(qint32 x)
{
    qint32 delta = x - m_x;

    for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        KisLayerSP layer = *it;
        layer->setX(layer->x() + delta);
    }
    m_x = x;
}

qint32 KisGroupLayer::y() const
{
    return m_y;
}

void KisGroupLayer::setY(qint32 y)
{
    qint32 delta = y - m_y;

    for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        KisLayerSP layer = *it;
        layer->setY(layer->y() + delta);
    }

    m_y = y;
}

QImage KisGroupLayer::createThumbnail(qint32 w, qint32 h)
{
    return m_projection->createThumbnail(w, h);
}

void KisGroupLayer::updateProjection(const QRect & rc)
{
    if ( !rc.isValid() ) return ;

    // Get the first layer in this group to start compositing with
    KisLayerSP child = lastChild();

    // No child -- clear the projection. Without children, a group layer is empty.
    if (!child)
        m_projection->clear();
    else
        m_projection->clear( rc );

    KisLayerSP startWith = KisLayerSP(0);
#ifdef DIRTY_AND_PROJECTION
    KisAdjustmentLayerSP adjLayer = KisAdjustmentLayerSP(0);
    KisLayerSP tmpPaintLayer = KisLayerSP(0);

    // If this is the rootlayer, don't do anything with adj. layers that are below the
    // first paintlayer
    bool gotPaintLayer = (!parent().isNull());

    // Look through all the child layers, searching for the first dirty layer
    // if it's found, and if we have found an adj. layer before the the dirty layer,
    // composite from the first adjustment layer searching back from the first dirty layer
    while (child) {
        KisAdjustmentLayerSP tmpAdjLayer = KisAdjustmentLayerSP(dynamic_cast<KisAdjustmentLayer*>(child.data()));
        if (tmpAdjLayer) {
            if (gotPaintLayer) {
                // If this adjustment layer is dirty, start compositing with the
                // previous layer, if there's one.
                if (tmpAdjLayer->isDirty(rc) && !adjLayer.isNull() && adjLayer->visible()) {
                    startWith = adjLayer->prevSibling();
                    break;
                }
                else if (tmpAdjLayer->visible() && !tmpAdjLayer->isDirty(rc)) {
                    // This is the first adj. layer that is not dirty -- the perfect starting point
                    adjLayer = tmpAdjLayer;
                }
                else {
                    startWith = tmpPaintLayer;
                }
            }
        }
        else {
            tmpPaintLayer = child;
            gotPaintLayer = true;
            // A non-adjustmentlayer that's dirty; if there's an adjustmentlayer
            // with a cache, we'll start from there.
            if (child->isDirty(rc)) {
                if (!adjLayer.isNull() && adjLayer->visible()) {
                    // the first layer on top of the adj. layer
                    startWith = adjLayer->prevSibling();
                }
                else {
                    startWith = child;
                }
                // break here: if there's no adj layer, we'll start with the layer->lastChild
                break;
            }
        }
        child = child->prevSibling();
    }

    if (!adjLayer.isNull() && startWith.isNull() && gotPaintLayer && adjLayer->prevSibling()) {
        startWith = adjLayer->prevSibling();
    }

    // No adj layer -- all layers inside the group must be recomposited
    if (adjLayer.isNull()) {
        startWith = lastChild();
    }
#endif

    startWith = lastChild();

    if (startWith.isNull()) {
        return;
    }
    m_projection->clear( rc );

    bool first = true; // The first layer in a stack needs special compositing
#ifdef DIRTY_AND_PROJECTION
    // Fill the projection either the cached data, if it's there
    KisFillPainter gc(m_projection);

    if (!adjLayer.isNull()) {
        gc.bitBlt(rc.left(), rc.top(),
                  COMPOSITE_COPY, adjLayer->cachedPaintDevice(), OPACITY_OPAQUE,
                  rc.left(), rc.top(), rc.width(), rc.height());
        first = false;
    }
    else {
        first = true;
    }
    gc.end();
#endif

    KisMergeVisitor visitor(m_projection, rc);

    child = startWith;

    while(child)
    {
        if(first)
        {
            // Copy the lowest layer rather than compositing it with the background
            // or an empty image. This means the layer's composite op is ignored,
            // which is consistent with Photoshop and gimp.
            const KoCompositeOp * cop = child->compositeOp();

            const bool block = child->signalsBlocked();
            child->blockSignals(true);

            // Composite Op copy doesn't take a mask/selection into account, so we need
            // to make a difference between a paintlayer with a mask, and one without
            KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(child.data());

            if (l && l->hasMask())
                child->setCompositeOpPrivate( cop->colorSpace()->compositeOp( COMPOSITE_OVER ) );
            else
                child->setCompositeOpPrivate( cop->colorSpace()->compositeOp( COMPOSITE_COPY ) );

            child->blockSignals(block);
            child->accept(visitor);
            child->blockSignals(true);
            child->setCompositeOpPrivate( cop );
            child->blockSignals(block);
            first = false;
        }
        else
            child->accept(visitor);

        child = child->prevSibling();
    }
}

void KisGroupLayer::notifyAboutToAdd(KisGroupLayer *p, int index)
{
    beginInsertRows(indexFromLayer(p), index, index);
    if (parent())
        parent()->notifyAboutToAdd(p, index);
}

void KisGroupLayer::notifyAdded(KisGroupLayer *p, int index)
{
    endInsertRows();
    if (parent())
        parent()->notifyAdded(p, index);
}

void KisGroupLayer::notifyAboutToRemove(KisGroupLayer *p, int index)
{
    beginRemoveRows(indexFromLayer(p), index, index);
    if (parent())
        parent()->notifyAboutToRemove(p, index);
}

void KisGroupLayer::notifyRemoved(KisGroupLayer *p, int index)
{
    endRemoveRows();
    if (parent())
        parent()->notifyRemoved(p, index);
}


#include "kis_group_layer.moc"
