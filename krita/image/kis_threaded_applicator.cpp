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
#include "kis_threaded_applicator.h"

#include <QRect>

#include <threadweaver/ThreadWeaver.h>

#include <kglobal.h>
#include <ksharedconfig.h>

#include "kis_paint_device.h"

using namespace ThreadWeaver;

class KisThreadedApplicator::Private {

public:

    KisPaintDeviceSP dev;
    QRect rc;
    KisJobFactory * jobFactory;
    int margin;
    int maxThreads;
    int tileSize;
    Weaver * weaver;
};


KisThreadedApplicator::KisThreadedApplicator( KisPaintDeviceSP dev, const QRect & rc, KisJobFactory * jobFactory, int margin )
{
    m_d = new Private();
    m_d->dev = dev;
    m_d->rc = rc;
    m_d->jobFactory = jobFactory;
    m_d->margin = margin;

    KSharedConfig::Ptr cfg = KGlobal::config();
    cfg->setGroup("");
    m_d->maxThreads = cfg->readEntry("maxthreads",  10);
    m_d->tileSize = cfg->readEntry( "threadingtilesize", 512 );

    m_d->weaver = new Weaver();
    m_d->weaver->setMaximumNumberOfThreads( m_d->maxThreads );
    connect( m_d->weaver, SIGNAL( jobDone(Job*) ), this, SLOT( jobDone( Job* ) ) );
}


KisThreadedApplicator::~KisThreadedApplicator()
{
    m_d->weaver->finish();
    delete m_d->weaver;
    delete m_d;
}

void KisThreadedApplicator::execute()
{
    int h = m_d->rc.height();
    int w = m_d->rc.width();
    int x = m_d->rc.x();
    int y = m_d->rc.y();

    // Note: we're doing columns first, so when we have small strip left
    // at the bottom, we have as few and as long runs of pixels left
    // as possible.
    if ( w <= m_d->tileSize && h <= m_d->tileSize ) {
        Job * job = m_d->jobFactory->createJob( this, m_d->dev, m_d->rc, m_d->margin );
        m_d->weaver->enqueue( job );
    }


    int wleft = w;
    int col = 0;
    while ( wleft > 0 ) {
        int hleft = h;
        int row = 0;
        while ( hleft > 0 ) {
            QRect subrect( col + x, row + y, qMin( wleft, m_d->tileSize ), qMin( hleft, m_d->tileSize ) );
            Job * job = m_d->jobFactory->createJob( this, m_d->dev, subrect, m_d->margin );
            m_d->weaver->enqueue( job );
            hleft -= m_d->tileSize;
            row += m_d->tileSize;

        }
        wleft -= m_d->tileSize;
        col += m_d->tileSize;
    }
    m_d->weaver->finish();
}

void KisThreadedApplicator::jobDone( Job* job)
{
    KisJob* kisJob = static_cast<KisJob*>( job );
    kisJob->jobDone();
    delete job;
}

#include "kis_threaded_applicator.moc"
