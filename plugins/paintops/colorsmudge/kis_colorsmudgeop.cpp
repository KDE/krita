/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_colorsmudgeop.h"

#include <QRect>

#include <KoColor.h>

#include <kis_brush.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_fixed_paint_device.h>
#include <kis_lod_transform.h>
#include <kis_spacing_information.h>
#include "kis_paintop_plugin_utils.h"

#include "KisInterstrokeData.h"
#include "KisInterstrokeDataFactory.h"

#include "kis_brush_option.h"

#include "KisColorSmudgeInterstrokeData.h"
#include "KisColorSmudgeStrategyLightness.h"
#include "KisColorSmudgeStrategyWithOverlay.h"
#include "KisColorSmudgeStrategyMask.h"
#include "KisColorSmudgeStrategyStamp.h"
#include "KisColorSmudgeStrategyMaskLegacy.h"

struct ColorSmudgeInterstrokeDataFactory : public KisInterstrokeDataFactory
{
    bool isCompatible(KisInterstrokeData *data) override {
        KisColorSmudgeInterstrokeData *colorSmudgeData =
            dynamic_cast<KisColorSmudgeInterstrokeData*>(data);

        return colorSmudgeData;
    }

    KisInterstrokeData * create(KisPaintDeviceSP device) override {
        KisColorSmudgeInterstrokeData *data = new KisColorSmudgeInterstrokeData(device);
        return data;
    }
};

KisColorSmudgeOp::KisColorSmudgeOp(const KisPaintOpSettingsSP settings, KisPainter* painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
    , m_firstRun(true)
    , m_smudgeRateOption()
    , m_colorRateOption("ColorRate", KisPaintOpOption::GENERAL, false)
    , m_smudgeRadiusOption()
{
    Q_UNUSED(node);

    Q_ASSERT(settings);
    Q_ASSERT(painter);
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_ratioOption.readOptionSetting(settings);
    m_spacingOption.readOptionSetting(settings);
    m_rateOption.readOptionSetting(settings);
    m_lightnessStrengthOption.readOptionSetting(settings);
    m_smudgeRateOption.readOptionSetting(settings);
    m_colorRateOption.readOptionSetting(settings);

    m_overlayModeOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_scatterOption.readOptionSetting(settings);
    m_gradientOption.readOptionSetting(settings);
    m_airbrushOption.readOptionSetting(settings);

    m_sizeOption.resetAllSensors();
    m_opacityOption.resetAllSensors();
    m_ratioOption.resetAllSensors();
    m_spacingOption.resetAllSensors();
    m_rateOption.resetAllSensors();
    m_lightnessStrengthOption.resetAllSensors();
    m_smudgeRateOption.resetAllSensors();
    m_colorRateOption.resetAllSensors();

    m_rotationOption.resetAllSensors();
    m_scatterOption.resetAllSensors();
    m_gradientOption.resetAllSensors();

    m_gradient = painter->gradient();

    const bool useNewEngine = m_brush->brushApplication() != ALPHAMASK || m_smudgeRateOption.getUseNewEngine();
    const bool useSmearAlpha = m_smudgeRateOption.getSmearAlpha();
    const bool useDullingMode = m_smudgeRateOption.getMode() == KisSmudgeOption::DULLING_MODE;
    const bool useOverlayMode = m_overlayModeOption.isChecked();

    if (useNewEngine){
        m_smudgeRadiusOption.updateRange(0.0, 1.0);
    } else {
        m_smudgeRadiusOption.updateRange(0.0, 3.0);
    }

    // Initialize smudge radius only after the proper range is set
    m_smudgeRadiusOption.readOptionSetting(settings);
    m_smudgeRadiusOption.resetAllSensors();

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

    if (useNewEngine && m_brush->brushApplication() == LIGHTNESSMAP) {
        m_strategy.reset(new KisColorSmudgeStrategyLightness(painter,
                                                             useSmearAlpha,
                                                             useDullingMode));
    } else if (useNewEngine && m_brush->brushApplication() == ALPHAMASK) {
        m_strategy.reset(new KisColorSmudgeStrategyMask(painter,
                                                        image,
                                                        useSmearAlpha,
                                                        useDullingMode,
                                                        useOverlayMode));
    } else if (useNewEngine &&
               (m_brush->brushApplication() == IMAGESTAMP ||
                m_brush->brushApplication() == GRADIENTMAP)) {
        m_strategy.reset(new KisColorSmudgeStrategyStamp(painter,
                                                         image,
                                                         useSmearAlpha,
                                                         useDullingMode,
                                                         useOverlayMode));
    } else {
        m_strategy.reset(new KisColorSmudgeStrategyMaskLegacy(painter,
                                                              image,
                                                              useSmearAlpha,
                                                              useDullingMode,
                                                              useOverlayMode));
    }

    m_strategy->initializePainting();
    m_paintColor = painter->paintColor().convertedTo(m_strategy->preciseColorSpace());
}

KisColorSmudgeOp::~KisColorSmudgeOp()
{
    qDeleteAll(m_hsvOptions);
    delete m_hsvTransform;
}

KisSpacingInformation KisColorSmudgeOp::paintAt(const KisPaintInformation& info)
{
    KisBrushSP brush = m_brush;

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

    // get the scaling factor calculated by the size option
    qreal scale = m_sizeOption.apply(info);
    scale *= KisLodTransform::lodToScale(painter()->device());
    qreal rotation = m_rotationOption.apply(info);
    if (checkSizeTooSmall(scale)) return KisSpacingInformation();

    qreal ratio = m_ratioOption.apply(info);

    KisDabShape shape(scale, ratio, rotation);

    QPointF scatteredPos =
        m_scatterOption.apply(info,
                              brush->maskWidth(shape, 0, 0, info),
                              brush->maskHeight(shape, 0, 0, info));

    const qreal smudgeRadiusPortion = m_smudgeRadiusOption.isChecked() ? m_smudgeRadiusOption.computeSizeLikeValue(info) : 0.0;

    KisSpacingInformation spacingInfo =
            effectiveSpacing(scale, rotation,
                             &m_airbrushOption, &m_spacingOption, info);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_strategy, spacingInfo);


    const qreal lightnessStrength = m_lightnessStrengthOption.apply(info);
    m_strategy->updateMask(m_dabCache, info, shape, scatteredPos, &m_dstDabRect, lightnessStrength);

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

    if (m_firstRun) {
        m_firstRun = false;
        return spacingInfo;
    }

    const qreal colorRate = m_colorRateOption.isChecked() ? m_colorRateOption.computeSizeLikeValue(info) : 0.0;
    const qreal smudgeRate = m_smudgeRateOption.isChecked() ? m_smudgeRateOption.computeSizeLikeValue(info) : 1.0;
    const qreal maxSmudgeRate = m_smudgeRateOption.getRate();
    const qreal fpOpacity = m_opacityOption.getOpacityf(info);

    KoColor paintColor = m_paintColor;

    m_gradientOption.apply(paintColor, m_gradient, info);
    if (m_hsvTransform) {
        Q_FOREACH (KisPressureHSVOption * option, m_hsvOptions) {
            option->apply(m_hsvTransform, info);
        }
        m_hsvTransform->transform(paintColor.data(), paintColor.data(), 1);
    }

    const QVector<QRect> dirtyRects =
            m_strategy->paintDab(srcDabRect, m_dstDabRect,
                                 paintColor,
                                 fpOpacity, colorRate,
                                 smudgeRate,
                                 maxSmudgeRate,
                                 lightnessStrength,
                                 smudgeRadiusPortion);

    painter()->addDirtyRects(dirtyRects);

    return spacingInfo;
}

KisSpacingInformation KisColorSmudgeOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    const qreal scale = m_sizeOption.apply(info) * KisLodTransform::lodToScale(painter()->device());
    const qreal rotation = m_rotationOption.apply(info);
    return effectiveSpacing(scale, rotation, &m_airbrushOption, &m_spacingOption, info);
}

KisTimingInformation KisColorSmudgeOp::updateTimingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveTiming(&m_airbrushOption, &m_rateOption, info);
}

KisInterstrokeDataFactory *KisColorSmudgeOp::createInterstrokeDataFactory(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface)
{
    bool needsInterstrokeData =
        settings->getBool(QString("SmudgeRate") + "UseNewEngine", false);

    if (!needsInterstrokeData) return 0;

    KisBrushOptionProperties brushOption;
    needsInterstrokeData &=
        brushOption.brushApplication(settings.data(), resourcesInterface) == LIGHTNESSMAP;

    return needsInterstrokeData ? new ColorSmudgeInterstrokeDataFactory() : 0;
}
