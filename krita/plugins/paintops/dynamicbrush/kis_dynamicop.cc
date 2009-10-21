/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_dynamicop.h"

#include <KoColor.h>

#include <kis_painter.h>
#include <kis_paint_information.h>
#include <kis_types.h>
#include <kis_paint_device.h>

#include "kis_dynamicop_settings.h"
#include "kis_dynamic_shape.h"
#include "kis_dynamic_coloring.h"
#include "kis_dynamic_scattering.h"
#include "kis_dynamic_shape_program.h"
#include "kis_dynamic_coloring_program.h"
#include "kis_dynamic_brush.h"

KisDynamicOp::KisDynamicOp(const KisDynamicOpSettings *settings, KisPainter *painter)
        : KisPaintOp(painter)
        , m_brush(0)
        , m_settings(settings)
{
    Q_ASSERT(settings);
    m_brush = m_settings->createBrush(painter);
    if (m_brush)
        m_brush->startPainting(painter);
}

KisDynamicOp::~KisDynamicOp()
{
    if (m_brush) {
        m_brush->endPainting();
        delete m_brush;
    }
}

void KisDynamicOp::paintAt(const KisPaintInformation& info)
{

    if (!painter()) return;
    if (!painter()->device()) return;
    if (!m_brush) return;

    KisPaintDeviceSP device = painter()->device();


    quint8 origOpacity = painter()->opacity();
    KoColor origColor = painter()->paintColor();

    KisDynamicScattering scatter = m_brush->shapeProgram()->scattering(info);

    double maxDist = scatter.maximumDistance();

    for (int i = 0; i < scatter.count(); i ++) {
        KisPaintInformation localInfo(info);

        localInfo.setPos(localInfo.pos() + QPointF(maxDist *(rand() / (double) RAND_MAX - 0.5), maxDist *(rand() / (double) RAND_MAX - 0.5)));

        KisDynamicShape* dabsrc = m_brush->shape()->clone();
        KisDynamicColoring* coloringsrc = m_brush->coloring()->clone();
        coloringsrc->selectColor(m_brush->coloringProgram()->mix(localInfo));

        m_brush->shapeProgram()->apply(dabsrc, localInfo);
        m_brush->coloringProgram()->apply(coloringsrc, localInfo);
        dabsrc->paintAt(localInfo.pos(), localInfo, coloringsrc);
        delete dabsrc;
        delete coloringsrc;
    }
    painter()->setOpacity(origOpacity);
    painter()->setPaintColor(origColor);
}
