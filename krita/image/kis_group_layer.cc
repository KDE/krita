/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#include <kglobal.h>
#include <kicon.h>
#include <QImage>
#include <QDateTime>

#include <KoColorSpace.h>

#include "kis_types.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_node_visitor.h"
#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_merge_visitor.h"
#include "kis_fill_painter.h"
#include "kis_projection.h"

class KisGroupLayer::Private
{
public:
    Private()
        : projection( 0 )
//         , projectionManager( 0 )
        , cacheProjection( true )
        , x( 0 )
        , y( 0 )
        {
        }

    KisPaintDeviceSP projection; // The cached composition of all
                                 // layers in this group
//     KisProjectionSP projectionManager; // owned by KisImage
    bool cacheProjection;
    qint32 x;
    qint32 y;
};

KisGroupLayer::KisGroupLayer(KisImageWSP img, const QString &name, quint8 opacity) :
    KisLayer(img, name, opacity),
    m_d( new Private() )
{
    m_d->projection = new KisPaintDevice(this, img->colorSpace(), name.toLatin1());
    updateSettings();
}

KisGroupLayer::KisGroupLayer(const KisGroupLayer &rhs) :
    KisLayer(rhs),
    m_d( new Private() )
{
    m_d->projection = new KisPaintDevice(*rhs.m_d->projection.data());
    updateSettings();
}

KisLayerSP KisGroupLayer::clone() const
{
    return KisLayerSP(new KisGroupLayer(*this));
}

KisGroupLayer::~KisGroupLayer()
{
    delete m_d;
}

KoColorSpace * KisGroupLayer::colorSpace()
{
    // Due to virtual void resetProjection(KisPaintDeviceSP to =
    // 0), the colorspace of the group layer can be different from the
    // colorspace of the image. (XXX: is that desirable? BSAR)
    return m_d->projection->colorSpace();
}

QIcon KisGroupLayer::icon() const
{
    return KIcon("folder");
}

void KisGroupLayer::setDirty()
{
    KisLayer::setDirty();
    emit rectDirtied( extent() );
//     if ( m_d->projectionManager )
//         m_d->projectionManager->addDirtyRect( extent() );
}


void KisGroupLayer::setDirty(const QRect & rect)
{
    KisLayer::setDirty( rect );
    emit rectDirtied( rect );
//     if ( m_d->projectionManager )
//         m_d->projectionManager->addDirtyRect( rect );
}


void KisGroupLayer::setDirty( const QRegion & region)
{
    KisLayer::setDirty( region );
    emit regionDirtied( region );
//     if ( m_d->projectionManager )
//         m_d->projectionManager->addDirtyRegion( region );

}


void KisGroupLayer::updateSettings()
{
    KConfigGroup cfg = KGlobal::config()->group("");
    m_d->cacheProjection = cfg.readEntry( "useProjections", true );
    emit settingsUpdated();
//     if ( m_d->projectionManager )
//         m_d->projectionManager->updateSettings();
}

// void KisGroupLayer::setProjectionManager( KisProjectionSP projectionManager )
// {
//     m_d->projectionManager = projectionManager;
// }

void KisGroupLayer::resetProjection(KisPaintDeviceSP to)
{
    if (to)
        m_d->projection = new KisPaintDevice(*to); /// XXX ### look into Copy on Write here (CoW)
    else
        m_d->projection = new KisPaintDevice(this, image()->colorSpace(), name().toLatin1());
}

bool KisGroupLayer::paintLayerInducesProjectionOptimization(KisPaintLayerSP l) const {
    return l && l->paintDevice()->colorSpace() == image()->colorSpace() && l->visible()
        && l->opacity() == OPACITY_OPAQUE && !l->temporaryTarget();// && !l->hasMask();
}

KisPaintDeviceSP KisGroupLayer::projection() const
{
    // We don't have a parent, and we've got only one child: abuse the child's
    // paint device as the projection if the child is visible
    if (parent().isNull() && childCount() == 1) {
        KisPaintLayer * l = dynamic_cast<KisPaintLayer*>(firstChild().data());
        if (paintLayerInducesProjectionOptimization(l)) {
            return l->paintDevice();
        }
    }
    return m_d->projection;
}

QRect KisGroupLayer::extent() const
{
    QRect groupExtent;

    for (uint i = 0; i < childCount(); ++i)
    {
        groupExtent |= (at( i ))->extent();
    }

    return groupExtent;
}

QRect KisGroupLayer::exactBounds() const
{

    QRect groupExactBounds;

    for (uint i = 0; i < childCount(); ++i)
    {
        groupExactBounds |= (at( i ))->exactBounds();
    }

    return groupExactBounds;
}

bool KisGroupLayer::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}


qint32 KisGroupLayer::x() const
{
    return m_d->x;
}

void KisGroupLayer::setX(qint32 x)
{
    qint32 delta = x - m_d->x;

    for (uint i = 0; i < childCount(); ++i)
    {
        KisNodeSP layer = at( i );
        layer->setX(layer->x() + delta);
    }
    m_d->x = x;
}

qint32 KisGroupLayer::y() const
{
    return m_d->y;
}

void KisGroupLayer::setY(qint32 y)
{
    qint32 delta = y - m_d->y;

    for (uint i = 0; i < childCount(); ++i)
    {
        KisNodeSP layer = at( i );
        layer->setY(layer->y() + delta);
    }

    m_d->y = y;
}

QImage KisGroupLayer::createThumbnail(qint32 w, qint32 h)
{
    return m_d->projection->createThumbnail(w, h);
}

void KisGroupLayer::updateProjection(const QRect & rc)
{
    if ( !rc.isValid() ) return ;
    if ( !isDirty( rc ) ) return;

    // Get the first layer in this group to start compositing with
    KisLayerSP child = dynamic_cast<KisLayer*>( lastChild().data() );

    // No child -- clear the projection. Without children, a group layer is empty.
    if (!child)
        m_d->projection->clear();
    else
        m_d->projection->clear( rc );

    KisLayerSP startWith = KisLayerSP(0);

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
                    startWith = dynamic_cast<KisLayer*>( adjLayer->prevSibling().data() );
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
        startWith = dynamic_cast<KisLayer*>( lastChild().data() );
    }


    startWith = dynamic_cast<KisLayer*>( lastChild().data() );

    if (startWith.isNull()) {
        return;
    }
    m_d->projection->clear( rc );

    bool first = true; // The first layer in a stack needs special compositing

    // Fill the projection either the cached data, if it's there
    KisFillPainter gc(m_d->projection);

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


    KisMergeVisitor visitor(m_d->projection, rc);

    child = startWith;

    while(child)
    {
        if(first)
        {
            // Copy the lowest layer rather than compositing it with the background
            // or an empty image. This means the layer's composite op is ignored,
            // which is consistent with Photoshop and gimp.
            const KoCompositeOp * cop = child->compositeOp();

            // Composite Op copy doesn't take a mask/selection into account, so we need
            // to make a difference between a paintlayer with a mask, and one without
            KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(child.data());

            if (l && l->hasEffectMasks())
                child->setCompositeOp( cop->colorSpace()->compositeOp( COMPOSITE_OVER ) );
            else
                child->setCompositeOp( cop->colorSpace()->compositeOp( COMPOSITE_COPY ) );

            child->accept(visitor);
            child->setCompositeOp( cop );
            first = false;
        }
        else
            child->accept(visitor);

        child = dynamic_cast<KisLayer*>( child->prevSibling().data() );
    }
}

#include "kis_group_layer.moc"
