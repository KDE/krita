/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_deform_paintop.h"
#include "kis_deform_paintop_settings.h"

#include <cmath>

#include <QRect>
#include <QList>
#include <QColor>
//#include <QMutexLocker>

#include <qdebug.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoInputDevice.h>
#include <KoCompositeOp.h>

#include <kis_image.h>
#include <kis_debug.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_random_accessor.h"

#include "kis_datamanager.h"


KisDeformPaintOp::KisDeformPaintOp(const KisDeformPaintOpSettings *settings, KisPainter * painter, KisImageSP image)
        : KisPaintOp(painter)
{
    Q_ASSERT(settings);
    m_image = image;
    m_deformBrush.setAction( settings->deformAction() );
    m_deformBrush.setRadius( settings->radius() );
    m_deformBrush.setDeformAmount ( settings->deformAmount() );
    m_deformBrush.setInterpolation( settings->bilinear() );
    m_deformBrush.setImage(image);
    m_deformBrush.setCounter(1);
    m_useMovementPaint = settings->useMovementPaint();
    m_deformBrush.setUseCounter( settings->useCounter() );
    m_deformBrush.setUseOldData( settings->useOldData() );

    if ( !settings->node() ){
        m_dev = 0;
    }else{
        m_dev = settings->node()->paintDevice();
    }
}

KisDeformPaintOp::~KisDeformPaintOp()
{
}

void KisDeformPaintOp::paintAt(const KisPaintInformation& info)
{
//    QMutexLocker locker(&m_mutex);

    if (!painter()) return;
    /* m_dev = painter()->device(); */
    if (!m_dev) return;

    dab = cachedDab();
    dab->clear();

    //write device, read device, position 
    m_deformBrush.paint(dab,m_dev, info);

    QRect rc = dab->extent();

    painter()->bitBlt( rc.x(), rc.y(), dab, rc.x(), rc.y(), rc.width(), rc.height());
}


double KisDeformPaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, double savedDist)
{
    paintAt(pi1);
    paintAt(pi2);
    return 0;
}
