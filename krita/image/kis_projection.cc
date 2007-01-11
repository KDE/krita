/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org
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
#include "kis_projection.h"

#include <QRegion>
#include <QRect>
#include <QVariant>

#include <threadAction/KoAction.h>
#include <KoExecutePolicy.h>
#include <threadweaver/ThreadWeaver.h>

#include "kis_image.h"
#include "kis_group_layer.h"

const int UPDATE_RECT_SIZE = 1024;

class KisProjection::Private {
public:
    KisImageSP image;
    KisGroupLayerSP rootLayer;
    KoAction * action;
    QRegion dirtyRegion; // The Qt manual assures me that QRegion is
                         // threadsafe... Let's hope that's really
                         // true!
    bool locked;
};


KisProjection::KisProjection( KisImageSP image, KisGroupLayerSP rootLayer )
{
    m_d = new Private();
    m_d->image = image;
    m_d->rootLayer = rootLayer;
    m_d->locked = false;
    m_d->action = new KoAction( this );
    // XXX: I need a policy that enables me to run a given number of
    // jobs concurrently, say 4 or so.
    m_d->action->setExecutePolicy( KoExecutePolicy::directPolicy ); // Why doesn't it execute with the simpleQueuedPolicy? That's what I want.
    m_d->action->setWeaver( ThreadWeaver::Weaver::instance() );

    connect( m_d->action, SIGNAL( triggered( const QVariant & ) ), SLOT( slotTriggered (const QVariant &)) );
    connect( m_d->action, SIGNAL( updateUi( const QVariant & ) ), SLOT( slotUpdateUi (const QVariant &)) );

    connect( this, SIGNAL( sigProjectionUpdated( const QRect & ) ), image.data(), SLOT(slotProjectionUpdated( const QRect &) ) );

    connect( rootLayer.data(), SIGNAL( sigDirtyRegionAdded( const QRegion & ) ), this, SLOT( slotAddDirtyRegion( const QRegion & ) ) );
    connect( rootLayer.data(), SIGNAL( sigDirtyRectAdded( const QRect & ) ), this, SLOT( slotAddDirtyRect( const QRect & ) ) );
}

KisProjection::~KisProjection()
{
    delete m_d->action;
    delete m_d;
}

void KisProjection::lock()
{
    kDebug(41010 ) << "KisProjection::lock()\n";

    m_d->locked = true;
}

void KisProjection::unlock()
{
    kDebug(41010 ) << "KisProjection::unlock()\n";

    m_d->locked = false;
    QVector<QRect> regionRects = m_d->dirtyRegion.rects();

    QVector<QRect>::iterator it = regionRects.begin();
    QVector<QRect>::iterator end = regionRects.end();
    while ( it != end ) {
        scheduleRect( *it );
        ++it;
    }

}

void KisProjection::setRootLayer( KisGroupLayerSP rootLayer )
{
    kDebug(41010) << "KisProjection::setRootLayer old: " << m_d->rootLayer << ", new: " << rootLayer << endl;
    m_d->rootLayer->disconnect( this );
    m_d->rootLayer = rootLayer;
    connect( rootLayer.data(), SIGNAL( sigDirtyRegionAdded( const QRegion & ) ), this, SLOT( slotAddDirtyRegion( const QRegion & ) ) );
    connect( rootLayer.data(), SIGNAL( sigDirtyRectAdded( const QRect & ) ), this, SLOT( slotAddDirtyRect( const QRect & ) ) );
}

bool KisProjection::upToDate(const QRect & rect)
{
    kDebug(41010) << "KisProjection::upToDate " << rect << endl;
    return m_d->dirtyRegion.intersects( QRegion( rect ) );
}

bool KisProjection::upToDate(const QRegion & region)
{
    kDebug(41010) << "KisProjection::upToDate " << region << endl;
    return m_d->dirtyRegion.intersects( region );
}

void KisProjection::slotAddDirtyRegion( const QRegion & region )
{

    //kDebug(41010) << "KisProjection::slotAddDirtyRegion " << region << ", bounding rect: " << region.boundingRect() << endl;
#if 1
    m_d->dirtyRegion += region;
    if ( !m_d->locked )
        scheduleRect( region.boundingRect() );
#else
    if ( !m_d->locked ) {
        QVector<QRect> regionRects = region.rects();

        QVector<QRect>::iterator it = regionRects.begin();
        QVector<QRect>::iterator end = regionRects.end();
        while ( it != end ) {
            scheduleRect( *it );
            ++it;
        }
    }
#endif

}

void KisProjection::slotAddDirtyRect( const QRect & rect )
{
    kDebug(41010) << "KisProjection::slotAddDirtyRect " << rect << endl;
    m_d->dirtyRegion += QRegion( rect );
    if ( !m_d->locked ) {
        scheduleRect( rect );
    }
}

void KisProjection::slotTriggered( const QVariant & rect )
{
    // Recomposite
    kDebug(41010) << "KisProjection::slotTriggered " << rect.toRect() << endl;
    m_d->rootLayer->updateProjection( rect.toRect() );
}

void KisProjection::slotUpdateUi( const QVariant & rect )
{
    kDebug(41010) << "KisProjection::slotUpdateUI " << rect.toRect() << endl;
    QRect rc = rect.toRect();
    m_d->dirtyRegion -= QRegion( rc );
    emit sigProjectionUpdated( rc );
}

void KisProjection::scheduleRect( const QRect & rc )
{
    kDebug(41010) << "Scheduled big rect: " << rc << endl;
    int h = rc.height();
    int w = rc.width();
    QRect imageRect = m_d->image->bounds();

    QVariant v;

    // Note: we're doing columns first, so when we have small strip left
    // at the bottom, we have as few and as long runs of pixels left
    // as possible.
    if ( w <= UPDATE_RECT_SIZE && h <= UPDATE_RECT_SIZE ) {
        v = QRect( rc );
        m_d->action->execute( &v );
    }

    int x = rc.x();
    int y = rc.y();

    int wleft = w;
    int col = 0;
    while ( wleft > 0 ) {
        int hleft = h;
        int row = 0;
        while ( hleft > 0 ) {
            v = QRect( col + x, row + y, qMin( wleft, UPDATE_RECT_SIZE ), qMin( hleft, UPDATE_RECT_SIZE ) );
            kDebug(41010) << "Scheduling subrect : " << v << ", wleft: " << wleft << ", hleft:" << hleft << endl;
            m_d->action->execute( &v );
            hleft -= UPDATE_RECT_SIZE;
            row += UPDATE_RECT_SIZE;

        }
        wleft -= UPDATE_RECT_SIZE;
        col += UPDATE_RECT_SIZE;
    }
}
#include "kis_projection.moc"
