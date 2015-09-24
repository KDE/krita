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
#include "kis_properties_configuration.h"
#include "kis_brush_option.h"
#include <kis_pressure_spacing_option.h>

#include <QGlobalStatic>
#include <QImage>
#include <QPainter>

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND

Q_GLOBAL_STATIC(TextBrushInitializationWorkaround, s_instance)


TextBrushInitializationWorkaround *TextBrushInitializationWorkaround::instance() {
    return s_instance;
}

void TextBrushInitializationWorkaround::preinitialize(const KisPropertiesConfiguration *settings) {
    if (KisBrushOption::isTextBrush(settings)) {
        KisBrushOption brushOption;
        brushOption.readOptionSetting(settings);
        m_brush = brushOption.brush();
        m_settings = settings;
    }
    else {
        m_brush = 0;
        m_settings = 0;
    }
}

KisBrushSP TextBrushInitializationWorkaround::tryGetBrush(const KisPropertiesConfiguration *settings) {
    return settings && settings == m_settings ? m_brush : 0;
}

TextBrushInitializationWorkaround::TextBrushInitializationWorkaround()
    : m_settings(0)
{}

TextBrushInitializationWorkaround::~TextBrushInitializationWorkaround()
{}



void KisBrushBasedPaintOp::preinitializeOpStatically(const KisPaintOpSettingsSP settings)
{
    TextBrushInitializationWorkaround::instance()->preinitialize(settings.data());
}

#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */


KisBrushBasedPaintOp::KisBrushBasedPaintOp(const KisPropertiesConfiguration* settings, KisPainter* painter)
    : KisPaintOp(painter)
{
    Q_ASSERT(settings);

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    m_brush =
            TextBrushInitializationWorkaround::instance()->tryGetBrush(settings);
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

    if (!m_brush) {
        KisBrushOption brushOption;
        brushOption.readOptionSetting(settings);
        m_brush = brushOption.brush();
    }

    m_precisionOption.readOptionSetting(settings);
    m_dabCache = new KisDabCache(m_brush);
    m_dabCache->setPrecisionOption(&m_precisionOption);

    m_mirrorOption.readOptionSetting(settings);
    m_dabCache->setMirrorPostprocessing(&m_mirrorOption);

    m_textureProperties.fillProperties(settings);
    m_dabCache->setTexturePostprocessing(&m_textureProperties);
}

KisBrushBasedPaintOp::~KisBrushBasedPaintOp()
{
    delete m_dabCache;
}

bool KisBrushBasedPaintOp::checkSizeTooSmall(qreal scale)
{
    scale *= m_brush->scale();

    return (scale * m_brush->width() < 0.01 || scale * m_brush->height() < 0.01);
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(qreal scale, qreal rotation) const
{
    // we parse dab rotation separately, so don't count it
    QSizeF metric = m_brush->characteristicSize(scale, scale, 0);
    return effectiveSpacing(metric.width(), metric.height(), 1.0, false, rotation);
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(qreal scale, qreal rotation, const KisPressureSpacingOption &spacingOption, const KisPaintInformation &pi) const {
    qreal extraSpacingScale = 1.0;
    if (spacingOption.isChecked()) {
        extraSpacingScale = spacingOption.apply(pi);
    }

    // we parse dab rotation separately, so don't count it
    QSizeF metric = m_brush->characteristicSize(scale, scale, 0);
    return effectiveSpacing(metric.width(), metric.height(), extraSpacingScale, spacingOption.isotropicSpacing(), rotation);
}

inline qreal calcAutoSpacing(qreal value, qreal coeff)
{
    return coeff * (value < 1.0 ? value : sqrt(value));
}

inline QPointF calcAutoSpacing(const QPointF &pt, qreal coeff)
{
    return QPointF(calcAutoSpacing(pt.x(), coeff), calcAutoSpacing(pt.y(), coeff));
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(qreal dabWidth, qreal dabHeight, qreal extraScale, bool isotropicSpacing, qreal rotation) const
{
    QPointF spacing;

    if (!isotropicSpacing) {
        if (m_brush->autoSpacingActive()) {
            spacing = calcAutoSpacing(QPointF(dabWidth, dabHeight), m_brush->autoSpacingCoeff());
        } else {
            spacing = QPointF(dabWidth, dabHeight);
            spacing *= m_brush->spacing();
        }
    }
    else {
        qreal significantDimension = qMax(dabWidth, dabHeight);
        if (m_brush->autoSpacingActive()) {
            significantDimension = calcAutoSpacing(significantDimension, m_brush->autoSpacingCoeff());
        } else {
            significantDimension *= m_brush->spacing();
        }
        spacing = QPointF(significantDimension, significantDimension);
        rotation = 0.0;
    }

    spacing *= extraScale;

    return KisSpacingInformation(spacing, rotation);
}

bool KisBrushBasedPaintOp::canPaint() const
{
    return m_brush != 0;
}
