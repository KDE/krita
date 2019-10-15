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

#include <QTest>

#include "testutil.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <resources/KoPattern.h>

#include "kis_transparency_mask.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_painter.h"

#include "kis_selection.h"
#include "kis_pixel_selection.h"

#include "layerstyles/kis_layer_style_projection_plane.h"
#include "kis_psd_layer_style.h"
#include "kis_paint_device_debug_utils.h"


void KisLayerStyleProjectionPlaneTest::test(KisPSDLayerStyleSP style, const QString testName)
{
    const QRect imageRect(0, 0, 200, 200);
    const QRect rFillRect(10, 10, 100, 100);
    const QRect tMaskRect(50, 50, 20, 20);
    const QRect partialSelectionRect(90, 50, 20, 20);

    const QRect updateRect1(10, 10, 50, 100);
    const QRect updateRect2(60, 10, 50, 100);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "styles test");

    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);
    image->addNode(layer);

    KisLayerStyleProjectionPlane plane(layer.data(), style);

    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "00L_initial", testName);

    //layer->paintDevice()->fill(rFillRect, KoColor(Qt::red, cs));
    {
        KisPainter gc(layer->paintDevice());
        gc.setPaintColor(KoColor(Qt::red, cs));
        gc.setFillStyle(KisPainter::FillStyleForegroundColor);
        gc.paintEllipse(rFillRect);
    }

    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "01L_fill", testName);

    KisPaintDeviceSP projection = new KisPaintDevice(cs);

    {
        const QRect changeRect = plane.changeRect(rFillRect, KisLayer::N_FILTHY);
        dbgKrita << ppVar(rFillRect) << ppVar(changeRect);

        plane.recalculate(changeRect, layer);

        KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "02L_recalculate_fill", testName);

        KisPainter painter(projection);
        plane.apply(&painter, changeRect);

        KIS_DUMP_DEVICE_2(projection, imageRect, "03P_apply_on_fill", testName);
    }

    //return;

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
        projection->clear(changeRect);

        dbgKrita << ppVar(partialSelectionRect) << ppVar(changeRect);

        plane.recalculate(changeRect, layer);

        KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "07L_recalculate_partial", testName);

        KisPainter painter(projection);
        plane.apply(&painter, changeRect);

        KIS_DUMP_DEVICE_2(projection, imageRect, "08P_apply_partial", testName);
    }

    // half updates
    transparencyMask->setVisible(false);

    {
        const QRect changeRect = plane.changeRect(updateRect1, KisLayer::N_FILTHY);
        projection->clear(changeRect);

        dbgKrita << ppVar(updateRect1) << ppVar(changeRect);

        plane.recalculate(changeRect, layer);

        KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "09L_recalculate_half1", testName);

        KisPainter painter(projection);
        plane.apply(&painter, changeRect);

        KIS_DUMP_DEVICE_2(projection, imageRect, "10P_apply_half1", testName);
    }

    {
        const QRect changeRect = plane.changeRect(updateRect2, KisLayer::N_FILTHY);
        projection->clear(changeRect);

        dbgKrita << ppVar(updateRect2) << ppVar(changeRect);

        plane.recalculate(changeRect, layer);

        KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "09L_recalculate_half1", testName);

        KisPainter painter(projection);
        plane.apply(&painter, changeRect);

        KIS_DUMP_DEVICE_2(projection, imageRect, "10P_apply_half2", testName);
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

#include <resources/KoStopGradient.h>

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
    QSharedPointer<KoStopGradient> gradient(
        KoStopGradient::fromQGradient(&testGradient));

    style->outerGlow()->setGradient(gradient);
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
    QSharedPointer<KoStopGradient> gradient(
        KoStopGradient::fromQGradient(&testGradient));

    style->outerGlow()->setGradient(gradient);
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
    QSharedPointer<KoStopGradient> gradient(
        KoStopGradient::fromQGradient(&testGradient));

    style->innerGlow()->setGradient(gradient);
    style->innerGlow()->setFillType(psd_fill_gradient);

    test(style, "glow_inner_grad");

    style->innerGlow()->setFillType(psd_fill_solid_color);
    style->innerGlow()->setSource(psd_glow_center);
    test(style, "glow_inner_grad_center");
}

#include <KoCompositeOpRegistry.h>


void KisLayerStyleProjectionPlaneTest::testSatin()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->satin()->setSize(15);
    style->satin()->setOpacity(80);
    style->satin()->setAngle(180);
    style->satin()->setEffectEnabled(true);
    style->satin()->setColor(Qt::white);
    style->satin()->setBlendMode(COMPOSITE_LINEAR_DODGE);

    test(style, "satin");
}

void KisLayerStyleProjectionPlaneTest::testColorOverlay()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->colorOverlay()->setOpacity(80);
    style->colorOverlay()->setEffectEnabled(true);
    style->colorOverlay()->setColor(Qt::white);
    style->colorOverlay()->setBlendMode(COMPOSITE_LINEAR_DODGE);

    test(style, "color_overlay");
}

void KisLayerStyleProjectionPlaneTest::testGradientOverlay()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->gradientOverlay()->setAngle(90);
    style->gradientOverlay()->setOpacity(80);
    style->gradientOverlay()->setEffectEnabled(true);
    style->gradientOverlay()->setBlendMode(COMPOSITE_LINEAR_DODGE);
    style->gradientOverlay()->setAlignWithLayer(true);
    style->gradientOverlay()->setScale(100);
    style->gradientOverlay()->setStyle(psd_gradient_style_diamond);

    QLinearGradient testGradient;
    testGradient.setColorAt(0.0, Qt::white);
    testGradient.setColorAt(0.5, Qt::green);
    testGradient.setColorAt(1.0, Qt::black);
    testGradient.setSpread(QGradient::ReflectSpread);
    QSharedPointer<KoStopGradient> gradient(
        KoStopGradient::fromQGradient(&testGradient));

    style->gradientOverlay()->setGradient(gradient);

    test(style, "grad_overlay");
}

void KisLayerStyleProjectionPlaneTest::testPatternOverlay()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->patternOverlay()->setOpacity(80);
    style->patternOverlay()->setEffectEnabled(true);
    style->patternOverlay()->setBlendMode(COMPOSITE_LINEAR_DODGE);
    style->patternOverlay()->setScale(100);

    style->patternOverlay()->setAlignWithLayer(false);

    QString fileName(TestUtil::fetchDataFileLazy("pattern.pat"));

    KoPatternSP pattern(new KoPattern(fileName));
    QVERIFY(pattern->load());

    style->patternOverlay()->setPattern(pattern);

    test(style, "pat_overlay");
}

void KisLayerStyleProjectionPlaneTest::testStroke()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->stroke()->setColor(Qt::blue);
    style->stroke()->setOpacity(80);
    style->stroke()->setEffectEnabled(true);
    style->stroke()->setBlendMode(COMPOSITE_OVER);

    style->stroke()->setSize(3);
    style->stroke()->setPosition(psd_stroke_center);

    test(style, "stroke_col_ctr");

    style->stroke()->setPosition(psd_stroke_outside);

    test(style, "stroke_col_out");

    style->stroke()->setPosition(psd_stroke_inside);

    test(style, "stroke_col_in");


    QString fileName(TestUtil::fetchDataFileLazy("pattern.pat"));
    KoPatternSP pattern(new KoPattern(fileName));
    QVERIFY(pattern->load());
    style->stroke()->setPattern(pattern);
    style->stroke()->setFillType(psd_fill_pattern);

    test(style, "stroke_pat");


    QLinearGradient testGradient;
    testGradient.setColorAt(0.0, Qt::white);
    testGradient.setColorAt(0.5, Qt::green);
    testGradient.setColorAt(1.0, Qt::black);
    testGradient.setSpread(QGradient::ReflectSpread);
    QSharedPointer<KoStopGradient> gradient(
        KoStopGradient::fromQGradient(&testGradient));

    style->stroke()->setGradient(gradient);
    style->stroke()->setFillType(psd_fill_gradient);

    test(style, "stroke_grad");

}

#include "layerstyles/gimp_bump_map.h"

void KisLayerStyleProjectionPlaneTest::testBumpmap()
{
    KisPixelSelectionSP device = new KisPixelSelection();

    const int numCycles = 30;
    const int step = 5;

    QRect applyRect(200, 100, 100, 100);
    QRect fillRect(210, 110, 80, 80);
    quint8 selectedness = 256 - numCycles * step;


    for (int i = 0; i < numCycles; i++) {
        device->select(fillRect, selectedness);

        fillRect = kisGrowRect(fillRect, -1);
        selectedness += step;
    }

    KIS_DUMP_DEVICE_2(device, applyRect, "00_initial", "bumpmap");


    bumpmap_vals_t bmvals;

    bmvals.azimuth = 240;
    bmvals.elevation = 30;
    bmvals.depth = 50;
    bmvals.ambient = 128;
    bmvals.compensate = false;
    bmvals.invert = false;
    bmvals.type = 0;

    bumpmap(device, applyRect, bmvals);

    KIS_DUMP_DEVICE_2(device, applyRect, "01_bumpmapped", "bumpmap");

}

void KisLayerStyleProjectionPlaneTest::testBevel()
{
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->bevelAndEmboss()->setEffectEnabled(true);

    style->bevelAndEmboss()->setAngle(135);
    style->bevelAndEmboss()->setAltitude(45);
    style->bevelAndEmboss()->setDepth(100);

    style->bevelAndEmboss()->setHighlightColor(Qt::white);
    style->bevelAndEmboss()->setHighlightBlendMode(COMPOSITE_OVER);
    style->bevelAndEmboss()->setHighlightOpacity(100);

    style->bevelAndEmboss()->setShadowColor(Qt::black);
    style->bevelAndEmboss()->setShadowBlendMode(COMPOSITE_OVER);
    style->bevelAndEmboss()->setShadowOpacity(100);

    QString fileName(TestUtil::fetchDataFileLazy("pattern.pat"));
    KoPatternSP pattern(new KoPattern(fileName));
    QVERIFY(pattern->load());

    style->bevelAndEmboss()->setTexturePattern(pattern);

    style->bevelAndEmboss()->setTextureEnabled(true);
    style->bevelAndEmboss()->setTextureDepth(-10);
    style->bevelAndEmboss()->setTextureInvert(false);

    style->bevelAndEmboss()->setStyle(psd_bevel_outer_bevel);
    style->bevelAndEmboss()->setDirection(psd_direction_up);
    style->bevelAndEmboss()->setSoften(0);
    test(style, "bevel_outer_up");

    style->bevelAndEmboss()->setTextureInvert(true);
    style->bevelAndEmboss()->setStyle(psd_bevel_outer_bevel);
    style->bevelAndEmboss()->setDirection(psd_direction_up);
    style->bevelAndEmboss()->setSoften(0);
    test(style, "bevel_outer_up_invert_texture");
    style->bevelAndEmboss()->setTextureInvert(false);

    style->bevelAndEmboss()->setStyle(psd_bevel_outer_bevel);
    style->bevelAndEmboss()->setDirection(psd_direction_down);
    style->bevelAndEmboss()->setSoften(0);
    test(style, "bevel_outer_down");

    style->bevelAndEmboss()->setStyle(psd_bevel_emboss);
    style->bevelAndEmboss()->setDirection(psd_direction_up);
    style->bevelAndEmboss()->setSoften(0);
    test(style, "bevel_emboss_up");

    style->bevelAndEmboss()->setStyle(psd_bevel_pillow_emboss);
    style->bevelAndEmboss()->setDirection(psd_direction_up);
    style->bevelAndEmboss()->setSoften(0);
    test(style, "bevel_pillow_up");

    style->bevelAndEmboss()->setStyle(psd_bevel_pillow_emboss);
    style->bevelAndEmboss()->setDirection(psd_direction_down);
    style->bevelAndEmboss()->setSoften(0);
    test(style, "bevel_pillow_down");

    style->bevelAndEmboss()->setStyle(psd_bevel_pillow_emboss);
    style->bevelAndEmboss()->setDirection(psd_direction_up);
    style->bevelAndEmboss()->setSoften(3);
    test(style, "bevel_pillow_up_soft");
}

#include "kis_ls_utils.h"

void KisLayerStyleProjectionPlaneTest::testBlending()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP layer = new KisPaintDevice(cs);
    KisPaintDeviceSP overlay = new KisPaintDevice(cs);
    KisPaintDeviceSP bg = new KisPaintDevice(cs);
    KisPaintDeviceSP result = new KisPaintDevice(cs);

    const int width = 20;
    KoColor color(Qt::transparent, cs);

    QVector<QColor> layerColors;
    QVector<QColor> overlayColors;
    QVector<QColor> bgColors;

    layerColors << QColor(0,   255, 0);
    layerColors << QColor(128, 255, 64);

    overlayColors << QColor(255,   0, 0);
    overlayColors << QColor(255, 128, 64);

    bgColors << QColor(0, 0, 0, 0);
    bgColors << QColor(0, 0, 0, 255);
    bgColors << QColor(255, 255, 255, 255);
    bgColors << QColor(64, 128, 255, 255);

    bgColors << QColor(0, 0, 0, 128);
    bgColors << QColor(255, 255, 255, 128);
    bgColors << QColor(64, 128, 255, 128);

    const int overlayOpacity = 255;
    const int layerOpacity = 255;
    int y = 1;
    Q_FOREACH(const QColor &layerColor, layerColors) {
        Q_FOREACH(const QColor &overlayColor, overlayColors) {
            Q_FOREACH(const QColor &bgColor, bgColors) {
                bg->setPixel(0, y, layerColor);
                bg->setPixel(1, y, overlayColor);
                bg->setPixel(2, y, bgColor);
                bg->setPixel(3, y, QColor(layerOpacity, layerOpacity, layerOpacity, 255));
                bg->setPixel(4, y, QColor(overlayOpacity, overlayOpacity, overlayOpacity, 255));

                for (int i = 5; i < width; i++) {
                    bg->setPixel(i, y, bgColor);
                }

                for (int i = 0; i <= 10; i++) {
                    const quint8 alpha = i == 0 ? 71 : qRound(255 * qreal(i) / 10);

                    {
                        QColor c(layerColor);
                        c.setAlpha(alpha);
                        layer->setPixel(7 + i, y, c);
                    }

                    {
                        QColor c(overlayColor);
                        c.setAlpha(alpha);
                        overlay->setPixel(7 + i, y, c);
                    }
                }

                y++;
            }
        }
    }

    const QRect rc = bg->exactBounds() | layer->exactBounds();


    KIS_DUMP_DEVICE_2(layer, rc, "00_layer", "dd");
    KIS_DUMP_DEVICE_2(overlay, rc, "01_overlay", "dd");
    KIS_DUMP_DEVICE_2(bg, rc, "02_bg", "dd");

    KisPaintDeviceSP originalBg = new KisPaintDevice(*bg);

    KisSelectionSP selection = new KisSelection();
    KisLsUtils::selectionFromAlphaChannel(layer, selection, rc);

    {
        KisSequentialIterator it(layer, rc);
        while (it.nextPixel()) {
            cs->setOpacity(it.rawData(), quint8(255), 1);
        }
    }

    {
        KisSequentialIterator it(overlay, rc);
        while (it.nextPixel()) {
            cs->setOpacity(it.rawData(), quint8(255), 1);
        }
    }

    KisPainter painter(bg);

    painter.setOpacity(layerOpacity);
    painter.setCompositeOp(COMPOSITE_OVER);

    painter.bitBlt(rc.topLeft(), layer, rc);

    painter.setOpacity(overlayOpacity);
    painter.setCompositeOp(COMPOSITE_ADD);

    painter.bitBlt(rc.topLeft(), overlay, rc);

    KIS_DUMP_DEVICE_2(bg, rc, "03_result", "dd");

    KisPainter bgPainter(originalBg);
    bgPainter.setCompositeOp(COMPOSITE_COPY);
    bgPainter.setSelection(selection);
    bgPainter.bitBlt(rc.topLeft(), bg, rc);

    KIS_DUMP_DEVICE_2(originalBg, rc, "04_knockout", "dd");
}

QTEST_MAIN(KisLayerStyleProjectionPlaneTest)
