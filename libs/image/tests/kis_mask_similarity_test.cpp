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
        scalarImage.invertPixels(); // Make pixel color black
        scalarImage.save(QString("scalar_mask.png"),"PNG");

        // Start vector processing
        m_paintDev->initialize(255);
        vectorized->initializeData(&data);
        vectorized->process(m_bounds);

        QImage vectorImage(m_paintDev->convertToQImage(m_colorSpace->profile()));
        vectorImage.invertPixels(); // Make pixel color black
        vectorImage.save(QString("vector_mask.png"),"PNG");

// Development debug.
// Count number of identical pixels
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

        // Check for differences, max error .5% of pixel mismatch
        int tolerance = m_bounds.width() * m_bounds.height() * .005f;
        QPoint tmpPt;
        QVERIFY(TestUtil::compareQImages(tmpPt,scalarImage, vectorImage, 0, 1, tolerance));
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
    QRect bounds(0,0,500,500);
    {
    KisCircleMaskGenerator circVectr(480, 1.0, 0.5, 0.5, 2, true);
    KisCircleMaskGenerator circScalar(circVectr);

    circScalar.resetMaskApplicator(true); // Force usage of scalar backend
    KisMaskSimilarityTester(circScalar.applicator(), circVectr.applicator(), bounds);
    }
}

void KisMaskSimilarityTest::testGaussCircleMask()
{
    QRect bounds(0,0,500,500);
    {
    KisGaussCircleMaskGenerator circVectr(480, 1.0, 0.5, 0.5, 2, true);
    circVectr.setDiameter(480);
    KisGaussCircleMaskGenerator circScalar(circVectr);

    circScalar.resetMaskApplicator(true); // Force usage of scalar backend
    KisMaskSimilarityTester(circScalar.applicator(), circVectr.applicator(), bounds);
    }
}

QTEST_MAIN(KisMaskSimilarityTest)
