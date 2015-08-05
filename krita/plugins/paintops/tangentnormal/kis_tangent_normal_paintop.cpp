/*
 *  Copyright (C) 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#include "kis_tangent_normal_paintop.h"

#include <QRect>

#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_brush_based_paintop_settings.h>
#include <kis_cross_device_color_picker.h>
#include <kis_fixed_paint_device.h>

KisTangentNormalPaintOp::KisTangentNormalPaintOp(const KisBrushBasedPaintOpSettings* settings, KisPainter* painter, KisNodeSP node, KisImageSP image):
    KisBrushBasedPaintOp(settings, painter),
    m_opacityOption(node),
    m_tempDev(painter->device()->createCompositionSourceDevice())

{
    Q_UNUSED(image);
    //Init, read settings, etc//
    m_tangentTiltOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_flowOption.readOptionSetting(settings);
    m_spacingOption.readOptionSetting(settings);
    m_softnessOption.readOptionSetting(settings);
    m_sharpnessOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_scatterOption.readOptionSetting(settings);

    m_sizeOption.resetAllSensors();
    m_opacityOption.resetAllSensors();
    m_flowOption.resetAllSensors();
    m_spacingOption.resetAllSensors();
    m_softnessOption.resetAllSensors();
    m_sharpnessOption.resetAllSensors();
    m_rotationOption.resetAllSensors();
    m_scatterOption.resetAllSensors();

    m_dabCache->setSharpnessPostprocessing(&m_sharpnessOption);
    m_rotationOption.applyFanCornersInfo(this);
}

KisTangentNormalPaintOp::~KisTangentNormalPaintOp()
{
    //destroy things here//
}

KisSpacingInformation KisTangentNormalPaintOp::paintAt(const KisPaintInformation& info)
{
    /*
     * For the color, the precision of tilt is only 60x60, and the precision of direction and rotation are 360 and 360*90.
     * You can't get more precise than 8bit. Therefore, we will check if the current space is RGB,
     * if so we request a profile with that space and 8bit bit depth, if not, just sRGB
     */
    KoColor currentColor = painter()->paintColor();
    QString currentSpace = currentColor.colorSpace()->colorModelId().id();
    const KoColorSpace* rgbColorSpace = KoColorSpaceRegistry::instance()->rgb8();
    if (currentSpace != "RGBA") {
	rgbColorSpace = KoColorSpaceRegistry::instance()->rgb8();
    } else {
	QString bit = rgbColorSpace->colorDepthId().id();//let Krita tell you what the bit depth string is.
	rgbColorSpace = KoColorSpaceRegistry::instance()->colorSpace("RGBA", bit, currentColor.profile() );
    }

    quint8 data[4];

    data[0] = 255;//blue
    data[1] = 128;//green
    data[2] = 128;//red
    data[3] = 255;//alpha, leave alone.

    quint8 r, g, b;
    m_tangentTiltOption.apply(info, &r, &g, &b);

    data[0] = b;//blue
    data[1] = g;//green
    data[2] = r;//red

    KoColor color(data, rgbColorSpace);//Should be default RGB(0.5,0.5,1.0)
    //draw stuff here, return kisspacinginformation.
    KisBrushSP brush = m_brush;

    if (!painter()->device() || !brush || !brush->canPaintFor(info)) {
        return KisSpacingInformation(1.0);
    }

    qreal scale    = m_sizeOption.apply(info);
    qreal rotation = m_rotationOption.apply(info);

    if (checkSizeTooSmall(scale)) return KisSpacingInformation();

    setCurrentScale(scale);
    setCurrentRotation(rotation);

    QPointF cursorPos =
        m_scatterOption.apply(info,
                              brush->maskWidth(scale, rotation, 0, 0, info),
                              brush->maskHeight(scale, rotation, 0, 0, info));

    m_maskDab =
        m_dabCache->fetchDab(rgbColorSpace, color, cursorPos,
                             scale, scale, rotation,
                             info, m_softnessOption.apply(info),
                             &m_dstDabRect);

    if (m_dstDabRect.isEmpty()) return KisSpacingInformation(1.0);

    QRect dabRect = m_maskDab->bounds();

    // sanity check
    Q_ASSERT(m_dstDabRect.size() == dabRect.size());
    Q_UNUSED(dabRect);

    quint8  oldOpacity = painter()->opacity();
    QString oldCompositeOpId = painter()->compositeOp()->id();


    m_opacityOption.setFlow(m_flowOption.apply(info));
    m_opacityOption.apply(painter(), info);
    //paint with the default color? Copied this from color smudge.//
    //painter()->setCompositeOp(COMPOSITE_COPY);
    //painter()->fill(0, 0, m_dstDabRect.width(), m_dstDabRect.height(), color);
    painter()->bltFixed(m_dstDabRect.topLeft(), m_maskDab, m_maskDab->bounds());
    painter()->renderMirrorMaskSafe(m_dstDabRect, m_maskDab, !m_dabCache->needSeparateOriginal());

    // restore orginal opacity and composite mode values
    painter()->setOpacity(oldOpacity);
    painter()->setCompositeOp(oldCompositeOpId);

    return effectiveSpacing(scale, rotation,
                            m_spacingOption, info);
}

void KisTangentNormalPaintOp::paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, KisDistanceInformation *currentDistance)
{
    if (m_sharpnessOption.isChecked() && m_brush && (m_brush->width() == 1) && (m_brush->height() == 1)) {

        if (!m_lineCacheDevice) {
            m_lineCacheDevice = m_tempDev;
        }
        else {
            m_lineCacheDevice->clear();
        }

        KisPainter p(m_lineCacheDevice);
	KoColor currentColor = painter()->paintColor();
	QString currentSpace = currentColor.colorSpace()->colorModelId().id();
	const KoColorSpace* rgbColorSpace = KoColorSpaceRegistry::instance()->rgb8();
	if (currentSpace != "RGBA") {
	    rgbColorSpace = KoColorSpaceRegistry::instance()->rgb8();
	} else {
	    QString bit = rgbColorSpace->colorDepthId().id();//let Krita tell you what the bit depth string is.
	    rgbColorSpace = KoColorSpaceRegistry::instance()->colorSpace("RGBA", bit, currentColor.profile() );
	}
	quint8 data[4];

	data[0] = 255;//blue
	data[1] = 128;//green
	data[2] = 128;//red
	data[3] = 255;//alpha, leave alone.

	quint8 r, g, b;
	m_tangentTiltOption.apply(pi2, &r, &g, &b);

	data[0] = b;//blue
	data[1] = g;//green
	data[2] = r;//red

	KoColor color(data, rgbColorSpace);
        p.setPaintColor(color);
        p.drawDDALine(pi1.pos(), pi2.pos());

        QRect rc = m_lineCacheDevice->extent();
        painter()->bitBlt(rc.x(), rc.y(), m_lineCacheDevice, rc.x(), rc.y(), rc.width(), rc.height());
	painter()->renderMirrorMask(rc, m_lineCacheDevice);
    }
    else {
        KisPaintOp::paintLine(pi1, pi2, currentDistance);
    }
}
