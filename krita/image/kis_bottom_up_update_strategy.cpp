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

#include "kis_node.h"
#include "kis_image.h"
#include "kis_projection.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_merge_visitor.h"

class KisBottomUpUpdateStrategy::Private
{
public:

    Private()
        : node(0)
        , image(0)
        , projection(0)
        , regionLock( QMutex::Recursive )
        {
        }

    KisNodeSP node;
    KisImageSP image;
    KisProjection * projection;
    
#ifdef USE_PAINTERPATH
    QPainterPath dirtyArea;
#else
    QRegion dirtyRegion;
#endif    
    QMutex regionLock;
};

KisBottomUpUpdateStrategy::KisBottomUpUpdateStrategy( KisNodeSP node )
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

void KisBottomUpUpdateStrategy::setDirty(const QRect & rc, const KisNodeSP filthyNode )
{
    Q_UNUSED( filthyNode );
    
    // If we're dirty, our parent is dirty, if we've got a parent
    if ( m_d->node->parent() ) {
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
        
        connect( m_d->projection, SIGNAL( sigProjectionUpdated( const QRect & ) ), 
                 m_d->image.data(), SLOT( slotProjectionUpdated( const QRect &) ) );
        connect( this, SIGNAL( rectDirtied(const QRect &)), 
                 m_d->projection, SLOT( addDirtyRect( const QRect & ) ) );
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

bool KisBottomUpUpdateStrategy::isDirty( const QRect & rect ) const
{
    QMutexLocker(&m_d->regionLock);
    return m_d->dirtyRegion.intersects( rect );
}

void KisBottomUpUpdateStrategy::setClean( const QRect & rc )
{
    QMutexLocker(&m_d->regionLock);
#ifdef USE_PAINTERPATH
    QPainterPath p;
    p.addRect(QRectF(rc));
    m_d->dirtyArea = m_d->dirtyArea.subtracted(p);
#else    
    m_d->dirtyRegion -= QRegion( rc );
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

QRegion KisBottomUpUpdateStrategy::dirtyRegion( const QRect & rc )
{
    QMutexLocker(&m_d->regionLock);
    return m_d->dirtyRegion.intersected( QRegion( rc) );
}

KisPaintDeviceSP KisBottomUpUpdateStrategy::updateGroupLayerProjection( const QRect & rc, KisPaintDeviceSP projection )
{

    if ( !rc.isValid() ) return projection;
    if ( !isDirty( rc ) ) return projection;

    // Get the first layer in this group to start compositing with
    KisLayerSP child = dynamic_cast<KisLayer*>( m_d->node->lastChild().data() );

    // No child -- clear the projection. Without children, a group layer is empty.
    if (!child) {
        projection->clear();
        return projection;
    }
    else {
        projection->clear( rc );
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
                if (   static_cast<KisBottomUpUpdateStrategy*>(tmpAdjLayer->updateStrategy())->isDirty(rc)
                    && !adjLayer.isNull() 
                    && adjLayer->visible()) 
                {
                    startWith = dynamic_cast<KisLayer*>( adjLayer->prevSibling().data() );
                    break;
                }
                else if (   tmpAdjLayer->visible() 
                         && !static_cast<KisBottomUpUpdateStrategy*>(tmpAdjLayer->updateStrategy())->isDirty(rc)) 
                {
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
            if (static_cast<KisBottomUpUpdateStrategy*>(child->updateStrategy())->isDirty(rc)) {
                if (!adjLayer.isNull() && adjLayer->visible()) {
                    // the first layer on top of the adj. layer
                    startWith = dynamic_cast<KisLayer*>( adjLayer->prevSibling().data() );
                }
                else {
                    startWith = child;
                }
                // break here: if there's no adj layer, we'll start with the layer->lastChild
                break;
            }
        }
        child = dynamic_cast<KisLayer*>( child->prevSibling().data() );
    }

    if (!adjLayer.isNull() && startWith.isNull() && gotPaintLayer && adjLayer->prevSibling()) {
        startWith = dynamic_cast<KisLayer*>( adjLayer->prevSibling().data() );
    }

    // No adj layer -- all layers inside the group must be recomposited
    if (adjLayer.isNull()) {
        startWith = dynamic_cast<KisLayer*>( m_d->node->firstChild().data() );
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
    }
    else {
        first = true;
    }

    KisMergeVisitor visitor(projection, rc);

    child = startWith;

    while( child )
    {
        child->accept(visitor);
        child = dynamic_cast<KisLayer*>( child->nextSibling().data() );
    }
    
    return projection;
}

#include "kis_bottom_up_update_strategy.moc"
