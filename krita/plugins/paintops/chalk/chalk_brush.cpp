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

#include "chalk_brush.h"
#include "brush_shape.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>

#include "kis_random_accessor.h"
#include <cmath>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif



ChalkBrush::ChalkBrush(const KisChalkPaintOpSettings* settings)
{
    m_counter = 0;
    m_settings = settings;
    m_radius = settings->radius();
    init();
}


ChalkBrush::~ChalkBrush()
{
}

void ChalkBrush::init()
{
    BrushShape bs;
    // some empiric values
    bs.fromGaussian(m_radius, 1.0f, 0.9f);
    m_bristles = bs.getBristles();
    srand48(time(0));
}

void ChalkBrush::paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color)
{
    m_inkColor = color;
    m_counter++;

    Bristle *bristle;

    qint32 pixelSize = dev->colorSpace()->pixelSize();
    KisRandomAccessor accessor = dev->createRandomAccessor((int)x, (int)y);

    qreal result;
    if (m_settings->inkDepletion()){
        //count decrementing of saturation and opacity
        result = log((qreal)m_counter);
        result = -(result * 10) / 100.0;

        if ( m_settings->saturation() ){
            QHash<QString, QVariant> params;
            params["h"] = 0.0;
            params["s"] = result;
            params["v"] = 0.0;

            KoColorTransformation* transfo = dev->colorSpace()->createColorTransformation("hsv_adjustment", params);
            transfo->transform(m_inkColor.data(), m_inkColor.data(), 1);
        }

        if ( m_settings->opacity() ){
            int opacity = qRound((1.0f + result) * OPACITY_OPAQUE);
            m_inkColor.setOpacity(opacity);
        }
    }
    
    int dx, dy;
    qreal dirt;
    for (int i = 0; i < m_bristles.size(); i++) {
        bristle = &m_bristles[i];

        // let's call that noise from ground to chalk :)
        dirt = drand48();
        if (bristle->distanceCenter() > m_radius || dirt < 0.5) {
            continue;
        }

        dx = qRound(x + bristle->x());
        dy = qRound(y + bristle->y());

        accessor.moveTo(dx, dy);
        memcpy(accessor.rawData(), m_inkColor.data(), pixelSize);
    }
}

