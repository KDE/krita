/*
 *  Copyright (c) 2018 Iván Santa María <ghevan@gmail.com>
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

#include "kis_mask_similarity_test.h"

#include <QTest>
#include <QtMath>
#include <KoColor.h>

#include <testutil.h>
#include "kis_auto_brush.h"
#include "kis_auto_brush_factory.h"
#include <brushengine/kis_paint_information.h>
#include "kis_mask_generator.h"

class KisMaskSimilarityTester
{
public:
    KisMaskSimilarityTester(KisMaskGenerator* _legacy, KisMaskGenerator* _vectorized)
        : legacy(_legacy)
        , vectorized(_vectorized)
    {
        int size = legacy->diameter();

        KisBrushSP brush = initializeBrush(legacy);
        KisBrushSP vBrush = initializeBrush(vectorized);

        KoColor color(Qt::black, colorSpace);

        QImage scalarImage = convertMaskToQImage(brush, color);
        QImage vectorImage = convertMaskToQImage(vBrush, color);

        // Generate images before testing to asses visualy any difference
        scalarImage.save(QString("scalar2.png"),"PNG");
        vectorImage.save(QString("vector2.png"),"PNG");

        QPoint tmp;
        QVERIFY(TestUtil::compareQImages(tmp,scalarImage, vectorImage, 0, 5));
        // Check error deviation between values is less than 0.05
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                qint16 error(qFabs(scalarImage.pixelColor(i,j).alphaF() - vectorImage.pixelColor(i,j).alphaF()) * 100);
                QVERIFY(error < 5);
            }
        }
    }

private:
    KisBrushSP initializeBrush(KisMaskGenerator *mg){
        KisBrushSP brush = new KisAutoBrush(mg, 1.0, 0.0);
        brush->setSpacing(0.15);
        brush->setAutoSpacing(true, 0.1);
        return brush;
    };

    QImage convertMaskToQImage(KisBrushSP brush, KoColor color){
        KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(colorSpace);
        brush->mask(dev, color, shape, info);
        return dev->convertToQImage(colorSpace->profile());
    };

protected:
    const KoColorSpace* colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintInformation info = KisPaintInformation(QPointF(40.0, 10.0), 0.5);
    KisDabShape shape = KisDabShape(1.0,1.0,1.0);

    KisMaskGenerator* legacy;
    KisMaskGenerator* vectorized;
    KisFixedPaintDeviceSP m_paintDevice;
};


void KisMaskSimilarityTest::testCircleMask()
{
    KisMaskSimilarityTester(
        new KisCircleMaskGenerator(40, 1.0, 0.5, 0.5, 3, true),
        new KisCircleMaskGenerator(40, 1.0, 0.5, 0.5, 2, true));
}

void KisMaskSimilarityTest::testGaussCircleMask()
{
    KisMaskSimilarityTester(
        new KisGaussCircleMaskGenerator(40, 1.0, 0.5, 0.5, 3, true),
        new KisGaussCircleMaskGenerator(40, 1.0, 0.5, 0.5, 2, true));
}

QTEST_MAIN(KisMaskSimilarityTest)
