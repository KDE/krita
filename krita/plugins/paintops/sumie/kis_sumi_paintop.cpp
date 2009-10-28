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

#include "kis_sumi_paintop.h"
#include "kis_sumi_paintop_settings.h"

#include <cmath>

#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>

#include "kis_paint_device.h"
#include "kis_painter.h"
#include <kis_vec.h>

#include "brush.h"
#include "brush_shape.h"

#ifdef BENCHMARK
    #include <QTime>
#endif


KisSumiPaintOp::KisSumiPaintOp(const
                               KisSumiPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
        , m_settings(settings)
        , m_image(image)
        , newStrokeFlag(true)

{
    Q_ASSERT(settings);
    BrushShape brushShape;

    if (settings->brushDimension() == 1) {
        brushShape.fromLine(settings->radius(), settings->sigma());
        brushShape.tresholdBristles(0.1);
    } else if (settings->brushDimension() == 2) {
        brushShape.fromGaussian(settings->radius(), settings->sigma());
        brushShape.tresholdBristles(0.1);
    } else {
        Q_ASSERT(false);
    }

    m_brush.setBrushShape(brushShape);

    m_brush.enableMousePressure(settings->mousePressure());

    m_brush.setInkDepletion(settings->curve());
    m_brush.setInkColor(painter->paintColor());

    m_brush.setScale(settings->scaleFactor());
    m_brush.setRandom(settings->randomFactor());
    m_brush.setShear(settings->shearFactor());

    m_brush.enableWeights(settings->useWeights());
    m_brush.enableOpacity(settings->useOpacity());
    m_brush.enableSaturation(settings->useSaturation());

    if (settings->useWeights()) {
        // TODO : improve the way the weights can be set..
        m_brush.setBristleInkAmountWeight(settings->bristleInkAmountWeight() / 100.0);
        m_brush.setBristleLengthWeight(settings->bristleLengthWeight() / 100.0);
        m_brush.setInkDepletionWeight(settings->inkDepletionWeight() / 100.0);
        m_brush.setPressureWeight(settings->pressureWeight() / 100.0);
    }

    if (!settings->node()) {
        m_dev = 0;
    } else {
        m_dev = settings->node()->paintDevice();
    }

#ifdef BENCHMARK
    m_count = m_total = 0;
#endif


}

KisSumiPaintOp::~KisSumiPaintOp()
{
}

void KisSumiPaintOp::paintAt(const KisPaintInformation& info)
{
    Q_UNUSED(info);
}


double KisSumiPaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, double savedDist)
{
#ifdef BENCHMARK
    QTime time;
    time.start();
#endif

    Q_UNUSED(savedDist);

    if (!painter()) return 0;

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    } else {
        m_dab->clear();
    }

    m_brush.paintLine(m_dab, m_dev, pi1, pi2);

    //QRect rc = m_dab->exactBounds();
    QRect rc = m_dab->extent();
    painter()->bitBlt(rc.topLeft(), m_dab, rc);


#ifdef BENCHMARK
    int msec = time.elapsed();
    kDebug() << msec << " ms/dab " << "[average: " << m_total / (qreal)m_count << "]";
    m_total += msec;
    m_count++;
#endif

    KisVector2D end = toKisVector2D(pi2.pos());
    KisVector2D start = toKisVector2D(pi1.pos());
    KisVector2D dragVec = end - start;
    return  dragVec.norm();
}
