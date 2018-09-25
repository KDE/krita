/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_ls_satin_filter.h"

#include <cstdlib>

#include <QBitArray>

#include <resources/KoAbstractGradient.h>

#include "psd.h"

#include "kis_convolution_kernel.h"
#include "kis_convolution_painter.h"
#include "kis_gaussian_kernel.h"

#include "kis_pixel_selection.h"
#include "kis_fill_painter.h"
#include "kis_iterator_ng.h"
#include "kis_random_accessor_ng.h"

#include "kis_psd_layer_style.h"

#include "kis_multiple_projection.h"
#include "kis_ls_utils.h"
#include "kis_layer_style_filter_environment.h"


KisLsSatinFilter::KisLsSatinFilter()
    : KisLayerStyleFilter(KoID("lssatin", i18n("Satin (style)")))
{
}

KisLsSatinFilter::KisLsSatinFilter(const KisLsSatinFilter &rhs)
    : KisLayerStyleFilter(rhs)
{
}

KisLayerStyleFilter *KisLsSatinFilter::clone() const
{
    return new KisLsSatinFilter(*this);
}

struct SatinRectsData
{
    enum Direction {
        NEED_RECT,
        CHANGE_RECT
    };

    SatinRectsData(const QRect &applyRect,
                    const psd_layer_effects_context *context,
                    const psd_layer_effects_satin *shadow,
                    Direction direction)
    {
        Q_UNUSED(direction);

        blur_size = shadow->size();
        offset = shadow->calculateOffset(context);

        // need rect calculation in reverse order
        dstRect = applyRect;

        srcRect = dstRect;

        int xGrow = qAbs(offset.x());
        int yGrow = qAbs(offset.y());
        satinNeedRect = srcRect.adjusted(-xGrow, -yGrow, xGrow, yGrow);

        blurNeedRect = blur_size ?
            KisLsUtils::growRectFromRadius(satinNeedRect, blur_size) : satinNeedRect;
    }

    inline QRect finalNeedRect() const {
        return blurNeedRect;
    }

    inline QRect finalChangeRect() const {
        // TODO: is it correct?
        return blurNeedRect;
    }

    qint32 blur_size;
    QPoint offset;

    QRect srcRect;
    QRect dstRect;
    QRect satinNeedRect;
    QRect blurNeedRect;
};

void blendAndOffsetSatinSelection(KisPixelSelectionSP dstSelection,
                                  KisPixelSelectionSP srcSelection,
                                  const bool invert,
                                  const QPoint &offset,
                                  const QRect &applyRect)
{
    KisSequentialIterator srcIt1(srcSelection, applyRect.translated(offset));
    KisSequentialIterator srcIt2(srcSelection, applyRect.translated(-offset));
    KisSequentialIterator dstIt(dstSelection, applyRect);

    while(dstIt.nextPixel() && srcIt1.nextPixel() && srcIt2.nextPixel()) {

        quint8 *dstPixelPtr = dstIt.rawData();
        quint8 *src1PixelPtr = srcIt1.rawData();
        quint8 *src2PixelPtr = srcIt2.rawData();

        if (!invert) {
            *dstPixelPtr = *dstPixelPtr * qAbs(*src1PixelPtr - *src2PixelPtr) >> 8;
        } else {
            *dstPixelPtr = *dstPixelPtr * (255 - qAbs(*src1PixelPtr - *src2PixelPtr)) >> 8;
        }
    }
}

void applySatin(KisPaintDeviceSP srcDevice,
                KisMultipleProjection *dst,
                const QRect &applyRect,
                const psd_layer_effects_context *context,
                const psd_layer_effects_satin *config,
                const KisLayerStyleFilterEnvironment *env)
{
    if (applyRect.isEmpty()) return;

    SatinRectsData d(applyRect, context, config, SatinRectsData::NEED_RECT);

    KisSelectionSP baseSelection =
        KisLsUtils::selectionFromAlphaChannel(srcDevice, d.blurNeedRect);

    KisPixelSelectionSP selection = baseSelection->pixelSelection();

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("0_selection_initial.png");

    KisPixelSelectionSP knockOutSelection = new KisPixelSelection(*selection);
    knockOutSelection->invert();

    //knockOutSelection->convertToQImage(0, QRect(0,0,300,300)).save("1_saved_knockout_selection.png");

    KisPixelSelectionSP tempSelection = new KisPixelSelection(*selection);

    KisLsUtils::applyGaussianWithTransaction(tempSelection, d.satinNeedRect, d.blur_size);

    //tempSelection->convertToQImage(0, QRect(0,0,300,300)).save("2_selection_blurred.png");

    /**
     * Contour correction
     */
    KisLsUtils::applyContourCorrection(tempSelection,
                                       d.satinNeedRect,
                                       config->contourLookupTable(),
                                       config->antiAliased(),
                                       config->edgeHidden());

    //tempSelection->convertToQImage(0, QRect(0,0,300,300)).save("3_selection_contour.png");

    blendAndOffsetSatinSelection(selection,
                                 tempSelection,
                                 config->invert(),
                                 d.offset,
                                 d.dstRect);

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("3_selection_satin_applied.png");

    /**
     * Knock-out original outline of the device from the resulting shade
     */
    if (config->knocksOut()) {
        KisLsUtils::knockOutSelection(selection,
                                      knockOutSelection,
                                      d.srcRect,
                                      d.dstRect,
                                      d.finalNeedRect(),
                                      config->invertsSelection());
    }
    //selection->convertToQImage(0, QRect(0,0,300,300)).save("5_selection_knocked_out.png");

    KisLsUtils::applyFinalSelection(KisMultipleProjection::defaultProjectionId(),
                                    baseSelection,
                                    srcDevice,
                                    dst,
                                    d.srcRect,
                                    d.dstRect,
                                    context,
                                    config,
                                    env);

    //dstDevice->convertToQImage(0, QRect(0,0,300,300)).save("6_dst_final.png");
}

void KisLsSatinFilter::processDirectly(KisPaintDeviceSP src,
                                       KisMultipleProjection *dst,
                                       const QRect &applyRect,
                                       KisPSDLayerStyleSP style,
                                       KisLayerStyleFilterEnvironment *env) const
{
    KIS_ASSERT_RECOVER_RETURN(style);

    const psd_layer_effects_satin *config = style->satin();
    if (!KisLsUtils::checkEffectEnabled(config, dst)) return;

    KisLsUtils::LodWrapper<psd_layer_effects_satin> w(env->currentLevelOfDetail(), config);
    applySatin(src, dst, applyRect, style->context(), w.config, env);
}

QRect KisLsSatinFilter::neededRect(const QRect &rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const
{
    const psd_layer_effects_satin *config = style->satin();
    if (!config->effectEnabled()) return rect;

    KisLsUtils::LodWrapper<psd_layer_effects_satin> w(env->currentLevelOfDetail(), config);
    SatinRectsData d(rect, style->context(), w.config, SatinRectsData::NEED_RECT);
    return rect | d.finalNeedRect();
}

QRect KisLsSatinFilter::changedRect(const QRect &rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const
{
    const psd_layer_effects_satin *config = style->satin();
    if (!config->effectEnabled()) return rect;

    KisLsUtils::LodWrapper<psd_layer_effects_satin> w(env->currentLevelOfDetail(), config);
    SatinRectsData d(rect, style->context(), w.config, SatinRectsData::CHANGE_RECT);
    return style->context()->keep_original ?
        d.finalChangeRect() : rect | d.finalChangeRect();
}
