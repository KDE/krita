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

#include <threadweaver/ThreadWeaver.h>

#include <kglobal.h>
#include <kconfig.h>

#include "kis_image.h"
#include "kis_group_layer.h"

using namespace ThreadWeaver;

class ProjectionJob : public Job
{
public:
    ProjectionJob( const QRect & rc, KisGroupLayerSP layer, QObject * parent )
        : Job( parent )
        , m_rc( rc )
        , m_rootLayer( layer )
        {
        }

    void run()
        {
            m_rootLayer->updateProjection( m_rc );
            // XXX: Also convert to QImage in the thread?
        }

    QRect rect() const { return m_rc; }

private:

    QRect m_rc;
    KisGroupLayerSP m_rootLayer;
};

class KisProjection::Private {
public:
    KisImageSP image;
    KisGroupLayerSP rootLayer;

    QRegion dirtyRegion; // The Qt manual assures me that QRegion is
                         // threadsafe... Let's hope that's really
                         // true!
    bool locked;
    Weaver * weaver;
    int updateRectSize;
};


KisProjection::KisProjection( KisImageSP image, KisGroupLayerSP rootLayer )
{
    m_d = new Private();
    m_d->image = image;
    m_d->rootLayer = rootLayer;
    m_d->locked = false;

    m_d->weaver = new Weaver();
    KSharedConfig::Ptr cfg = KGlobal::config();
    cfg->setGroup("");
    m_d->weaver->setMaximumNumberOfThreads( cfg->readEntry("maxthreads",  10) );
    m_d->updateRectSize = cfg->readEntry( "updaterectsize", 512 );
    connect( m_d->weaver, SIGNAL( jobDone(Job*) ), this, SLOT( slotUpdateUi( Job* ) ) );

    connect( this, SIGNAL( sigProjectionUpdated( const QRect & ) ), image.data(), SLOT(slotProjectionUpdated( const QRect &) ) );

    connect( rootLayer.data(), SIGNAL( sigDirtyRegionAdded( const QRegion & ) ), this, SLOT( slotAddDirtyRegion( const QRegion & ) ) );
    connect( rootLayer.data(), SIGNAL( sigDirtyRectAdded( const QRect & ) ), this, SLOT( slotAddDirtyRect( const QRect & ) ) );
}

KisProjection::~KisProjection()
{
    m_d->weaver->finish();
    delete m_d->weaver;
    delete m_d;
}

void KisProjection::lock()
{
    m_d->locked = true;
}

void KisProjection::unlock()
{
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
    m_d->rootLayer->disconnect( this );
    m_d->rootLayer = rootLayer;
    connect( rootLayer.data(), SIGNAL( sigDirtyRegionAdded( const QRegion & ) ), this, SLOT( slotAddDirtyRegion( const QRegion & ) ) );
    connect( rootLayer.data(), SIGNAL( sigDirtyRectAdded( const QRect & ) ), this, SLOT( slotAddDirtyRect( const QRect & ) ) );
}

bool KisProjection::upToDate(const QRect & rect)
{
    return m_d->dirtyRegion.intersects( QRegion( rect ) );
}

bool KisProjection::upToDate(const QRegion & region)
{
    return m_d->dirtyRegion.intersects( region );
}

void KisProjection::slotAddDirtyRegion( const QRegion & region )
{

    m_d->dirtyRegion += region;

#if 1
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
    m_d->dirtyRegion += QRegion( rect );
    if ( !m_d->locked ) {
        scheduleRect( rect );
    }
}

void KisProjection::slotUpdateUi( Job* job )
{
    ProjectionJob* pjob = static_cast<ProjectionJob*>( job );
    m_d->dirtyRegion -= QRegion( pjob->rect() );
    emit sigProjectionUpdated( pjob->rect() );
    delete pjob;
}

void KisProjection::scheduleRect( const QRect & rc )
{
    int h = rc.height();
    int w = rc.width();
    int x = rc.x();
    int y = rc.y();

    // Note: we're doing columns first, so when we have small strip left
    // at the bottom, we have as few and as long runs of pixels left
    // as possible.
    if ( w <= m_d->updateRectSize && h <= m_d->updateRectSize ) {
        ProjectionJob * job = new ProjectionJob( rc, m_d->rootLayer, this );
        m_d->weaver->enqueue( job );
    }


    int wleft = w;
    int col = 0;
    while ( wleft > 0 ) {
        int hleft = h;
        int row = 0;
        while ( hleft > 0 ) {
            QRect rc( col + x, row + y, qMin( wleft, m_d->updateRectSize ), qMin( hleft, m_d->updateRectSize ) );
            ProjectionJob * job = new ProjectionJob( rc, m_d->rootLayer, this );
            m_d->weaver->enqueue( job );
            hleft -= m_d->updateRectSize;
            row += m_d->updateRectSize;

        }
        wleft -= m_d->updateRectSize;
        col += m_d->updateRectSize;
    }
}
#include "kis_projection.moc"
