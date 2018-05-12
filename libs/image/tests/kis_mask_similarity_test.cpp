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
#include <KoColor.h>
#include <testutil.h>

#include "kis_brush_mask_applicator_base.h"
#include "kis_mask_generator.h"

class KisMaskSimilarityTester
{
public:
    KisMaskSimilarityTester(KisBrushMaskApplicatorBase* _legacy, KisBrushMaskApplicatorBase* _vectorized, QRect _bounds)
        : legacy(_legacy)
        , vectorized(_vectorized)
        , m_bounds(_bounds)
    {
        KoColor color(Qt::black, colorSpace);
        KisFixedPaintDeviceSP m_paintDev = new KisFixedPaintDevice(colorSpace);

        m_paintDev->setRect(m_bounds);
        m_paintDev->initialize();


        MaskProcessingData data(m_paintDev, colorSpace,
                                0.0, 1.0,
                                m_bounds.width() / 2, m_bounds.height() / 2,0);

        legacy->initializeData(&data);
        legacy->process(m_bounds);

        QImage scalarImage = m_paintDev->convertToQImage(colorSpace->profile());
        scalarImage.save(QString("scalar_v2.png"),"PNG");


//        vectorized->process(bounds);

//        QImage scalarImage = convertMaskToQImage(device, color);

//        KisBrushSP brush = initializeBrush(legacy);
//        KisBrushSP vBrush = initializeBrush(vectorized);



//        QImage scalarImage = convertMaskToQImage(brush, color);
//        QImage vectorImage = convertMaskToQImage(vBrush, color);

//        // Generate images before testing to asses visualy any difference
//        scalarImage.save(QString("scalar_new.png"),"PNG");
//        vectorImage.save(QString("vector2.png"),"PNG");

//        QPoint tmp;
//        QVERIFY(TestUtil::compareQImages(tmp,scalarImage, vectorImage, 0, 5));
//        // Check error deviation between values is less than 0.05
        for (int i = 0; i < scalarImage.width(); ++i) {
            for (int j = 0; j < scalarImage.height(); ++j) {
                qDebug() << scalarImage.pixelColor(i,j);
//                qint16 error(qFabs(scalarImage.pixelColor(i,j).alphaF() - vectorImage.pixelColor(i,j).alphaF()) * 100);
//                QVERIFY(error < 5);
            }
        }
    }

private:
//    KisBrushSP initializeBrush(KisMaskGenerator *mg){
//        KisBrushSP brush = new KisAutoBrush(mg, 1.0, 0.0);
//        brush->setSpacing(0.15);
//        brush->setAutoSpacing(true, 0.1);
//        return brush;
//    };

//    QImage convertMaskToQImage(KisFixedPaintDeviceSP dev, KoColor color){
        //KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(colorSpace);
        //brush->mask(dev, color, shape, info);
//        return dev->convertToQImage(colorSpace->profile());
//    };

protected:
    const KoColorSpace* colorSpace = KoColorSpaceRegistry::instance()->rgb8();
//    KisPaintInformation info = KisPaintInformation(QPointF(40.0, 10.0), 0.5);
//    KisDabShape shape = KisDabShape(1.0,1.0,1.0);

    KisBrushMaskApplicatorBase* legacy;
    KisBrushMaskApplicatorBase* vectorized;
    QRect m_bounds;
    KisFixedPaintDeviceSP m_paintDev;
};


void KisMaskSimilarityTest::testCircleMask()
{
    QRect bounds(0,0,40,40);
    KisMaskSimilarityTester(
        (new KisCircleMaskGenerator(40, 1.0, 0.5, 0.5, 3, true))->applicator(),
        (new KisCircleMaskGenerator(40, 1.0, 0.5, 0.5, 2, true))->applicator(), bounds);
}

//void KisMaskSimilarityTest::testGaussCircleMask()
//{
//    QRect bounds(0,0,40,40);
//    KisMaskSimilarityTester(
//        (new KisGaussCircleMaskGenerator(40, 1.0, 0.5, 0.5, 3, true))->applicator(),
//        (new KisGaussCircleMaskGenerator(40, 1.0, 0.5, 0.5, 2, true))->applicator(), bounds);
//}

QTEST_MAIN(KisMaskSimilarityTest)
