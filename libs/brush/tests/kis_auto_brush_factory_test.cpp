#include "kis_auto_brush_factory_test.h"

#include <QTest>
#include <testutil.h>
#include "kis_auto_brush.h"
#include "kis_auto_brush_factory.h"
#include "kis_mask_generator.h"
#include <KoColor.h>
#include <brushengine/kis_paint_information.h>

void KisAutoBrushFactoryTest::testXMLClone()
{
    // Set up an autobrush.
    KisBrushSP brush(new KisAutoBrush(new KisCircleMaskGenerator(20, 0.6, 0.8, 0.4, 3, true), 1.0, 0.0));
    brush->setSpacing(0.15);
    brush->setAutoSpacing(true, 0.1);

    // Try to clone the brush by converting to XML and back.
    QDomDocument d;
    QDomElement e = d.createElement("Brush");
    brush->toXML(d, e);
    KisBrushSP clone = KisAutoBrushFactory().createBrush(e);

    // Test that the clone has the same settings as the original brush.
    QCOMPARE(brush->width(), clone->width());
    QCOMPARE(brush->height(), clone->height());
    QCOMPARE(brush->angle(), clone->angle());
    QCOMPARE(brush->spacing(), clone->spacing());
    QCOMPARE(brush->autoSpacingActive(), clone->autoSpacingActive());
    QCOMPARE(brush->autoSpacingCoeff(), clone->autoSpacingCoeff());

    // Test that the clone draws the same as the original brush.

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5);
    KisDabShape shape(0.9, 0.7, 1.0);
    KoColor color(Qt::yellow, cs);

    KisFixedPaintDeviceSP fdev1 = new KisFixedPaintDevice(cs);
    brush->mask(fdev1, color, shape, info);
    QImage res1 = fdev1->convertToQImage(0);

    KisFixedPaintDeviceSP fdev2 = new KisFixedPaintDevice(cs);
    clone->mask(fdev2, color, shape, info);
    QImage res2 = fdev2->convertToQImage(0);

    QCOMPARE(res1, res2);
}

QTEST_MAIN(KisAutoBrushFactoryTest)
