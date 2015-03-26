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

#include "kis_ls_bevel_emboss_filter.h"

#include <cstdlib>

#include <QBitArray>

#include <KoUpdater.h>
#include <KoPattern.h>

#include <KoAbstractGradient.h>

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

#include "kis_convolution_kernel.h"
#include <kis_convolution_painter.h>

#include "gimp_bump_map.h"
#include "kis_transaction.h"
#include "kis_selection_filters.h"


KisLsBevelEmbossFilter::KisLsBevelEmbossFilter()
    : KisLayerStyleFilter(KoID("lsstroke", i18n("Stroke (style)")))
{
}

QRect growSelectionUniform(KisPixelSelectionSP selection, int growSize, const QRect &applyRect)
{
    QRect changeRect = applyRect;

    if (growSize > 0) {
        KisGrowSelectionFilter filter(growSize, growSize);
        changeRect = filter.changeRect(applyRect);
        filter.process(selection, applyRect);
    } else if (growSize < 0) {
        KisShrinkSelectionFilter filter(qAbs(growSize), qAbs(growSize), false);
        changeRect = filter.changeRect(applyRect);
        filter.process(selection, applyRect);
    }

    return changeRect;
}

void paintBevelSelection(KisPixelSelectionSP srcSelection,
                         KisPixelSelectionSP dstSelection,
                         const QRect &applyRect,
                         int size,
                         int initialSize,
                         bool invert)
{
    KisSelectionSP tmpBaseSelection = new KisSelection(new KisSelectionEmptyBounds(0));
    KisPixelSelectionSP tmpSelection = tmpBaseSelection->pixelSelection();

    // NOTE: we are not using createCompositionSourceDevice() intentionally,
    //       because the source device doesn't have alpha channel
    KisPixelSelectionSP fillDevice = new KisPixelSelection();

    KisPainter gc(dstSelection);
    gc.setCompositeOp(COMPOSITE_COPY);

    for (int i = 0; i < size; i++) {
        const int growSize = initialSize - i - 1;

        quint8 selectedness = invert ?
            qRound(qreal(size - i - 1) / size * 255.0) :
            qRound(qreal(i + 1) / size * 255.0);
        fillDevice->setDefaultPixel(&selectedness);

        tmpSelection->makeCloneFromRough(srcSelection, srcSelection->selectedRect());

        QRect changeRect = growSelectionUniform(tmpSelection, growSize, applyRect);

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

    do {
        const quint8 *srcPtr = srcIt.rawDataConst();
        quint8 *dstPtr = dstIt.rawData();
        *dstPtr = mapTable[*srcPtr];
    } while(srcIt.nextPixel() && dstIt.nextPixel());
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

    do {
        quint8 *dstPtr = dstIt.rawData();
        *dstPtr = mapTable[*dstPtr];
    } while(dstIt.nextPixel());
}

void KisLsBevelEmbossFilter::applyBevelEmboss(KisPaintDeviceSP srcDevice,
                                              KisPaintDeviceSP dstDevice,
                                              const QRect &applyRect,
                                              const psd_layer_effects_bevel_emboss *config,
                                              KisLayerStyleFilterEnvironment *env) const
{
    if (applyRect.isEmpty()) return;

    QRect totalNeedRect = applyRect;

    KisSelectionSP baseSelection = KisLsUtils::selectionFromAlphaChannel(srcDevice, totalNeedRect);
    KisPixelSelectionSP selection = baseSelection->pixelSelection();

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("0_selection_initial.png");

    const int size = config->size();

    int limitingGrowSize = 0;
    KisPixelSelectionSP bumpmapSelection = new KisPixelSelection(new KisSelectionEmptyBounds(0));

    switch (config->style()) {
    case psd_bevel_outer_bevel:
        paintBevelSelection(selection, bumpmapSelection, applyRect, size, size, false);
        limitingGrowSize = size;
        break;
    case psd_bevel_inner_bevel:
        paintBevelSelection(selection, bumpmapSelection, applyRect, size, 0, false);
        limitingGrowSize = 0;
        break;
    case psd_bevel_emboss: {
        const int initialSize = std::ceil(qreal(size) / 2.0);
        paintBevelSelection(selection, bumpmapSelection, applyRect, size, initialSize, false);
        limitingGrowSize = initialSize;
        break;
    }
    case psd_bevel_pillow_emboss: {
        const int halfSizeF = std::floor(qreal(size) / 2.0);
        const int halfSizeC = std::ceil(qreal(size) / 2.0);
        // TODO: probably not correct!
        paintBevelSelection(selection, bumpmapSelection, applyRect, halfSizeC, halfSizeC, false);
        paintBevelSelection(selection, bumpmapSelection, applyRect, halfSizeF, 0, true);
        limitingGrowSize = halfSizeC;
        break;
    }
    case psd_bevel_stroke_emboss:
        qWarning() << "WARNING: Stroke Emboss style is not implemented yet!";
        return;
    }

    KisPixelSelectionSP limitingSelection = new KisPixelSelection(*selection);
    {
        QRect changeRectUnused =
            growSelectionUniform(limitingSelection,
                                 limitingGrowSize,
                                 totalNeedRect);
        Q_UNUSED(changeRectUnused);
    }

    //bumpmapSelection->convertToQImage(0, QRect(0,0,300,300)).save("1_selection_xconv.png");

    if (config->textureEnabled()) {
        KisPixelSelectionSP textureSelection = new KisPixelSelection(new KisSelectionEmptyBounds(0));

        KisLsUtils::fillPattern(textureSelection, totalNeedRect, env,
                                config->textureScale(),
                                config->texturePattern(),
                                config->textureHorizontalPhase(),
                                config->textureVerticalPhase(),
                                config->textureAlignWithLayer());

        int contrastadj = 0;


        { // i'm not going to decrypt it, just copy :(

            using namespace std;

            int tex_depth = config->textureDepth();

            if (tex_depth >= 0.0) {
                if (tex_depth <= 100.0) {
                    contrastadj = int(round((1-(tex_depth/100.0)) * -127));
                } else {
                    contrastadj = int(round(((tex_depth-100.0)/900.0) * 127));
                }
            } else {
                textureSelection->invert();
                if (tex_depth >= -100.0) {
                    contrastadj = int(round((1-(abs(tex_depth)/100.0)) * -127));
                } else {
                    contrastadj = int(round(((abs(tex_depth)-100.0)/900.0) * 127));
                }
            }
        }

        qreal contrast = qBound(-1.0, qreal(contrastadj) / 127.0, 1.0);
        mapPixelValues(textureSelection, ContrastOp(contrast), totalNeedRect);

        {
            KisPainter gc(bumpmapSelection);
            gc.setCompositeOp(COMPOSITE_MULT);
            gc.bitBlt(totalNeedRect.topLeft(), textureSelection, totalNeedRect);
            gc.end();
        }
    }

    //bumpmapSelection->convertToQImage(0, QRect(0,0,300,300)).save("15_selection_texture.png");

    if (config->contourEnabled()) {
        if (config->range() != KisLsUtils::FULL_PERCENT_RANGE) {
            KisLsUtils::adjustRange(bumpmapSelection, totalNeedRect, config->range());
        }

        KisLsUtils::applyContourCorrection(bumpmapSelection,
                                           totalNeedRect,
                                           config->contourLookupTable(),
                                           config->antiAliased(),
                                           true);
    }

    bumpmap_vals_t bmvals;

    bmvals.azimuth = config->angle();
    bmvals.elevation = config->altitude();
    bmvals.depth = config->depth();
    bmvals.ambient = 0;
    bmvals.compensate = false; // FIXME: originally was 'true', but it doesn't work
    bmvals.invert = config->direction() == psd_direction_down; // FIXME: doesn't work
    bmvals.type = 0;

    bumpmap(bumpmapSelection, totalNeedRect, bmvals);

    //bumpmapSelection->convertToQImage(0, QRect(0,0,300,300)).save("3_selection_bumpmap.png");

    { // TODO: optimize!

        KisLsUtils::applyContourCorrection(bumpmapSelection,
                                           totalNeedRect,
                                           config->glossContourLookupTable(),
                                           config->glossAntiAliased(),
                                           true);

    }

    if (config->soften()) {
        KisLsUtils::applyGaussian(bumpmapSelection, totalNeedRect, config->soften());
    }


    if (config->textureEnabled() && config->textureInvert()) {
        // TODO: probably also not correct
        bumpmapSelection->invert();
    }

    selection->clear();
    mapPixelValues(bumpmapSelection, selection,
                   ShadowsFetchOp(), totalNeedRect);
    selection->applySelection(limitingSelection, SELECTION_INTERSECT);

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("4_shadows_sel.png");

    {
        const KoColor fillColor(config->shadowColor(), dstDevice->colorSpace());
        const QRect &fillRect = totalNeedRect;
        KisPaintDeviceSP fillDevice = new KisPaintDevice(dstDevice->colorSpace());
        fillDevice->setDefaultPixel(fillColor.data());
        KisPainter gc(dstDevice);

        gc.setSelection(baseSelection);
        gc.setCompositeOp(config->shadowBlendMode());
        gc.setOpacity(config->shadowOpacity());
        gc.bitBlt(fillRect.topLeft(), fillDevice, fillRect);
        gc.end();
    }

    selection->clear();
    mapPixelValues(bumpmapSelection, selection,
                   HighlightsFetchOp(), totalNeedRect);

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("5_highlights_sel.png");

    {
        const KoColor fillColor(config->highlightColor(), dstDevice->colorSpace());
        const QRect &fillRect = totalNeedRect;
        KisPaintDeviceSP fillDevice = new KisPaintDevice(dstDevice->colorSpace());
        fillDevice->setDefaultPixel(fillColor.data());
        KisPainter gc(dstDevice);
        gc.setSelection(baseSelection);
        gc.setCompositeOp(config->highlightBlendMode());
        gc.setOpacity(config->highlightOpacity());
        gc.bitBlt(fillRect.topLeft(), fillDevice, fillRect);
        gc.end();
    }
}

void KisLsBevelEmbossFilter::processDirectly(KisPaintDeviceSP src,
                                         KisPaintDeviceSP dst,
                                         const QRect &applyRect,
                                         KisPSDLayerStyleSP style,
                                         KisLayerStyleFilterEnvironment *env) const
{
    Q_UNUSED(env);
    KIS_ASSERT_RECOVER_RETURN(style);

    const psd_layer_effects_bevel_emboss *config = style->bevelEmboss();
    if (!config->effectEnabled()) return;

    applyBevelEmboss(src, dst, applyRect, config, env);
}

QRect KisLsBevelEmbossFilter::neededRect(const QRect &rect, KisPSDLayerStyleSP style) const
{
    Q_UNUSED(style);
    return rect;
}

QRect KisLsBevelEmbossFilter::changedRect(const QRect &rect, KisPSDLayerStyleSP style) const
{
    const int borderSize = style->stroke()->size() + 1;
    return kisGrowRect(rect, borderSize);
}
