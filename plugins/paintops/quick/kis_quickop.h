/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef _KIS_QUICKOP_H_
#define _KIS_QUICKOP_H_

#include <QRect>

#include <kis_brush_based_paintop.h>
#include <kis_types.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_spacing_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option.h>
#include <kis_pressure_gradient_option.h>

#include "kis_multipoint_painter.h"

class QPointF;
class KoAbstractGradient;
class KisBrushBasedPaintOpSettings;
class KisPainter;

class KisQuickOp: public KisBrushBasedPaintOp
{
public:
    KisQuickOp(const KisBrushBasedPaintOpSettings* settings, KisPainter* painter, KisNodeSP node, KisImageSP image);
    virtual ~KisQuickOp();

    KisSpacingInformation paintAt(const KisPaintInformation& info);

private:
    KisPressureSizeOption     m_sizeOption;
    KisPressureOpacityOption  m_opacityOption;
    KisPressureSpacingOption  m_spacingOption;
    KisPressureScatterOption  m_scatterOption;

   QTime m_lastPaintTime;
   QVector<KisMultipointPainter::Point> m_points;
   KisMultipointPainter m_multipointPainter;
   KisPaintDeviceSP m_device;
   KoColor m_color;
};

#endif // _KIS_QUICKOP_H_
