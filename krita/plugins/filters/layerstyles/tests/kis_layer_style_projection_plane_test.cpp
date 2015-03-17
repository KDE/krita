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
#include "kis_psd_layer_style.h"


void KisLayerStyleProjectionPlaneTest::test(KisPSDLayerStyleSP style, const QString testName)
{
    const QRect imageRect(0, 0, 200, 200);
    const QRect rFillRect(10, 10, 100, 100);
    const QRect tMaskRect(50, 50, 20, 20);
    const QRect partialSelectionRect(90, 50, 20, 20);


    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "styles test");

    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);
    image->addNode(layer);

    KisLayerStyleProjectionPlane plane(layer.data(), style);

    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "00L_initial", testName);

    layer->paintDevice()->fill(rFillRect, KoColor(Qt::red, cs));

    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "01L_fill", testName);

    KisPaintDeviceSP projection = new KisPaintDevice(cs);

    {
        const QRect changeRect = plane.changeRect(rFillRect, KisLayer::N_FILTHY);
        qDebug() << ppVar(rFillRect) << ppVar(changeRect);

        plane.recalculate(changeRect, layer);

        KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "02L_recalculate_fill", testName);

        KisPainter painter(projection);
        plane.apply(&painter, changeRect);

        KIS_DUMP_DEVICE_2(projection, imageRect, "03P_apply_on_fill", testName);
    }

    return;

    KisTransparencyMaskSP transparencyMask = new KisTransparencyMask();

    KisSelectionSP selection = new KisSelection();
    selection->pixelSelection()->select(tMaskRect, OPACITY_OPAQUE_U8);
    transparencyMask->setSelection(selection);
    image->addNode(transparencyMask, layer);

    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "04L_mask_added", testName);

    plane.recalculate(imageRect, layer);

    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "05L_mask_added_recalculated", testName);

    {
        projection->clear();

        KisPainter painter(projection);
        plane.apply(&painter, imageRect);

        KIS_DUMP_DEVICE_2(projection, imageRect, "06P_apply_on_mask", testName);
    }

    selection->pixelSelection()->select(partialSelectionRect, OPACITY_OPAQUE_U8);

    {
        const QRect changeRect = plane.changeRect(partialSelectionRect, KisLayer::N_FILTHY);
        qDebug() << ppVar(rFillRect) << ppVar(changeRect);

        plane.recalculate(changeRect, layer);

        KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "07L_recalculate_partial", testName);

        KisPainter painter(projection);
        plane.apply(&painter, changeRect);

        KIS_DUMP_DEVICE_2(projection, imageRect, "08P_apply_partial", testName);
    }
}

void KisLayerStyleProjectionPlaneTest::testShadow()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->dropShadow()->setSize(15);
    style->dropShadow()->setDistance(15);
    style->dropShadow()->setOpacity(70);
    style->dropShadow()->setNoise(30);
    style->dropShadow()->setEffectEnabled(true);

    style->innerShadow()->setSize(10);
    style->innerShadow()->setSpread(10);
    style->innerShadow()->setDistance(5);
    style->innerShadow()->setOpacity(70);
    style->innerShadow()->setNoise(30);
    style->innerShadow()->setEffectEnabled(true);

    test(style, "shadow");
}

void KisLayerStyleProjectionPlaneTest::testGlow()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->outerGlow()->setSize(15);
    style->outerGlow()->setSpread(10);
    style->outerGlow()->setOpacity(70);
    style->outerGlow()->setNoise(30);
    style->outerGlow()->setEffectEnabled(true);
    style->outerGlow()->setColor(Qt::green);

    test(style, "glow_outer");
}

#include "KoStopGradient.h"

void KisLayerStyleProjectionPlaneTest::testGlowGradient()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->outerGlow()->setSize(15);
    style->outerGlow()->setSpread(10);
    style->outerGlow()->setOpacity(70);
    style->outerGlow()->setNoise(10);
    style->outerGlow()->setEffectEnabled(true);
    style->outerGlow()->setColor(Qt::green);

    QLinearGradient testGradient;
    testGradient.setColorAt(0.0, Qt::white);
    testGradient.setColorAt(0.5, Qt::green);
    testGradient.setColorAt(1.0, Qt::black);
    testGradient.setSpread(QGradient::ReflectSpread);
    QScopedPointer<KoStopGradient> gradient(
        KoStopGradient::fromQGradient(&testGradient));

    style->outerGlow()->setGradient(gradient.data());
    style->outerGlow()->setFillType(psd_fill_gradient);

    test(style, "glow_outer_grad");
}

void KisLayerStyleProjectionPlaneTest::testGlowGradientJitter()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->outerGlow()->setSize(15);
    style->outerGlow()->setSpread(10);
    style->outerGlow()->setOpacity(70);
    style->outerGlow()->setNoise(0);
    style->outerGlow()->setEffectEnabled(true);
    style->outerGlow()->setColor(Qt::green);

    QLinearGradient testGradient;
    testGradient.setColorAt(0.0, Qt::white);
    testGradient.setColorAt(0.5, Qt::green);
    testGradient.setColorAt(1.0, Qt::black);
    testGradient.setSpread(QGradient::ReflectSpread);
    QScopedPointer<KoStopGradient> gradient(
        KoStopGradient::fromQGradient(&testGradient));

    style->outerGlow()->setGradient(gradient.data());
    style->outerGlow()->setFillType(psd_fill_gradient);
    style->outerGlow()->setJitter(20);

    test(style, "glow_outer_grad_jit");
}

void KisLayerStyleProjectionPlaneTest::testGlowInnerGradient()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->innerGlow()->setSize(15);
    style->innerGlow()->setSpread(10);
    style->innerGlow()->setOpacity(80);
    style->innerGlow()->setNoise(10);
    style->innerGlow()->setEffectEnabled(true);
    style->innerGlow()->setColor(Qt::white);

    QLinearGradient testGradient;
    testGradient.setColorAt(0.0, Qt::white);
    testGradient.setColorAt(0.5, Qt::green);
    testGradient.setColorAt(1.0, Qt::black);
    testGradient.setSpread(QGradient::ReflectSpread);
    QScopedPointer<KoStopGradient> gradient(
        KoStopGradient::fromQGradient(&testGradient));

    style->innerGlow()->setGradient(gradient.data());
    style->innerGlow()->setFillType(psd_fill_gradient);

    test(style, "glow_inner_grad");
}

QTEST_KDEMAIN(KisLayerStyleProjectionPlaneTest, GUI)
