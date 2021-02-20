/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  See LayerFX plugin for Gimp as a reference implementation of this style:
 *  http://registry.gimp.org/node/186
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_ls_bevel_emboss_filter.h"

#include <cstdlib>

#include <QBitArray>

#include <KoUpdater.h>
#include <resources/KoPattern.h>

#include <resources/KoAbstractGradient.h>

#include "psd.h"

#include "kis_convolution_kernel.h"
#include "kis_convolution_painter.h"
#include "kis_gaussian_kernel.h"

#include "kis_pixel_selection.h"
#include "kis_fill_painter.h"
#include "kis_gradient_painter.h"
#include "kis_iterator_ng.h"
#include "kis_random_accessor_ng.h"

#include "kis_psd_layer_style.h"
#include "kis_layer_style_filter_environment.h"

#include "kis_ls_utils.h"

#include "gimp_bump_map.h"
#include "kis_transaction.h"
#include "kis_multiple_projection.h"
#include "kis_cached_paint_device.h"


KisLsBevelEmbossFilter::KisLsBevelEmbossFilter()
    : KisLayerStyleFilter(KoID("lsstroke", i18n("Stroke (style)")))
{
}

KisLayerStyleFilter *KisLsBevelEmbossFilter::clone() const
{
    return new KisLsBevelEmbossFilter(*this);
}

KisLsBevelEmbossFilter::KisLsBevelEmbossFilter(const KisLsBevelEmbossFilter &rhs)
    : KisLayerStyleFilter(rhs)
{
}

void paintBevelSelection(KisPixelSelectionSP srcSelection,
                         KisPixelSelectionSP dstSelection,
                         const QRect &applyRect,
                         int size,
                         int initialSize,
                         bool invert,
                         KisLayerStyleFilterEnvironment *env)
{
    KisCachedSelection::Guard s1(*env->cachedSelection());
    KisSelectionSP tmpBaseSelection = s1.selection();
    KisPixelSelectionSP tmpSelection = tmpBaseSelection->pixelSelection();

    // NOTE: we are not using createCompositionSourceDevice() intentionally,
    //       because the source device doesn't have alpha channel
    KisCachedSelection::Guard s2(*env->cachedSelection());
    KisPixelSelectionSP fillDevice = s2.selection()->pixelSelection();

    KisPainter gc(dstSelection);
    gc.setCompositeOp(COMPOSITE_COPY);

    for (int i = 0; i < size; i++) {
        const int growSize = initialSize - i - 1;

        quint8 selectedness = invert ?
            qRound(qreal(size - i - 1) / size * 255.0) :
            qRound(qreal(i + 1) / size * 255.0);
        fillDevice->setDefaultPixel(KoColor(&selectedness, fillDevice->colorSpace()));

        tmpSelection->makeCloneFromRough(srcSelection, srcSelection->selectedRect());

        QRect changeRect = KisLsUtils::growSelectionUniform(tmpSelection, growSize, applyRect);

        gc.setSelection(tmpBaseSelection);
        gc.bitBlt(changeRect.topLeft(), fillDevice, changeRect);
    }
}

struct ContrastOp {
    static const bool supportsCaching = false;

    ContrastOp(qreal contrast)
        : m_contrast(contrast)
    {
    }

    int operator() (int iValue) {
        qreal value = qreal(iValue - 127) / 127.0;

        qreal slant = std::tan ((m_contrast + 1) * M_PI_4);
        value = (value - 0.5) * slant + 0.5;

        return qRound(value * 255.0);
    }

private:
    qreal m_contrast;
};

struct HighlightsFetchOp {
    static const bool supportsCaching = true;

    int operator() (int value) {
        return qRound(qMax(0, value - 127) * (255.0 / (255 - 127)));
    }
};

struct ShadowsFetchOp {
    static const bool supportsCaching = true;

    int operator() (int value) {
        return 255 - qRound(qMin(value, 127) * (255.0 / 127.0));
    }
};

template <class MapOp>
void mapPixelValues(KisPixelSelectionSP srcSelection,
                    KisPixelSelectionSP dstSelection,
                    MapOp mapOp,
                    const QRect &applyRect)
{
    static quint8 mapTable[256];
    static bool mapInitialized = false;

    if (!MapOp::supportsCaching || !mapInitialized) {
        mapInitialized = true;

        for (int i = 0; i < 256; i++) {
            mapTable[i] = mapOp(i);
        }
    }

    KisSequentialConstIterator srcIt(srcSelection, applyRect);
    KisSequentialIterator dstIt(dstSelection, applyRect);

    while (srcIt.nextPixel() && dstIt.nextPixel()) {
        const quint8 *srcPtr = srcIt.rawDataConst();
        quint8 *dstPtr = dstIt.rawData();
        *dstPtr = mapTable[*srcPtr];
    }
}

template <class MapOp>
void mapPixelValues(KisPixelSelectionSP dstSelection,
                    MapOp mapOp,
                    const QRect &applyRect)
{
    static quint8 mapTable[256];
    static bool mapInitialized = false;

    if (!MapOp::supportsCaching || !mapInitialized) {
        mapInitialized = true;

        for (int i = 0; i < 256; i++) {
            mapTable[i] = mapOp(i);
        }
    }

    KisSequentialIterator dstIt(dstSelection, applyRect);

    while (dstIt.nextPixel()) {
        quint8 *dstPtr = dstIt.rawData();
        *dstPtr = mapTable[*dstPtr];
    }
}

struct BevelEmbossRectCalculator
{
    BevelEmbossRectCalculator(const QRect &applyRect,
                              const psd_layer_effects_bevel_emboss *config) {

        shadowHighlightsFinalRect = applyRect;
        applyGaussianRect = shadowHighlightsFinalRect;
        applyGlossContourRect = KisLsUtils::growRectFromRadius(applyGaussianRect, config->soften());
        applyBumpmapRect = applyGlossContourRect;
        applyContourRect = applyBumpmapRect;
        applyTextureRect = applyContourRect;
        applyBevelRect = calcBevelNeedRect(applyTextureRect, config);
        initialFetchRect = kisGrowRect(applyBevelRect, 1);
    }

    QRect totalChangeRect(const QRect &applyRect, const psd_layer_effects_bevel_emboss *config) {
        QRect changeRect = calcBevelChangeRect(applyRect, config);
        changeRect = kisGrowRect(changeRect, 1); // bumpmap method
        changeRect = KisLsUtils::growRectFromRadius(changeRect, config->soften());
        return changeRect;
    }

    QRect totalNeedRect(const QRect &applyRect, const psd_layer_effects_bevel_emboss *config) {
        QRect changeRect = applyRect;
        changeRect = KisLsUtils::growRectFromRadius(changeRect, config->soften());
        changeRect = kisGrowRect(changeRect, 1); // bumpmap method
        changeRect = calcBevelNeedRect(applyRect, config);
        return changeRect;
    }

    QRect initialFetchRect;
    QRect applyBevelRect;
    QRect applyTextureRect;
    QRect applyContourRect;
    QRect applyBumpmapRect;
    QRect applyGlossContourRect;
    QRect applyGaussianRect;
    QRect shadowHighlightsFinalRect;

private:
    QRect calcBevelChangeRect(const QRect &applyRect, const psd_layer_effects_bevel_emboss *config) {
        const int size = config->size();
        int limitingGrowSize = 0;

        switch (config->style()) {
        case psd_bevel_outer_bevel:
            limitingGrowSize = size;
            break;
        case psd_bevel_inner_bevel:
            limitingGrowSize = 0;
            break;
        case psd_bevel_emboss: {
            const int initialSize = std::ceil(qreal(size) / 2.0);
            limitingGrowSize = initialSize;
            break;
        }
        case psd_bevel_pillow_emboss: {
            const int halfSizeC = std::ceil(qreal(size) / 2.0);
            limitingGrowSize = halfSizeC;
            break;
        }
        case psd_bevel_stroke_emboss:
            warnKrita << "WARNING: Stroke Emboss style is not implemented yet!";
            return applyRect;
        }

        return kisGrowRect(applyRect, limitingGrowSize);
    }

    QRect calcBevelNeedRect(const QRect &applyRect, const psd_layer_effects_bevel_emboss *config) {
        const int size = config->size();
        int limitingGrowSize = size;

        return kisGrowRect(applyRect, limitingGrowSize);
    }
};

void KisLsBevelEmbossFilter::applyBevelEmboss(KisPaintDeviceSP srcDevice,
                                              KisMultipleProjection *dst,
                                              const QRect &applyRect,
                                              const psd_layer_effects_bevel_emboss *config,
                                              KisLayerStyleFilterEnvironment *env) const
{
    if (applyRect.isEmpty()) return;

    BevelEmbossRectCalculator d(applyRect, config);

    KisCachedSelection::Guard s1(*env->cachedSelection());
    KisSelectionSP baseSelection = s1.selection();
    KisLsUtils::selectionFromAlphaChannel(srcDevice, baseSelection, d.initialFetchRect);

    KisPixelSelectionSP selection = baseSelection->pixelSelection();

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("0_selection_initial.png");

    const int size = config->size();

    int limitingGrowSize = 0;
    KisCachedSelection::Guard s2(*env->cachedSelection());
    KisPixelSelectionSP bumpmapSelection = s2.selection()->pixelSelection();

    switch (config->style()) {
    case psd_bevel_outer_bevel:
        paintBevelSelection(selection, bumpmapSelection, d.applyBevelRect, size, size, false, env);
        limitingGrowSize = size;
        break;
    case psd_bevel_inner_bevel:
        paintBevelSelection(selection, bumpmapSelection, d.applyBevelRect, size, 0, false, env);
        limitingGrowSize = 0;
        break;
    case psd_bevel_emboss: {
        const int initialSize = std::ceil(qreal(size) / 2.0);
        paintBevelSelection(selection, bumpmapSelection, d.applyBevelRect, size, initialSize, false, env);
        limitingGrowSize = initialSize;
        break;
    }
    case psd_bevel_pillow_emboss: {
        const int halfSizeF = std::floor(qreal(size) / 2.0);
        const int halfSizeC = std::ceil(qreal(size) / 2.0);
        // TODO: probably not correct!
        paintBevelSelection(selection, bumpmapSelection, d.applyBevelRect, halfSizeC, halfSizeC, false, env);
        paintBevelSelection(selection, bumpmapSelection, d.applyBevelRect, halfSizeF, 0, true, env);
        limitingGrowSize = halfSizeC;
        break;
    }
    case psd_bevel_stroke_emboss:
        warnKrita << "WARNING: Stroke Emboss style is not implemented yet!";
        return;
    }

    KisCachedSelection::Guard s3(*env->cachedSelection());
    KisPixelSelectionSP limitingSelection = s3.selection()->pixelSelection();
    limitingSelection->makeCloneFromRough(selection, selection->selectedRect());
    {
        QRect changeRectUnused =
            KisLsUtils::growSelectionUniform(limitingSelection,
                                             limitingGrowSize,
                                             d.applyBevelRect);
        Q_UNUSED(changeRectUnused);
    }

    //bumpmapSelection->convertToQImage(0, QRect(0,0,300,300)).save("1_selection_xconv.png");

    if (config->textureEnabled()) {
        KisCachedSelection::Guard s4(*env->cachedSelection());
        KisPixelSelectionSP textureSelection = s4.selection()->pixelSelection();

        KisLsUtils::fillPattern(textureSelection, d.applyTextureRect, env,
                                config->textureScale(),
                                config->texturePattern(),
                                config->textureHorizontalPhase(),
                                config->textureVerticalPhase(),
                                config->textureAlignWithLayer());

        int contrastadj = 0;

        {
            using namespace std;

            int tex_depth = config->textureDepth();

            if (tex_depth >= 0.0) {
                if (tex_depth <= 100.0) {
                    contrastadj = int(qRound((1-(tex_depth/100.0)) * -127));
                } else {
                    contrastadj = int(qRound(((tex_depth-100.0)/900.0) * 127));
                }
            } else {
                textureSelection->invert();
                if (tex_depth >= -100.0) {
                    contrastadj = int(qRound((1-(abs(tex_depth)/100.0)) * -127));
                } else {
                    contrastadj = int(qRound(((abs(tex_depth)-100.0)/900.0) * 127));
                }
            }
        }

        qreal contrast = qBound(-1.0, qreal(contrastadj) / 127.0, 1.0);
        mapPixelValues(textureSelection, ContrastOp(contrast), d.applyTextureRect);

        {
            KisPainter gc(bumpmapSelection);
            gc.setCompositeOp(COMPOSITE_MULT);
            gc.bitBlt(d.applyTextureRect.topLeft(), textureSelection, d.applyTextureRect);
            gc.end();
        }
    }

    //bumpmapSelection->convertToQImage(0, QRect(0,0,300,300)).save("15_selection_texture.png");

    if (config->contourEnabled()) {
        if (config->range() != KisLsUtils::FULL_PERCENT_RANGE) {
            KisLsUtils::adjustRange(bumpmapSelection, d.applyContourRect, config->range());
        }

        KisLsUtils::applyContourCorrection(bumpmapSelection,
                                           d.applyContourRect,
                                           config->contourLookupTable(),
                                           config->antiAliased(),
                                           true);
    }

    bumpmap_vals_t bmvals;

    bmvals.azimuth = config->angle();
    bmvals.elevation = config->altitude();
    bmvals.depth = config->depth();
    bmvals.ambient = 0;
    bmvals.compensate = true;
    bmvals.invert = config->direction() == psd_direction_down;
    bmvals.type = LINEAR;

    bumpmap(bumpmapSelection, d.applyBumpmapRect, bmvals);

    //bumpmapSelection->convertToQImage(0, QRect(0,0,300,300)).save("3_selection_bumpmap.png");

    { // TODO: optimize!

        KisLsUtils::applyContourCorrection(bumpmapSelection,
                                           d.applyGlossContourRect,
                                           config->glossContourLookupTable(),
                                           config->glossAntiAliased(),
                                           true);

    }

    if (config->soften()) {
        KisLsUtils::applyGaussianWithTransaction(bumpmapSelection, d.applyGaussianRect, config->soften());
    }


    if (config->textureEnabled() && config->textureInvert()) {
        bumpmapSelection->invert();
    }

    selection->clear();
    mapPixelValues(bumpmapSelection, selection,
                   ShadowsFetchOp(), d.shadowHighlightsFinalRect);
    selection->applySelection(limitingSelection, SELECTION_INTERSECT);

    //dstDevice->convertToQImage(0, QRect(0,0,300,300)).save("4_dst_before_apply.png");
    //selection->convertToQImage(0, QRect(0,0,300,300)).save("4_shadows_sel.png");

    {
        KisPaintDeviceSP dstDevice = dst->getProjection("00_bevel_shadow",
                                                        config->shadowBlendMode(),
                                                        config->shadowOpacity(),
                                                        QBitArray(),
                                                        srcDevice);

        const KoColor fillColor(config->shadowColor(), dstDevice->colorSpace());
        const QRect &fillRect = d.shadowHighlightsFinalRect;

        KisCachedPaintDevice::Guard d1(dstDevice, *env->cachedPaintDevice());
        KisPaintDeviceSP fillDevice = d1.device();
        fillDevice->setDefaultPixel(fillColor);

        KisPainter::copyAreaOptimized(fillRect.topLeft(), fillDevice, dstDevice, fillRect, baseSelection);
    }

    selection->clear();
    mapPixelValues(bumpmapSelection, selection,
                   HighlightsFetchOp(), d.shadowHighlightsFinalRect);
    selection->applySelection(limitingSelection, SELECTION_INTERSECT);

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("5_highlights_sel.png");

    {
        KisPaintDeviceSP dstDevice = dst->getProjection("01_bevel_highlight",
                                                        config->highlightBlendMode(),
                                                        config->highlightOpacity(),
                                                        QBitArray(),
                                                        srcDevice);

        const KoColor fillColor(config->highlightColor(), dstDevice->colorSpace());
        const QRect &fillRect = d.shadowHighlightsFinalRect;

        KisCachedPaintDevice::Guard d1(dstDevice, *env->cachedPaintDevice());
        KisPaintDeviceSP fillDevice = d1.device();
        fillDevice->setDefaultPixel(fillColor);

        KisPainter::copyAreaOptimized(fillRect.topLeft(), fillDevice, dstDevice, fillRect, baseSelection);
    }
}

void KisLsBevelEmbossFilter::processDirectly(KisPaintDeviceSP src,
                                             KisMultipleProjection *dst,
                                             KisLayerStyleKnockoutBlower *blower,
                                             const QRect &applyRect,
                                             KisPSDLayerStyleSP style,
                                             KisLayerStyleFilterEnvironment *env) const
{
    Q_UNUSED(env);
    Q_UNUSED(blower);
    KIS_ASSERT_RECOVER_RETURN(style);

    const psd_layer_effects_bevel_emboss *config = style->bevelAndEmboss();
    if (!KisLsUtils::checkEffectEnabled(config, dst)) return;

    KisLsUtils::LodWrapper<psd_layer_effects_bevel_emboss> w(env->currentLevelOfDetail(), config);
    applyBevelEmboss(src, dst, applyRect, w.config, env);
}

QRect KisLsBevelEmbossFilter::neededRect(const QRect &rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const
{
    const psd_layer_effects_bevel_emboss *config = style->bevelAndEmboss();
    if (!config->effectEnabled()) return rect;

    KisLsUtils::LodWrapper<psd_layer_effects_bevel_emboss> w(env->currentLevelOfDetail(), config);

    BevelEmbossRectCalculator d(rect, w.config);
    return d.totalNeedRect(rect, w.config);
}

QRect KisLsBevelEmbossFilter::changedRect(const QRect &rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const
{
    const psd_layer_effects_bevel_emboss *config = style->bevelAndEmboss();
    if (!config->effectEnabled()) return rect;

    KisLsUtils::LodWrapper<psd_layer_effects_bevel_emboss> w(env->currentLevelOfDetail(), config);

    BevelEmbossRectCalculator d(rect, w.config);
    return d.totalChangeRect(rect, w.config);
}
