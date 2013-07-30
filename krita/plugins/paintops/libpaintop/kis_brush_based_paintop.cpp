/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_brush_based_paintop.h"
#include "kis_brush.h"
#include "kis_properties_configuration.h"
#include "kis_brush_option.h"
#include <kis_pressure_spacing_option.h>


#include <QImage>
#include <QPainter>

KisBrushBasedPaintOp::KisBrushBasedPaintOp(const KisPropertiesConfiguration* settings, KisPainter* painter)
        : KisPaintOp(painter)
{
    Q_ASSERT(settings);
    KisBrushOption brushOption;
    brushOption.readOptionSetting(settings);
    m_brush = brushOption.brush();

    m_precisionOption.readOptionSetting(settings);
    m_dabCache = new KisDabCache(m_brush);
    m_dabCache->setPrecisionOption(&m_precisionOption);

    m_textureProperties.fillProperties(settings);
    m_dabCache->setTexturePostprocessing(&m_textureProperties);
}

KisBrushBasedPaintOp::~KisBrushBasedPaintOp()
{
    delete m_dabCache;
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(int dabWidth, int dabHeight) const
{
    return effectiveSpacing(dabWidth, dabHeight, 1.0, false);
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(int dabWidth, int dabHeight, const KisPressureSpacingOption &spacingOption, const KisPaintInformation &pi) const
{
    qreal extraSpacingScale = 1.0;
    if (spacingOption.isChecked()) {
        extraSpacingScale = spacingOption.apply(pi);
    }

    return effectiveSpacing(dabWidth, dabHeight, extraSpacingScale, false);
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(int dabWidth, int dabHeight, qreal extraScale, bool isotropicSpacing) const
{
    QPointF spacing;

    if (!isotropicSpacing) {
        spacing = QPointF(dabWidth, dabHeight);
    } else {
        qreal significantDimension = qMax(dabWidth, dabHeight);
        spacing = QPointF(significantDimension, significantDimension);
    }

    spacing *= extraScale * m_brush->spacing();

    return spacing;
}

double KisBrushBasedPaintOp::spacing(double scale) const
{
    // XXX: The spacing should vary as the pressure changes along the line.
    // This is a quick simplification.
    double xSpacing = m_brush->xSpacing(scale);
    double ySpacing = m_brush->ySpacing(scale);

    if (xSpacing < 0.5) {
        xSpacing = 0.5;
    }
    if (ySpacing < 0.5) {
        ySpacing = 0.5;
    }

    if (xSpacing > ySpacing) {
        return xSpacing;
    } else {
        return ySpacing;
    }
}

bool KisBrushBasedPaintOp::canPaint() const
{
    return m_brush != 0;
}
