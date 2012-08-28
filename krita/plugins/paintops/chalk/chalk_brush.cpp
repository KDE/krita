/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#if defined(_WIN32) || defined(_WIN64)
#include <stdlib.h>
#define srand48 srand
inline double drand48() {
    return double(rand()) / RAND_MAX;
}
#endif

#include "chalk_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>

#include "kis_random_accessor_ng.h"
#include <cmath>
#include <ctime>


ChalkBrush::ChalkBrush(const ChalkProperties* properties, KoColorTransformation* transformation)
{
    m_transfo = transformation;
    if (m_transfo) {
        m_transfo->setParameter(m_transfo->parameterId("h"), 0.0);
        m_saturationId = m_transfo->parameterId("s"); // cache for later usage
        m_transfo->setParameter(m_transfo->parameterId("v"), 0.0);
    }
    else {
        m_saturationId = -1;
    }
    

    m_counter = 0;
    m_properties = properties;
    srand48(time(0));
}


ChalkBrush::~ChalkBrush()
{
    delete m_transfo;
}


void ChalkBrush::paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color)
{
    m_inkColor = color;
    m_counter++;

    qint32 pixelSize = dev->colorSpace()->pixelSize();
    KisRandomAccessorSP accessor = dev->createRandomAccessorNG((int)x, (int)y);

    qreal result;
    if (m_properties->inkDepletion) {
        //count decrementing of saturation and opacity
        result = log((qreal)m_counter);
        result = -(result * 10) / 100.0;

        if (m_properties->useSaturation) {
            if (m_transfo) {
                m_transfo->setParameter(m_saturationId,result);
                m_transfo->transform(m_inkColor.data(), m_inkColor.data(), 1);
            }
            
        }

        if (m_properties->useOpacity) {
            qreal opacity = (1.0f + result);
            m_inkColor.setOpacity(opacity);
        }
    }
    
    int pixelX, pixelY;
    int radiusSquared = m_properties->radius * m_properties->radius;
    double dirtThreshold = 0.5;
    
    for (int by = -m_properties->radius; by <= m_properties->radius; by++) {
        int bySquared = by*by;
        for (int bx = -m_properties->radius; bx <= m_properties->radius; bx++) {
            // let's call that noise from ground to chalk :)
            if ( ((bx*bx + bySquared) > radiusSquared) || drand48() < dirtThreshold) {
                continue;
            }

            pixelX = qRound(x + bx);
            pixelY = qRound(y + by);

            accessor->moveTo(pixelX, pixelY);
            memcpy(accessor->rawData(), m_inkColor.data(), pixelSize);
        }
    }
}

