/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_layer_style_projection_plane_test.h"

#include <qtest_kde.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_transparency_mask.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_painter.h"

#include "kis_selection.h"
#include "kis_pixel_selection.h"


#include "../kis_layer_style_projection_plane.h"


void KisLayerStyleProjectionPlaneTest::test()
{
    const QRect imageRect(0, 0, 200, 200);
    const QRect rFillRect(10, 10, 100, 100);
    const QRect tMaskRect(50, 50, 20, 20);
    const QRect partialSelectionRect(90, 50, 20, 20);


    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "styles test");

    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);
    image->addNode(layer);

    KisLayerStyleProjectionPlane plane(layer);

    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "00L_initial", "dd");

    layer->paintDevice()->fill(rFillRect, KoColor(Qt::red, cs));

    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "01L_fill", "dd");

    KisPaintDeviceSP projection = new KisPaintDevice(cs);

    {
        const QRect changeRect = plane.changeRect(rFillRect, KisLayer::N_FILTHY);
        qDebug() << ppVar(rFillRect) << ppVar(changeRect);

        plane.recalculate(changeRect, layer);

        KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "02L_recalculate_fill", "dd");

        KisPainter painter(projection);
        plane.apply(&painter, changeRect);

        KIS_DUMP_DEVICE_2(projection, imageRect, "03P_apply_on_fill", "dd");
    }

    KisTransparencyMaskSP transparencyMask = new KisTransparencyMask();

    KisSelectionSP selection = new KisSelection();
    selection->pixelSelection()->select(tMaskRect, OPACITY_OPAQUE_U8);
    transparencyMask->setSelection(selection);
    image->addNode(transparencyMask, layer);

    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "04L_mask_added", "dd");

    plane.recalculate(imageRect, layer);

    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "05L_mask_added_recalculated", "dd");

    {
        projection->clear();

        KisPainter painter(projection);
        plane.apply(&painter, imageRect);

        KIS_DUMP_DEVICE_2(projection, imageRect, "06P_apply_on_mask", "dd");
    }

    selection->pixelSelection()->select(partialSelectionRect, OPACITY_OPAQUE_U8);

    {
        const QRect changeRect = plane.changeRect(partialSelectionRect, KisLayer::N_FILTHY);
        qDebug() << ppVar(rFillRect) << ppVar(changeRect);

        plane.recalculate(changeRect, layer);

        KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "07L_recalculate_partial", "dd");

        KisPainter painter(projection);
        plane.apply(&painter, changeRect);

        KIS_DUMP_DEVICE_2(projection, imageRect, "08P_apply_partial", "dd");
    }

}

QTEST_KDEMAIN(KisLayerStyleProjectionPlaneTest, GUI)
