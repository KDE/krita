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

#include "kis_dyna_paintop.h"
#include "kis_dyna_paintop_settings.h"

#include <cmath>
#include <math.h>

#include <QRect>
#include <QColor>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_random_accessor.h>

#include "filter.h"

KisDynaPaintOp::KisDynaPaintOp(const KisDynaPaintOpSettings *settings, KisPainter * painter, KisImageSP image)
    : KisPaintOp( painter )
    , m_settings( settings )
    , m_image ( image )
{
    m_dynaBrush.setRadius( settings->radius() );
    m_dynaBrush.setAmount( settings->amount() );
    m_dynaBrush.setImage( image );
}

KisDynaPaintOp::~KisDynaPaintOp()
{
}

double KisDynaPaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, double savedDist){

    dab = cachedDab();
    dab->clear();

    qreal x1, y1;

    x1 = pi1.pos().x();
    y1 = pi1.pos().y();

    m_dynaBrush.initMouse( pi2.pos() );
    m_dynaBrush.paint(dab, x1, y1, painter()->paintColor());

    QRect rc = dab->extent();

    painter()->bltSelection(
        rc.x(), rc.y(),
        painter()->compositeOp(),
        dab,
        painter()->opacity(),
        rc.x(), rc.y(),
        rc.width(), rc.height());


    KisVector2D end = toKisVector2D(pi2.pos());
    KisVector2D start = toKisVector2D(pi1.pos());
    KisVector2D dragVec = end - start;
    return  dragVec.norm();
}

void KisDynaPaintOp::paintAt(const KisPaintInformation& info)
{
/*    if (!painter()) return;

    dab = cachedDab();
    dab->clear();

    qreal x1, y1;

    x1 = info.pos().x();
    y1 = info.pos().y();

    m_dynaBrush.initMouse( info.pos() );
    m_dynaBrush.paint(dab, x1, y1, painter()->paintColor());

    QRect rc = dab->extent();

    painter()->bltSelection(
        rc.x(), rc.y(),
        painter()->compositeOp(),
        dab,
        painter()->opacity(),
        rc.x(), rc.y(),
        rc.width(), rc.height());
*/
}
