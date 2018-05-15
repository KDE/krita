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
#include "krita_utils.h"


class KisMaskSimilarityTester
{
public:
    KisMaskSimilarityTester(KisBrushMaskApplicatorBase *_legacy, KisBrushMaskApplicatorBase *_vectorized, QRect _bounds)
        : legacy(_legacy)
        , vectorized(_vectorized)
        , m_bounds(_bounds)
    {
        KisFixedPaintDeviceSP m_paintDev = new KisFixedPaintDevice(m_colorSpace);
        m_paintDev->setRect(m_bounds);
        m_paintDev->initialize(255);

        MaskProcessingData data(m_paintDev, m_colorSpace,
                                0.0, 1.0,
                                m_bounds.width() / 2.0, m_bounds.height() / 2.0,0);

        // Start legacy scalar processing
        legacy->initializeData(&data);
        legacy->process(m_bounds);

        QImage scalarImage(m_paintDev->convertToQImage(m_colorSpace->profile()));
        scalarImage.invertPixels();
        scalarImage.save(QString("scalar_mask.png"),"PNG");

        // Start vector processing
        m_paintDev->initialize(255);
        vectorized->initializeData(&data);
//        QVector<QRect> rects = KritaUtils::splitRectIntoPatches(m_paintDev->bounds(), QSize(3, 3));
//        Q_FOREACH (const QRect &rc, rects) {
//            vectorized->process(rc);
//        }
        vectorized->process(m_bounds);

        QImage vectorImage(m_paintDev->convertToQImage(m_colorSpace->profile()));
        vectorImage.invertPixels();
        vectorImage.save(QString("vector_mask.png"),"PNG");

        // Check for differences, max error .5% of pixel mismatch
        int tolerance(m_bounds.width() * m_bounds.height() * .005f);
        // qDebug() << "tolerance: " << tolerance;
        QPoint tmpPt;
        QVERIFY(TestUtil::compareQImages(tmpPt,scalarImage, vectorImage, 0, tolerance));

        // Check error deviation between values is less than 0.05
// Development debug.
//        int equals = 0;
//        for (int i = 0; i < scalarImage.width(); ++i) {
//            for (int j = 0; j < scalarImage.height(); ++j) {
//                if (scalarImage.pixelColor(i,j) == vectorImage.pixelColor(i,j)){
//                    equals++;
//                } else {
//                    qDebug() << scalarImage.pixelColor(i,j) << " " << vectorImage.pixelColor(i,j);
//                }
//            }
//        }
//        qDebug() << "Equal Pixels: " << equals;
//        qDebug() << scalarImage;
//        qDebug() << vectorImage;
    }

private:

protected:
    const KoColorSpace *m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();

    KisBrushMaskApplicatorBase *legacy;
    KisBrushMaskApplicatorBase *vectorized;
    QRect m_bounds;
    KisFixedPaintDeviceSP m_paintDev;
};


void KisMaskSimilarityTest::testCircleMask()
{
//    QRect rect500(0,0,500,500);
//    {
//    KisCircleMaskGenerator circScalar(250, 1.0, 0.5, 0.5, 2, true);
//    KisCircleMaskGenerator circVectr(250, 1.0, 0.5, 0.5, 2, true);
//    KisMaskSimilarityTester(circScalar.applicator(), circVectr.applicator(), rect500);
//    }

    QRect rect40(0,0,40,40);
    {
    KisCircleMaskGenerator circVectr(20, 1.0, 0.5, 0.5, 2, true);
    KisCircleMaskGenerator circScalar(circVectr);

    circScalar.resetMaskApplicator(true);
    KisMaskSimilarityTester(circScalar.applicator(), circVectr.applicator(), rect40);

    circVectr.resetMaskApplicator(true);
    KisMaskSimilarityTester(circScalar.applicator(), circVectr.applicator(), rect40);
    }
}

//void KisMaskSimilarityTest::testGaussCircleMask()
//{
//    QRect bounds(0,0,40,40);
//    KisMaskSimilarityTester(
//        (new KisGaussCircleMaskGenerator(40, 1.0, 0.5, 0.5, 3, true))->applicator(),
//        (new KisGaussCircleMaskGenerator(40, 1.0, 0.5, 0.5, 2, true))->applicator(), bounds);
//}

QTEST_MAIN(KisMaskSimilarityTest)
