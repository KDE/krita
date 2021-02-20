/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_layer_styles_test.h"

#include <QTest>

#include "kis_transaction.h"

#include <KoColor.h>


#include "layerstyles/kis_layer_style_filter.h"
#include "layerstyles/kis_layer_style_filter_environment.h"
#include "layerstyles/kis_ls_drop_shadow_filter.h"
#include "kis_psd_layer_style.h"
#include "layerstyles/kis_multiple_projection.h"
#include "layerstyles/KisLayerStyleKnockoutBlower.h"

#include <testutil.h>
#include "testimage.h"


struct TestConfig {
    TestConfig()
        : distance(0),
          angle(0),
          spread(0),
          size(0),
          noise(0),
          knocks_out(false),
          keep_original(false)
    {
    }

    int distance;
    int angle;
    int spread;
    int size;
    int noise;
    int knocks_out;
    int opacity;
    bool keep_original;

    void writeProperties(KisPSDLayerStyleSP style) const {
        style->context()->keep_original = keep_original;

        style->dropShadow()->setEffectEnabled(true);
        style->dropShadow()->setDistance(distance);
        style->dropShadow()->setSpread(spread);
        style->dropShadow()->setSize(size);
        style->dropShadow()->setNoise(noise);
        style->dropShadow()->setKnocksOut(knocks_out);
        style->dropShadow()->setOpacity(opacity);
        style->dropShadow()->setAngle(angle);
    }

    QString genTestname(const QString &prefix) const {
        return QString("%1_d_%2_an_%3_sz_%4_spr_%5_nz_%6_ko_%7_keep_%8")
            .arg(prefix)
            .arg(distance)
            .arg(angle)
            .arg(size)
            .arg(spread)
            .arg(noise)
            .arg(knocks_out)
            .arg(keep_original);
    }
};


void testDropShadowImpl(const TestConfig &config,
                        const QVector<QRect> &applyRects,
                        const QString &testName,
                        bool useSeparateDevices)
{
    Q_UNUSED(useSeparateDevices);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QRect srcRect(50, 50, 100, 100);
    QRect dstRect(0, 0, 200, 200);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(srcRect, KoColor(Qt::red, cs));

    KisMultipleProjection projection;

    KisLsDropShadowFilter lsFilter;
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    config.writeProperties(style);



    TestUtil::MaskParent parent;
    KisLayerStyleFilterEnvironment env(parent.layer.data());
    KisLayerStyleKnockoutBlower blower;

    Q_FOREACH (const QRect &rc, applyRects) {
        lsFilter.processDirectly(dev, &projection, &blower, rc, style, &env);
    }

    // drop shadow doesn't use global knockout
    QVERIFY(blower.isEmpty());


    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    projection.apply(dst, dstRect, &env);

    QImage resultImage =
        dst->convertToQImage(0, dstRect);

    TestUtil::checkQImage(resultImage,
                          "layer_styles_test",
                          "common",
                          config.genTestname(testName));
}

void KisLayerStylesTest::testLayerStylesFull()
{
    TestConfig c;
    c.distance = 20;
    c.angle = 135;
    c.spread = 50;
    c.size = 10;
    c.noise = 30;
    c.knocks_out = false;
    c.opacity = 50;
    c.keep_original = false;

    testDropShadowImpl(c, QVector<QRect>() << QRect(0,0,200,200), "full", false);
}

void KisLayerStylesTest::testLayerStylesPartial()
{
    QVector<QRect> rects;

    for (int y = 0; y < 200; y += 50) {
        for (int x = 0; x < 200; x += 50) {
            rects << QRect(x, y, 50, 50);
        }
    }

    TestConfig c;
    c.distance = 20;
    c.angle = 135;
    c.spread = 50;
    c.size = 10;
    c.noise = 30;
    c.knocks_out = false;
    c.opacity = 50;
    c.keep_original = false;

    testDropShadowImpl(c, rects, "partial", true);
}

void KisLayerStylesTest::testLayerStylesPartialVary()
{
    QVector<QRect> rects;

    for (int y = 0; y < 200; y += 50) {
        for (int x = 0; x < 200; x += 50) {
            rects << QRect(x, y, 50, 50);
        }
    }

    TestConfig c;
    c.distance = 20;
    c.angle = 135;
    c.spread = 50;
    c.size = 10;
    c.noise = 30;
    c.knocks_out = false;
    c.opacity = 50;
    c.keep_original = true;

    testDropShadowImpl(c, rects, "partial", true);

    c.noise = 90;
    testDropShadowImpl(c, rects, "partial", true);

    c.noise = 0;
    testDropShadowImpl(c, rects, "partial", true);

    c.noise = 10;
    testDropShadowImpl(c, rects, "partial", true);

    c.angle = 90;
    testDropShadowImpl(c, rects, "partial", true);

    c.angle = 45;
    testDropShadowImpl(c, rects, "partial", true);

    c.knocks_out = true;
    testDropShadowImpl(c, rects, "partial", true);

    c.spread = 90;
    testDropShadowImpl(c, rects, "partial", true);


}

void testDropShadowNeedChangeRects(int distance,
                                   int noise,
                                   int size,
                                   int spread,
                                   const QRect &applyRect,
                                   const QRect &needRect,
                                   const QRect &changeRect)
{
    TestConfig c;
    c.distance = distance;
    c.spread = spread;
    c.size = size;
    c.noise = noise;
    c.angle = 90;
    c.knocks_out = false;
    c.opacity = 50;


    KisLsDropShadowFilter lsFilter;
    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    c.writeProperties(style);


    TestUtil::MaskParent parent;
    KisLayerStyleFilterEnvironment env(parent.layer.data());

    QCOMPARE(lsFilter.neededRect(applyRect, style, &env), needRect);
    QCOMPARE(lsFilter.changedRect(applyRect, style, &env), changeRect);
}

void KisLayerStylesTest::testLayerStylesRects()
{
    QRect applyRect;
    QRect needRect;
    QRect changeRect;

    applyRect = QRect(10,10,10,10);
    needRect = applyRect;
    changeRect = applyRect;
    testDropShadowNeedChangeRects(0, 0, 0, 0, applyRect, needRect, changeRect);

    applyRect = QRect(10,10,10,10);
    needRect = QRect(10,0,10,20);
    changeRect = QRect(10,10,10,20);
    testDropShadowNeedChangeRects(10, 0, 0, 0, applyRect, needRect, changeRect);

    applyRect = QRect(10,10,10,10);
    needRect = QRect(2,2,26,26);
    changeRect = QRect(2,2,26,26);
    testDropShadowNeedChangeRects(0, 30, 0, 0, applyRect, needRect, changeRect);

    applyRect = QRect(10,10,10,10);
    needRect = QRect(-2,-2,34,34);
    changeRect = QRect(-2,-2,34,34);
    testDropShadowNeedChangeRects(0, 0, 10, 0, applyRect, needRect, changeRect);


    applyRect = QRect(10,10,10,10);
    needRect = QRect(-2,-2,34,34);
    changeRect = QRect(-2,-2,34,34);
    testDropShadowNeedChangeRects(0, 0, 10, 50, applyRect, needRect, changeRect);

    applyRect = QRect(10,10,10,10);
    needRect = QRect(-2,-2,34,34);
    changeRect = QRect(-2,-2,34,34);
    testDropShadowNeedChangeRects(0, 0, 10, 75, applyRect, needRect, changeRect);
}

KISTEST_MAIN(KisLayerStylesTest)
