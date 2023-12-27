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

KisColorSmudgeOp::KisColorSmudgeOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
    , m_firstRun(true)
    , m_sizeOption(settings.data())
    , m_ratioOption(settings.data())
    , m_opacityOption(settings.data())
    , m_spacingOption(settings.data())
    , m_rateOption(settings.data())
    , m_rotationOption(settings.data())
    , m_scatterOption(settings.data())
    , m_paintThicknessOption(settings.data())
    , m_gradientOption(settings.data())
    , m_smudgeRateOption(settings.data())
    , m_colorRateOption(settings.data())
    , m_smudgeRadiusOption(settings.data())
{
    Q_UNUSED(node);
    Q_ASSERT(painter);

    m_airbrushData.read(settings.data());
    m_overlayModeData.read(settings.data());

    m_gradient = painter->gradient();

    // useNewEngine should be true if brushApplication is not ALPHAMASK
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_brush->brushApplication() == ALPHAMASK || m_smudgeRateOption.useNewEngine());

    const bool useSmearAlpha = m_smudgeRateOption.smearAlpha();
    const bool useDullingMode = m_smudgeRateOption.mode() == KisSmudgeLengthOptionData::DULLING_MODE;
    const bool useOverlayMode = m_overlayModeData.isChecked;

    m_rotationOption.applyFanCornersInfo(this);

    if (m_brush->brushApplication() == LIGHTNESSMAP) {
        KisPaintThicknessOptionData::ThicknessMode thicknessMode =
            m_paintThicknessOption.isChecked() ?
                m_paintThicknessOption.mode() :
                KisPaintThicknessOptionData::OVERWRITE;

        m_strategy.reset(new KisColorSmudgeStrategyLightness(painter,
                                                             useSmearAlpha,
                                                             useDullingMode,
                                                             thicknessMode));
    } else if (m_smudgeRateOption.useNewEngine() &&
               m_brush->brushApplication() == ALPHAMASK) {
        m_strategy.reset(new KisColorSmudgeStrategyMask(painter,
                                                        image,
                                                        useSmearAlpha,
                                                        useDullingMode,
                                                        useOverlayMode));
    } else if (m_brush->brushApplication() == IMAGESTAMP ||
               m_brush->brushApplication() == GRADIENTMAP) {
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

    m_hsvOptions.append(KisHSVOption::createHueOption(settings.data()));
    m_hsvOptions.append(KisHSVOption::createSaturationOption(settings.data()));
    m_hsvOptions.append(KisHSVOption::createValueOption(settings.data()));

    /// color transformation must be created **after** the color
    /// space of m_paintColor has been set up
    Q_FOREACH (KisHSVOption * option, m_hsvOptions) {
        if (option->isChecked() && !m_hsvTransform) {
            m_hsvTransform = m_paintColor.colorSpace()->createColorTransformation("hsv_adjustment", QHash<QString, QVariant>());
        }
    }
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
    if (m_smudgeRateOption.mode() == KisSmudgeLengthOptionData::SMEARING_MODE) {
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
                             &m_airbrushData, &m_spacingOption, info);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_strategy, spacingInfo);


    const qreal paintThickness = m_paintThicknessOption.apply(info);
    m_strategy->updateMask(m_dabCache, info, shape, scatteredPos, &m_dstDabRect, paintThickness);

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
    const qreal maxSmudgeRate = m_smudgeRateOption.strengthValue();
    const qreal fpOpacity = m_opacityOption.apply(info);

    KoColor paintColor = m_paintColor;

    m_gradientOption.apply(paintColor, m_gradient, info);
    if (m_hsvTransform) {
        Q_FOREACH (KisHSVOption *option, m_hsvOptions) {
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
                                 paintThickness,
                                 smudgeRadiusPortion);

    painter()->addDirtyRects(dirtyRects);

    return spacingInfo;
}

KisSpacingInformation KisColorSmudgeOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    const qreal scale = m_sizeOption.apply(info) * KisLodTransform::lodToScale(painter()->device());
    const qreal rotation = m_rotationOption.apply(info);
    return effectiveSpacing(scale, rotation, &m_airbrushData, &m_spacingOption, info);
}

KisTimingInformation KisColorSmudgeOp::updateTimingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveTiming(&m_airbrushData, &m_rateOption, info);
}

KisInterstrokeDataFactory *KisColorSmudgeOp::createInterstrokeDataFactory(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface)
{

    KisBrushOptionProperties brushOption;
    const bool needsInterstrokeData =
        brushOption.brushApplication(settings.data(), resourcesInterface) == LIGHTNESSMAP;

    const bool needsNewEngine = settings->getBool(QString("SmudgeRate") + "UseNewEngine", false);
    KIS_SAFE_ASSERT_RECOVER_NOOP(!needsInterstrokeData || needsNewEngine);

    return needsInterstrokeData ? new ColorSmudgeInterstrokeDataFactory() : 0;
}
