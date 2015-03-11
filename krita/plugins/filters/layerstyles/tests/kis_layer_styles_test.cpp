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

#include "kis_layer_styles_test.h"

#include <qtest_kde.h>

#include "kis_transaction.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "testutil.h"

#include "../kis_layer_style_filter.h"

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

    void writeProperties(KisFilterConfiguration *kfc) const {
        kfc->setProperty("drop_shadow/distance", distance);
        kfc->setProperty("context/global_angle", angle);
        kfc->setProperty("drop_shadow/spread", spread);
        kfc->setProperty("drop_shadow/size", size);
        kfc->setProperty("drop_shadow/noise", noise);
        kfc->setProperty("drop_shadow/knocks_out", knocks_out);
        kfc->setProperty("drop_shadow/opacity", opacity);
        kfc->setProperty("context/keep_original", keep_original);
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
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QRect srcRect(50, 50, 100, 100);
    QRect dstRect(0, 0, 200, 200);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(srcRect, KoColor(Qt::red, cs));

    KisPaintDeviceSP dst = dev;

    if (useSeparateDevices) {
        dst = new KisPaintDevice(cs);
    }

    KisFilterSP f = KisFilterRegistry::instance()->value("lsdropshadow");
    Q_ASSERT(f);

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT(kfc);

    config.writeProperties(kfc);

    KisTransaction t(dst);

    KisLayerStyleFilter *lsFilter = dynamic_cast<KisLayerStyleFilter*>(f.data());

    foreach (const QRect &rc, applyRects) {
        lsFilter->processDirectly(dev, dst, rc, kfc);
    }

    t.end();

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
    KisFilterSP f = KisFilterRegistry::instance()->value("lsdropshadow");
    Q_ASSERT(f);

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT(kfc);

    kfc->setProperty("drop_shadow/angle", 0); // not used
    kfc->setProperty("drop_shadow/distance", distance);
    kfc->setProperty("context/global_angle", 90);
    kfc->setProperty("drop_shadow/spread", spread);
    kfc->setProperty("drop_shadow/size", size);
    kfc->setProperty("drop_shadow/noise", noise);
    kfc->setProperty("drop_shadow/knocks_out", false);
    kfc->setProperty("drop_shadow/opacity", 50);

    kfc->setProperty("context/keep_original", false);

    KisLayerStyleFilter *lsFilter = dynamic_cast<KisLayerStyleFilter*>(f.data());

    QCOMPARE(lsFilter->neededRect(applyRect, kfc), needRect);
    QCOMPARE(lsFilter->changedRect(applyRect, kfc), changeRect);
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

QTEST_KDEMAIN(KisLayerStylesTest, GUI)
