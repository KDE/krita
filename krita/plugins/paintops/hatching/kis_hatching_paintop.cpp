/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_hatching_paintop.h"
#include "kis_hatching_paintop_settings.h"

#include <cmath>
#include <QRect>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_paint_information.h>

#include <kis_pressure_opacity_option.h>

KisHatchingPaintOp::KisHatchingPaintOp(const KisHatchingPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
        , m_image(image)
{
    m_settings = new KisHatchingPaintOpSettings();
    settings->initializeTwin(m_settings);
    
    m_hatchingBrush = new HatchingBrush(m_settings);
    //m_opacityOption.readOptionSetting(settings);
    //m_opacityOption.sensor()->reset();
    //m_attributes.loadSettings(settings);
}

KisHatchingPaintOp::~KisHatchingPaintOp()
{
    delete m_hatchingBrush;
}

double KisHatchingPaintOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) return 1; /*
    if (!painter()->device()) return 1;
    if (!m_hatchingBrush) return 1;
   */ 
    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    } else {
        m_dab->clear();
    }
    /*
    KisFixedPaintDeviceSP dab = cachedDab();
    KoColor color = painter()->paintColor();
    color.convertTo(dab->colorSpace());
    //brush->mask(dab, color, scale, scale, 0.0, info);
    */
    
    qreal x1, y1;

    x1 = info.pos().x();
    y1 = info.pos().y();

    if (!m_settings->subpixelprecision)
    {
        modf(x1, &x1);
        modf(y1, &y1);
    }
    
    //quint8 origOpacity = m_opacityOption.apply(painter(), info);
    m_hatchingBrush->paint(m_dab, x1, y1, painter()->paintColor());

    QRect rc = m_dab->extent();
    QRect limits(QPoint(0,0), QPoint(fabs(m_settings->origin_x), fabs(m_settings->origin_y)));
    
    if (m_settings->opaquebackground)
    {
        KoColor aersh = painter()->backgroundColor();
        painter()->device()->fill((x1), (y1), (limits.width()-1), (limits.height()-1), aersh.data()); //this plus yellow background = french fry brush
    }
    painter()->bitBlt(x1, y1, m_dab, 0, 0, limits.width(), limits.height());
    
    printf ("Ancho: %i . Alto: %i. \n", limits.width(), limits.height());
    //painter()->setOpacity(origOpacity);
    return 1;
}



