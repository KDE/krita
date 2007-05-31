/*
 *  Copyright (c) 2007 Boudewijn Rempt
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

#include "kis_clone_layer.h"

#include <QImage>
#include <QString>

#include <kicon.h>
#include <kdebug.h>

#include <KoColorSpace.h>

#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_layer_visitor.h"
#include "kis_layer.h"

class KisCloneLayer::Private {
public:
    KisPaintDeviceSP projection;
    KisLayerSP copyFrom;
    CopyLayerType type;
    qint32 x;
    qint32 y;
};

KisCloneLayer::KisCloneLayer(KisLayerSP from, KisImageSP img, const QString &name, quint8 opacity)
    : KisLayer( img, name, opacity )
{
    m_d = new Private();
    m_d->projection = 0;
    m_d->copyFrom = from;
    m_d->type = COPY_PROJECTION;
    m_d->x = 0;
    m_d->y = 0;
}

KisCloneLayer::KisCloneLayer(const KisCloneLayer& rhs)
    : KisLayer( rhs )
    , KisIndirectPaintingSupport( rhs )
{
    m_d = new Private();
    // XXX: Yah, booh!
    m_d->projection = const_cast<KisPaintDevice*>( rhs.projection().data() );
    m_d->copyFrom = rhs.copyFrom();
    m_d->type = rhs.copyType();
    m_d->x = rhs.x();
    m_d->y = rhs.y();
}

KisCloneLayer::~KisCloneLayer()
{
    delete m_d;
}

void KisCloneLayer::updateProjection(const QRect& r)
{
    if ( !m_d->copyFrom ) return;

    // if there are effect masks, apply them to the either original or
    // to the original's projection.
    if ( hasEffectMasks() ) {
        if ( m_d->projection == 0 ) {
            m_d->projection = new KisPaintDevice( m_d->copyFrom->colorSpace() );
        }

        qint32 deltaX = m_d->copyFrom->x() - m_d->x;
        qint32 deltaY = m_d->copyFrom->y() - m_d->y;
        QRect rc = r.translated( deltaX, deltaY );

        KisPainter gc( m_d->projection );
        gc.setCompositeOp( colorSpace()->compositeOp( COMPOSITE_COPY ) );

        switch ( m_d->type ) {
        case COPY_PROJECTION:
            gc.bitBlt( rc.topLeft(), m_d->copyFrom->projection(), rc );
            break;
        case COPY_ORIGINAL:
        default:
            gc.bitBlt( rc.topLeft(), m_d->copyFrom->original(), rc );
        }
        applyEffectMasks(m_d->projection, r);
    }
}


KisPaintDeviceSP KisCloneLayer::projection() const
{
    // if there are no effect masks and x & y are the same, return
    // either the original data or the projection of the original
    // layer.

    // if there are no effect masks but x & y are different, create a
    // kispaintdevice that shares the data manager with the original
    // and set x and y different, return that

    return m_d->projection;
}

KisPaintDeviceSP KisCloneLayer::paintDevice() const
{
    return 0;
}

QIcon KisCloneLayer::icon() const
{
    return KIcon("edit-copy");
}

KoDocumentSectionModel::PropertyList KisCloneLayer::properties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::properties();
    if ( m_d->copyFrom )
        l << KoDocumentSectionModel::Property(i18n("Copy From"), m_d->copyFrom->name());
    return l;
}

/// Return a copy of this layer
KisLayerSP KisCloneLayer::clone() const
{
    return KisLayerSP( new KisCloneLayer( *this ) );
}

qint32 KisCloneLayer::x() const
{
    return m_d->x;
}

void KisCloneLayer::setX(qint32 x)
{
    m_d->x = x;
    setDirty();
}

qint32 KisCloneLayer::y() const
{
    return m_d->y;
}

void KisCloneLayer::setY(qint32 y)
{
    m_d->y = y;
    setDirty();
}

/// Returns an approximation of where the bounds on actual data are in this layer
QRect KisCloneLayer::extent() const
{
    if ( m_d->copyFrom ) {
        QRect rc = m_d->copyFrom->extent();
        rc.moveTo( m_d->x, m_d->y );
        return rc;
    }
    return QRect();
}

/// Returns the exact bounds of where the actual data resides in this layer
QRect KisCloneLayer::exactBounds() const
{
    if ( m_d->copyFrom ) {
        QRect rc = m_d->copyFrom->exactBounds();
        rc.moveTo( m_d->x, m_d->y );
        return rc;
    }
    return QRect();
}

bool KisCloneLayer::accept(KisLayerVisitor & v)
{
    return v.visit( this );
}

QImage KisCloneLayer::createThumbnail(qint32 w, qint32 h)
{
    if ( !m_d->copyFrom ) return QImage();

    if ( hasEffectMasks() ) {
        return m_d->projection->createThumbnail( w, h );
    }
    else {
        return m_d->copyFrom->createThumbnail( w, h );
    }
}

void KisCloneLayer::setCopyFrom( KisLayerSP fromLayer, CopyLayerType type )
{
    m_d->type = type;
    m_d->copyFrom = fromLayer;
    setDirty();
}

KisLayerSP KisCloneLayer::copyFrom() const
{
    return m_d->copyFrom;
}

void KisCloneLayer::setCopyType( CopyLayerType type )
{
    if ( type != m_d->type ) {
        m_d->type = type;
        if ( m_d->copyFrom )
            setDirty();
    }
}

CopyLayerType KisCloneLayer::copyType() const
{
    return m_d->type;
}
