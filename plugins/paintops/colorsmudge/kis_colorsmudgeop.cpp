/*
 *  Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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

#include "kis_colorsmudgeop.h"

#include <cmath>
#include <memory>
#include <QRect>

#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoColorProfile.h>
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
#include <kis_lod_transform.h>
#include <kis_spacing_information.h>
#include <KoColorModelStandardIds.h>

KisColorSmudgeOp::KisColorSmudgeOp(const KisPaintOpSettingsSP settings, KisPainter* painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
    , m_firstRun(true)
    , m_image(image)
    , m_precisePainterWrapper(painter->device())
    , m_tempDev(m_precisePainterWrapper.createPreciseCompositionSourceDevice())
    , m_backgroundPainter(new KisPainter(m_tempDev))
    , m_smudgePainter(new KisPainter(m_tempDev))
    , m_colorRatePainter(new KisPainter(m_tempDev))
    , m_finalPainter(new KisPainter(m_precisePainterWrapper.preciseDevice()))
    , m_smudgeRateOption()
    , m_colorRateOption("ColorRate", KisPaintOpOption::GENERAL, false)
    , m_smudgeRadiusOption()
{
    Q_UNUSED(node);

    Q_ASSERT(settings);
    Q_ASSERT(painter);
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_spacingOption.readOptionSetting(settings);
    m_smudgeRateOption.readOptionSetting(settings);
    m_colorRateOption.readOptionSetting(settings);
    m_smudgeRadiusOption.readOptionSetting(settings);
    m_overlayModeOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_scatterOption.readOptionSetting(settings);
    m_gradientOption.readOptionSetting(settings);

    m_sizeOption.resetAllSensors();
    m_opacityOption.resetAllSensors();
    m_spacingOption.resetAllSensors();
    m_smudgeRateOption.resetAllSensors();
    m_colorRateOption.resetAllSensors();
    m_smudgeRadiusOption.resetAllSensors();
    m_rotationOption.resetAllSensors();
    m_scatterOption.resetAllSensors();
    m_gradientOption.resetAllSensors();

    m_gradient = painter->gradient();

    m_backgroundPainter->setCompositeOp(COMPOSITE_COPY);
    // Smudge Painter works in default COMPOSITE_OVER mode
    m_colorRatePainter->setCompositeOp(painter->compositeOp()->id());

    m_finalPainter->setCompositeOp(COMPOSITE_COPY);
    m_finalPainter->setSelection(painter->selection());
    m_finalPainter->setChannelFlags(painter->channelFlags());
    m_finalPainter->copyMirrorInformationFrom(painter);

    m_paintColor = painter->paintColor().convertedTo(m_tempDev->colorSpace());
    m_preciseColorRateCompositeOp =
        m_tempDev->colorSpace()->compositeOp(m_colorRatePainter->compositeOp()->id());

    m_hsvOptions.append(KisPressureHSVOption::createHueOption());
    m_hsvOptions.append(KisPressureHSVOption::createSaturationOption());
    m_hsvOptions.append(KisPressureHSVOption::createValueOption());

    Q_FOREACH (KisPressureHSVOption * option, m_hsvOptions) {
        option->readOptionSetting(settings);
        option->resetAllSensors();
        if (option->isChecked() && !m_hsvTransform) {
            m_hsvTransform = m_paintColor.colorSpace()->createColorTransformation("hsv_adjustment", QHash<QString, QVariant>());
        }
    }
    m_rotationOption.applyFanCornersInfo(this);

    if (m_overlayModeOption.isChecked() && m_image && m_image->projection()){
        m_preciseImageDeviceWrapper.reset(new KisPrecisePaintDeviceWrapper(m_image->projection()));
    }
}

KisColorSmudgeOp::~KisColorSmudgeOp()
{
    qDeleteAll(m_hsvOptions);
    delete m_hsvTransform;
}

void KisColorSmudgeOp::updateMask(const KisPaintInformation& info, double scale, double rotation, const QPointF &cursorPoint)
{
    static const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    m_maskDab = m_dabCache->fetchDab(cs,
                                     color,
                                     cursorPoint,
                                     KisDabShape(scale, 1.0, rotation),
                                     info,
                                     1.0,
                                     &m_dstDabRect);

    // sanity check
    KIS_ASSERT_RECOVER_NOOP(m_dstDabRect.size() == m_maskDab->bounds().size());
}

inline void KisColorSmudgeOp::getTopLeftAligned(const QPointF &pos, const QPointF &hotSpot, qint32 *x, qint32 *y)
{
    QPointF topLeft = pos - hotSpot;

    qreal xFraction, yFraction; // will not be used
    splitCoordinate(topLeft.x(), x, &xFraction);
    splitCoordinate(topLeft.y(), y, &yFraction);
}

KisSpacingInformation KisColorSmudgeOp::paintAt(const KisPaintInformation& info)
{
    KisBrushSP brush = m_brush;
    const bool useDullingMode = m_smudgeRateOption.getMode() == KisSmudgeOption::DULLING_MODE;

    /* This is a fix for dulling + overlay + paint,
     * this should allow the image to composite paint addition effects correctly
     * while also respecting overlay mode. */
    bool useAlternatePrecisionSource = (m_overlayModeOption.isChecked() &&
                                        useDullingMode &&
                                        m_preciseImageDeviceWrapper!= nullptr);

    KisPrecisePaintDeviceWrapper &activeWrapper = useAlternatePrecisionSource ? *m_preciseImageDeviceWrapper :
                                                                                 m_precisePainterWrapper;

    // Simple error catching
    if (!painter()->device() || !brush || !brush->canPaintFor(info)) {
        return KisSpacingInformation(1.0);
    }

    if (m_smudgeRateOption.getMode() == KisSmudgeOption::SMEARING_MODE) {
        /**
        * Disable handling of the subpixel precision. In the smudge op we
        * should read from the aligned areas of the image, so having
        * additional internal offsets, created by the subpixel precision,
        * will worsen the quality (at least because
        * QRectF(m_dstDabRect).center() will not point to the real center
        * of the brush anymore).
        * Of course, this only really matters with smearing_mode (bug:327235),
        * and you only notice the lack of subpixel precision in the dulling methods.
        */
        m_dabCache->disableSubpixelPrecision();
    }

#if 0
    //if precision
    KoColor colorSpaceChanger = painter()->paintColor();
    const KoColorSpace* preciseColorSpace = colorSpaceChanger.colorSpace();
    if (colorSpaceChanger.colorSpace()->colorDepthId().id() == "U8") {
    preciseColorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceChanger.colorSpace()->colorModelId().id(), "U16", colorSpaceChanger.profile() );
        colorSpaceChanger.convertTo(preciseColorSpace);
    }
    painter()->setPaintColor(colorSpaceChanger);
#endif

    // get the scaling factor calculated by the size option
    qreal scale = m_sizeOption.apply(info);
    scale *= KisLodTransform::lodToScale(painter()->device());
    qreal rotation = m_rotationOption.apply(info);
    if (checkSizeTooSmall(scale)) return KisSpacingInformation();

    KisDabShape shape(scale, 1.0, rotation);

    QPointF scatteredPos =
        m_scatterOption.apply(info,
                              brush->maskWidth(shape, 0, 0, info),
                              brush->maskHeight(shape, 0, 0, info));

    QPointF hotSpot = brush->hotSpot(shape, info);

    /**
     * Update the brush mask.
     *
     * Upon leaving the function:
     *   o m_maskDab stores the new mask
     *   o m_maskBounds stores the extents of the mask paint device
     *   o m_dstDabRect stores the destination rect where the mask is going
     *     to be written to
     */
    updateMask(info, scale, rotation, scatteredPos);

    QPointF newCenterPos = QRectF(m_dstDabRect).center();
    /**
     * Save the center of the current dab to know where to read the
     * data during the next pass. We do not save scatteredPos here,
     * because it may differ slightly from the real center of the
     * brush (due to rounding effects), which will result in a
     * really weird quality.
     */
    QRect srcDabRect = m_dstDabRect.translated((m_lastPaintPos - newCenterPos).toPoint());

    m_lastPaintPos = newCenterPos;

    KisSpacingInformation spacingInfo =
        effectiveSpacing(scale, rotation,
                         m_spacingOption, info);

    if (m_firstRun) {
        m_firstRun = false;
        return spacingInfo;
    }

    const qreal fpOpacity = (qreal(painter()->opacity()) / 255.0) * m_opacityOption.getOpacityf(info);

    if (m_image && m_overlayModeOption.isChecked()) {
        m_image->blockUpdates();
        m_backgroundPainter->bitBlt(QPoint(), m_image->projection(), srcDabRect);
        m_image->unblockUpdates();
    }
    else {
        // IMPORTANT: Clear the temporary painting device to transparent black.
        //            It will only clear the extents of the brush.
        m_tempDev->clear(QRect(QPoint(), m_dstDabRect.size()));
    }

    // stored in the color space of the paintColor
    KoColor dullingFillColor = m_paintColor;

    QPoint canvasLocalSamplePoint = (srcDabRect.topLeft() + hotSpot).toPoint();

    if (!useDullingMode) {
        activeWrapper.readRect(srcDabRect);
        m_smudgePainter->bitBlt(QPoint(), activeWrapper.preciseDevice(), srcDabRect);
    } else {
        if (m_smudgeRadiusOption.isChecked()) {
            const qreal effectiveSize = 0.5 * (m_dstDabRect.width() + m_dstDabRect.height());

            const QRect sampleRect = m_smudgeRadiusOption.sampleRect(info, effectiveSize, canvasLocalSamplePoint);
            activeWrapper.readRect(sampleRect);

            m_smudgeRadiusOption.apply(&dullingFillColor, info, effectiveSize, canvasLocalSamplePoint.x(), canvasLocalSamplePoint.y(), activeWrapper.preciseDevice());
            KIS_SAFE_ASSERT_RECOVER_NOOP(*dullingFillColor.colorSpace() == *m_tempDev->colorSpace());
        } else {
            // get the pixel on the canvas that lies beneath the hot spot
            // of the dab and fill  the temporary paint device with that color
            activeWrapper.readRect(QRect(canvasLocalSamplePoint, QSize(1,1)));
            KisCrossDeviceColorPickerInt colorPicker(activeWrapper.preciseDevice(), dullingFillColor);
            colorPicker.pickColor(canvasLocalSamplePoint.x(), canvasLocalSamplePoint.y(), dullingFillColor.data());
            KIS_SAFE_ASSERT_RECOVER_NOOP(*dullingFillColor.colorSpace() == *m_tempDev->colorSpace());
        }
    }

    // if the user selected the color smudge option,
    // we will mix some color into the temporary painting device (m_tempDev)
    if (m_colorRateOption.isChecked()) {
        // this will apply the opacity (selected by the user) to copyPainter
        // (but fit the rate inbetween the range 0.0 to (1.0-SmudgeRate))
        qreal maxColorRate = qMax<qreal>(1.0 - m_smudgeRateOption.getRate(), 0.2);
        m_colorRateOption.apply(*m_colorRatePainter, info, 0.0, maxColorRate, fpOpacity);

        // paint a rectangle with the current color (foreground color)
        // or a gradient color (if enabled)
        // into the temporary painting device and use the user selected
        // composite mode
        KoColor color = m_paintColor;
        m_gradientOption.apply(color, m_gradient, info);
        if (m_hsvTransform) {
            Q_FOREACH (KisPressureHSVOption * option, m_hsvOptions) {
                option->apply(m_hsvTransform, info);
            }
            m_hsvTransform->transform(color.data(), color.data(), 1);
        }

        if (!useDullingMode) {
            KIS_SAFE_ASSERT_RECOVER(*m_colorRatePainter->device()->colorSpace() == *color.colorSpace()) {
                color.convertTo(m_colorRatePainter->device()->colorSpace());
            }

            m_colorRatePainter->fill(0, 0, m_dstDabRect.width(), m_dstDabRect.height(), color);
        } else {
            KIS_SAFE_ASSERT_RECOVER(*dullingFillColor.colorSpace() == *color.colorSpace()) {
                color.convertTo(dullingFillColor.colorSpace());
            }
            KIS_SAFE_ASSERT_RECOVER_NOOP(*dullingFillColor.colorSpace() == *m_tempDev->colorSpace());
            m_preciseColorRateCompositeOp->composite(dullingFillColor.data(), 0,
                                                     color.data(), 0,
                                                     0, 0,
                                                     1, 1,
                                                     m_colorRatePainter->opacity());
        }
    }

    if (useDullingMode) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(*dullingFillColor.colorSpace() == *m_tempDev->colorSpace());
        m_tempDev->fill(QRect(0, 0, m_dstDabRect.width(), m_dstDabRect.height()), dullingFillColor);
    }

    m_precisePainterWrapper.readRects(m_finalPainter->calculateAllMirroredRects(m_dstDabRect));

    // if color is disabled (only smudge) and "overlay mode" is enabled
    // then first blit the region under the brush from the image projection
    // to the painting device to prevent a rapid build up of alpha value
    // if the color to be smudged is semi transparent.
    if (m_image && m_overlayModeOption.isChecked() && !m_colorRateOption.isChecked()) {
        m_finalPainter->setOpacity(OPACITY_OPAQUE_U8);
        m_image->blockUpdates();
        // TODO: check if this code is correct in mirrored mode! Technically, the
        //       painter renders the mirrored dab only, so we should also prepare
        //       the overlay for it in all the places.
        m_finalPainter->bitBlt(m_dstDabRect.topLeft(), m_image->projection(), m_dstDabRect);
        m_image->unblockUpdates();
    }

    // set opacity calculated by the rate option
    m_smudgeRateOption.apply(*m_finalPainter, info, 0.0, 1.0, fpOpacity);

    // then blit the temporary painting device on the canvas at the current brush position
    // the alpha mask (maskDab) will be used here to only blit the pixels that are in the area (shape) of the brush
    m_finalPainter->bitBltWithFixedSelection(m_dstDabRect.x(), m_dstDabRect.y(), m_tempDev, m_maskDab, m_dstDabRect.width(), m_dstDabRect.height());
    m_finalPainter->renderMirrorMaskSafe(m_dstDabRect, m_tempDev, 0, 0, m_maskDab, !m_dabCache->needSeparateOriginal());

    const QVector<QRect> dirtyRects = m_finalPainter->takeDirtyRegion();
    m_precisePainterWrapper.writeRects(dirtyRects);
    painter()->addDirtyRects(dirtyRects);

    return spacingInfo;
}

KisSpacingInformation KisColorSmudgeOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    const qreal scale = m_sizeOption.apply(info) * KisLodTransform::lodToScale(painter()->device());
    const qreal rotation = m_rotationOption.apply(info);
    return effectiveSpacing(scale, rotation, m_spacingOption, info);
}
