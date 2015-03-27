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

#include "kis_ls_stroke_filter.h"

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



KisLsStrokeFilter::KisLsStrokeFilter()
    : KisLayerStyleFilter(KoID("lsstroke", i18n("Stroke (style)")))
{
}

void paintPathOnSelection(KisPixelSelectionSP selection,
                          const QPainterPath &path,
                          const QRect &applyRect,
                          int size)
{
    QPen pen(Qt::white, size);
    KisPainter gc(selection);
    gc.setPaintColor(KoColor(Qt::white, selection->colorSpace()));
    gc.drawPainterPath(path, pen, applyRect);
    gc.end();
}

void KisLsStrokeFilter::applyStroke(KisPaintDeviceSP srcDevice,
                                    KisPaintDeviceSP dstDevice,
                                    const QRect &applyRect,
                                    const psd_layer_effects_stroke *config,
                                    KisLayerStyleFilterEnvironment *env) const
{
    if (applyRect.isEmpty()) return;

    KisSelectionSP baseSelection = new KisSelection(new KisSelectionEmptyBounds(0));
    KisPixelSelectionSP selection = baseSelection->pixelSelection();

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("0_selection_initial.png");

    QPainterPath strokePath = env->layerOutlineCache();
    if (strokePath.isEmpty()) return;

    if (config->position() == psd_stroke_center) {
        paintPathOnSelection(selection, strokePath,
                             applyRect, config->size());
    } else if (config->position() == psd_stroke_outside ||
               config->position() == psd_stroke_inside) {

        paintPathOnSelection(selection, strokePath,
                                         applyRect, 2 * config->size());

        KisSelectionSP knockOutSelection =
            KisLsUtils::selectionFromAlphaChannel(srcDevice, applyRect);

        // disabled intentionally, because it creates artifacts on smooth lines
        // KisLsUtils::findEdge(knockOutSelection->pixelSelection(), applyRect, true);

        if (config->position() == psd_stroke_inside) {
            knockOutSelection->pixelSelection()->invert();
        }

        KisPainter gc(selection);
        gc.setCompositeOp(COMPOSITE_ERASE);
        gc.bitBlt(applyRect.topLeft(), knockOutSelection->pixelSelection(), applyRect);
        gc.end();
    }

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("1_selection_stroke.png");

    KisPaintDeviceSP fillDevice = new KisPaintDevice(dstDevice->colorSpace());
    KisLsUtils::fillOverlayDevice(fillDevice, applyRect, config, env);


    KisPainter gc(dstDevice);
    const QString compositeOp = config->blendMode();
    const quint8 opacityU8 = 255.0 / 100.0 * config->opacity();

    gc.setCompositeOp(compositeOp);
    gc.setOpacity(opacityU8);
    gc.setSelection(baseSelection);

    gc.bitBlt(applyRect.topLeft(), fillDevice, applyRect);
}

void KisLsStrokeFilter::processDirectly(KisPaintDeviceSP src,
                                         KisPaintDeviceSP dst,
                                         const QRect &applyRect,
                                         KisPSDLayerStyleSP style,
                                         KisLayerStyleFilterEnvironment *env) const
{
    Q_UNUSED(env);
    KIS_ASSERT_RECOVER_RETURN(style);

    const psd_layer_effects_stroke *config = style->stroke();
    if (!config->effectEnabled()) return;

    applyStroke(src, dst, applyRect, config, env);
}

QRect KisLsStrokeFilter::neededRect(const QRect &rect, KisPSDLayerStyleSP style) const
{
    Q_UNUSED(style);
    return rect;
}

QRect KisLsStrokeFilter::changedRect(const QRect &rect, KisPSDLayerStyleSP style) const
{
    const int borderSize = style->stroke()->size() + 1;
    return kisGrowRect(rect, borderSize);
}
