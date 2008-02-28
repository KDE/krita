/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2007
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

#include "filter/kis_filter_job.h"
#include <QObject>
#include <QRect>

#include <kis_debug.h>

#include <KoProgressUpdater.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>


#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_paint_device.h"
#include "filter/kis_filter_processing_information.h"
#include "kis_painter.h"
#include "kis_selection.h"

KisFilterJob::KisFilterJob( const KisFilter* filter,
                            const KisFilterConfiguration * config,
                            QObject * parent, KisPaintDeviceSP dev,
                            const QRect & rc,
                            int margin,
                            KoUpdater * updater  )
    : KisJob( parent, dev, rc, margin )
    , m_filter( filter )
    , m_config( config )
    , m_updater( updater)
{
}


void KisFilterJob::run()
{
    // XXX: Is it really necessary to output the filter on a second paint device and
    //      then blit it back? (boud)
    KisPaintDeviceSP dst = new KisPaintDevice( m_dev->colorSpace() );
    QRect marginRect = m_rc.adjusted( -m_margin, -m_margin, m_margin, m_margin );

    m_filter->process( KisFilterConstProcessingInformation( m_dev, marginRect.topLeft()),
                        KisFilterProcessingInformation( dst, marginRect.topLeft() ),
                        marginRect.size(),
                        m_config,
                        m_updater );
    KisPainter p( m_dev );
    p.setCompositeOp( m_dev->colorSpace()->compositeOp( COMPOSITE_COPY ) );
    p.bitBlt( m_rc.topLeft(), dst, m_rc );
    p.end();
    m_updater->setProgress(100);
}

KisFilterJobFactory::KisFilterJobFactory( const KisFilter* filter, const KisFilterConfiguration * config )
    : m_filter( filter )
    , m_config( config )
{
}

ThreadWeaver::Job * KisFilterJobFactory::createJob(QObject * parent, KisPaintDeviceSP dev, const QRect & rc, int margin, KoUpdater * updater )
{
    return new KisFilterJob( m_filter, m_config, parent, dev, rc, margin, updater );
}

