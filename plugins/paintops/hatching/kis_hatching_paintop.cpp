/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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
#include <brushengine/kis_paintop.h>
#include <kis_brush_based_paintop.h>
#include <brushengine/kis_paint_information.h>
#include <kis_fixed_paint_device.h>
#include <kis_pressure_opacity_option.h>
#include <kis_lod_transform.h>
#include <kis_spacing_information.h>



#include <KoColorSpaceRegistry.h>

KisHatchingPaintOp::KisHatchingPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
{
    Q_UNUSED(node);

    m_settings = new KisHatchingPaintOpSettings();
    static_cast<const KisHatchingPaintOpSettings*>(settings.data())->initializeTwin(m_settings);

    m_hatchingBrush = new HatchingBrush(m_settings);

    m_angleOption.readOptionSetting(settings);
    m_crosshatchingOption.readOptionSetting(settings);
    m_separationOption.readOptionSetting(settings);
    m_thicknessOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
    m_angleOption.resetAllSensors();
    m_crosshatchingOption.resetAllSensors();
    m_separationOption.resetAllSensors();
    m_thicknessOption.resetAllSensors();
    m_opacityOption.resetAllSensors();
    m_sizeOption.resetAllSensors();
}

KisHatchingPaintOp::~KisHatchingPaintOp()
{
    delete m_hatchingBrush;
}

KisSpacingInformation KisHatchingPaintOp::paintAt(const KisPaintInformation& info)
{
    //------START SIMPLE ERROR CATCHING-------
    if (!painter()->device()) return KisSpacingInformation(1.0);

    if (!m_hatchedDab)
        m_hatchedDab = source()->createCompositionSourceDevice();
    else
        m_hatchedDab->clear();

    //Simple convenience renaming, I'm thinking of removing these inherited quirks
    KisBrushSP brush = m_brush;
    KisPaintDeviceSP device = painter()->device();

    //Macro to catch errors
    Q_ASSERT(brush);

    //----------SIMPLE error catching code, maybe it's not even needed------
    if (!brush) return KisSpacingInformation(1.0);
    if (!brush->canPaintFor(info)) return KisSpacingInformation(1.0);

    //SENSOR-depending settings
    m_settings->anglesensorvalue = m_angleOption.apply(info);
    m_settings->crosshatchingsensorvalue = m_crosshatchingOption.apply(info);
    m_settings->separationsensorvalue = m_separationOption.apply(info);
    m_settings->thicknesssensorvalue = m_thicknessOption.apply(info);

    const qreal additionalScale = KisLodTransform::lodToScale(painter()->device());
    const double scale = additionalScale * m_sizeOption.apply(info);
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return KisSpacingInformation(1.0);
    KisDabShape shape(scale, 1.0, 0.0);

    quint8 origOpacity = m_opacityOption.apply(painter(), info);

    /*----Fetch the Dab----*/
    static const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    QRect dstRect;
    KisFixedPaintDeviceSP maskDab =
        m_dabCache->fetchDab(cs, color, info.pos(),
                             shape,
                             info, 1.0, &dstRect);

    // sanity check
    KIS_ASSERT_RECOVER_NOOP(dstRect.size() == maskDab->bounds().size());

    /*-----Convenient renaming for the limits of the maskDab, this will be used
    to hatch a dab of just the right size------*/
    qint32 x, y, sw, sh;
    dstRect.getRect(&x, &y, &sw, &sh);

    //------This If_block pre-fills the future m_hatchedDab with a pretty backgroundColor
    if (m_settings->opaquebackground) {
        KoColor aersh = painter()->backgroundColor();
        m_hatchedDab->fill(0, 0, (sw - 1), (sh - 1), aersh.data()); //this plus yellow background = french fry brush
    }

    /* If block describing how to stack hatching passes to generate
    crosshatching according to user specifications */
    if (m_settings->enabledcurvecrosshatching) {
        if (m_settings->perpendicular) {
            if (m_settings->crosshatchingsensorvalue > 0.5)
                m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(90), painter()->paintColor(), additionalScale);
        }
        else if (m_settings->minusthenplus) {
            if (m_settings->crosshatchingsensorvalue > 0.33)
                m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(-45), painter()->paintColor(), additionalScale);
            if (m_settings->crosshatchingsensorvalue > 0.67)
                m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(45), painter()->paintColor(), additionalScale);
        }
        else if (m_settings->plusthenminus) {
            if (m_settings->crosshatchingsensorvalue > 0.33)
                m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(45), painter()->paintColor(), additionalScale);
            if (m_settings->crosshatchingsensorvalue > 0.67)
                m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(-45), painter()->paintColor(), additionalScale);
        }
        else if (m_settings->moirepattern) {
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle((m_settings->crosshatchingsensorvalue) * 360), painter()->paintColor(), additionalScale);
        }
    } else {
        if (m_settings->perpendicular) {
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(90), painter()->paintColor(), additionalScale);
        }
        else if (m_settings->minusthenplus) {
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(-45), painter()->paintColor(), additionalScale);
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(45), painter()->paintColor(), additionalScale);
        }
        else if (m_settings->plusthenminus) {
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(45), painter()->paintColor(), additionalScale);
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(-45), painter()->paintColor(), additionalScale);
        }
        else if (m_settings->moirepattern) {
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(-10), painter()->paintColor(), additionalScale);
        }
    }

    if (m_settings->enabledcurveangle)
      m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle((m_settings->anglesensorvalue)*360+m_settings->angle), painter()->paintColor(), additionalScale);

    // The base hatch... unless moiré or angle
    if (!m_settings->moirepattern && !m_settings->enabledcurveangle)
        m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, m_settings->angle, painter()->paintColor(), additionalScale);


    // The most important line, the one that paints to the screen.
    painter()->bitBltWithFixedSelection(x, y, m_hatchedDab, maskDab, sw, sh);
    painter()->renderMirrorMaskSafe(QRect(QPoint(x, y), QSize(sw, sh)), m_hatchedDab, 0, 0, maskDab,
                                    !m_dabCache->needSeparateOriginal());
    painter()->setOpacity(origOpacity);

    return effectiveSpacing(scale);
}

KisSpacingInformation KisHatchingPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    const qreal scale = KisLodTransform::lodToScale(painter()->device()) * m_sizeOption.apply(info);
    return effectiveSpacing(scale);
}

double KisHatchingPaintOp::spinAngle(double spin)
{
    double tempangle = m_settings->angle + spin;
    qint8 factor = 1;

    if (tempangle < 0)
        factor = -1;

    tempangle = fabs(fmod(tempangle, 180));

    if ((tempangle >= 0) && (tempangle <= 90))
        return factor * tempangle;
    else if ((tempangle > 90) && (tempangle <= 180))
        return factor * -(180 - tempangle);

    return 0;   // this should never be executed except if NAN
}
