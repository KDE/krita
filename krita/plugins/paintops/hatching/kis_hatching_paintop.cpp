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
#include <kis_paintop.h>
#include <kis_brush_based_paintop.h>
#include <kis_paint_information.h>
#include <kis_fixed_paint_device.h>
#include <kis_pressure_opacity_option.h>

#include <KoColorSpaceRegistry.h>

KisHatchingPaintOp::KisHatchingPaintOp(const KisHatchingPaintOpSettings *settings, KisPainter * painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
    , m_image(image)
{
    Q_UNUSED(node);

    m_settings = new KisHatchingPaintOpSettings();
    settings->initializeTwin(m_settings);

    m_hatchingBrush = new HatchingBrush(m_settings);

    m_crosshatchingOption.readOptionSetting(settings);
    m_separationOption.readOptionSetting(settings);
    m_thicknessOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
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
    if (!painter()->device()) return 1;
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
    if (!brush) return 1;
    if (!brush->canPaintFor(info)) return 1;

    //SENSOR-depending settings
    m_settings->crosshatchingsensorvalue = m_crosshatchingOption.apply(info);
    m_settings->separationsensorvalue = m_separationOption.apply(info);
    m_settings->thicknesssensorvalue = m_thicknessOption.apply(info);

    double scale = m_sizeOption.apply(info);
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return 1.0;

    setCurrentScale(scale);

    quint8 origOpacity = m_opacityOption.apply(painter(), info);

    /*----Fetch the Dab----*/
    static const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    QRect dstRect;
    KisFixedPaintDeviceSP maskDab =
        m_dabCache->fetchDab(cs, color, info.pos(),
                             scale, scale, 0.0,
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

    // Trick for moire pattern to look better
    bool donotbasehatch = false;

    /* If block describing how to stack hatching passes to generate
    crosshatching according to user specifications */
    if (m_settings->enabledcurvecrosshatching) {
        if (m_settings->perpendicular) {
            if (m_settings->crosshatchingsensorvalue > 0.5)
                m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(90), painter()->paintColor());
        }
        else if (m_settings->minusthenplus) {
            if (m_settings->crosshatchingsensorvalue > 0.33)
                m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(-45), painter()->paintColor());
            if (m_settings->crosshatchingsensorvalue > 0.67)
                m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(45), painter()->paintColor());
        }
        else if (m_settings->plusthenminus) {
            if (m_settings->crosshatchingsensorvalue > 0.33)
                m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(45), painter()->paintColor());
            if (m_settings->crosshatchingsensorvalue > 0.67)
                m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(-45), painter()->paintColor());
        }
        else if (m_settings->moirepattern) {
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle((m_settings->crosshatchingsensorvalue) * 180), painter()->paintColor());
            donotbasehatch = true;
        }
    } else {
        if (m_settings->perpendicular) {
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(90), painter()->paintColor());
        }
        else if (m_settings->minusthenplus) {
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(-45), painter()->paintColor());
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(45), painter()->paintColor());
        }
        else if (m_settings->plusthenminus) {
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(45), painter()->paintColor());
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(-45), painter()->paintColor());
        }
        else if (m_settings->moirepattern) {
            m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, spinAngle(-10), painter()->paintColor());
        }
    }

    if (!donotbasehatch)
        m_hatchingBrush->hatch(m_hatchedDab, x, y, sw, sh, m_settings->angle, painter()->paintColor());

    // The most important line, the one that paints to the screen.
    painter()->bitBltWithFixedSelection(x, y, m_hatchedDab, maskDab, sw, sh);
    painter()->renderMirrorMaskSafe(QRect(QPoint(x, y), QSize(sw, sh)), m_hatchedDab, 0, 0, maskDab,
                                    !m_dabCache->needSeparateOriginal());
    painter()->setOpacity(origOpacity);

    return effectiveSpacing(scale, 0.0);
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
