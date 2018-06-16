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
#include <QPointF>

#include <KoColor.h>
#include <testutil.h>

#include "kis_brush_mask_applicator_base.h"
#include "kis_mask_generator.h"
#include "kis_cubic_curve.h"
#include "krita_utils.h"

enum MaskType {
    DEFAULT, CIRC_GAUSS, CIRC_SOFT, RECT_GAUSS, RECT_SOFT, STAMP
};

class KisMaskSimilarityTester
{

public:
    KisMaskSimilarityTester(KisBrushMaskApplicatorBase *_legacy, KisBrushMaskApplicatorBase *_vectorized,  QRect _bounds, MaskType type, bool renderImage = true)
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

        // Start vector processing
        m_paintDev->initialize(255);
        vectorized->initializeData(&data);
        vectorized->process(m_bounds);

        QImage vectorImage(m_paintDev->convertToQImage(m_colorSpace->profile()));
        vectorImage.invertPixels(); // Make pixel color black

        if (renderImage) {
            scalarImage.save(QString(getTypeName(type) + "_scalar_mask.png"),"PNG");
            vectorImage.save(QString(getTypeName(type) +"_vector_mask.png"),"PNG");
        }

        // Check for differences, max errors: 0
        QPoint tmpPt;
        QVERIFY(TestUtil::compareQImages(tmpPt,scalarImage, vectorImage, 0, 2, 0));
    }

private:
    QString getTypeName(MaskType type) {
        QString strName;
        switch (type) {
        case CIRC_GAUSS:
            strName = "CircGauss";
            break;
        case CIRC_SOFT:
            strName = "CircSoft";
            break;
        case RECT_GAUSS:
            strName = "RectGauss";
            break;
        case RECT_SOFT:
            strName = "RectSoft";
            break;
        case STAMP:
            strName = "Stamp";
            break;
        default:
            strName = "Default";
            break;
        }
        return strName;
    }

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
    KisCircleMaskGenerator circVectr(500, 1.0, 0.5, 0.5, 2, true);
    KisCircleMaskGenerator circScalar(circVectr);

    circScalar.resetMaskApplicator(true); // Force usage of scalar backend
    KisMaskSimilarityTester(circScalar.applicator(), circVectr.applicator(), bounds, DEFAULT);
    }
}

void KisMaskSimilarityTest::testGaussCircleMask()
{
    QRect bounds(0,0,500,500);
    {
        KisGaussCircleMaskGenerator circVectr(500, 1.0, .8, .2, 2, true);
        circVectr.setDiameter(500);
        KisGaussCircleMaskGenerator circScalar(circVectr);

        circScalar.resetMaskApplicator(true); // Force usage of scalar backend
        KisMaskSimilarityTester(circScalar.applicator(), circVectr.applicator(), bounds, CIRC_GAUSS);
    }
    // Exahustive test
    for (size_t i = 0; i <= 100; i += 3){
        for (size_t j = 0; j <= 100; j += 3){
            for (size_t k = 0; k <= 100; k += 15){
        {
            KisGaussCircleMaskGenerator circVectr(500, k/100.f, i/100.f, j/100.f, 2, true);
            circVectr.setDiameter(500);
            KisGaussCircleMaskGenerator circScalar(circVectr);

            circScalar.resetMaskApplicator(true); // Force usage of scalar backend
            KisMaskSimilarityTester(circScalar.applicator(), circVectr.applicator(), bounds,CIRC_GAUSS,false);
        }
    } } } // end for
}

void KisMaskSimilarityTest::testSoftCircleMask()
{
    QRect bounds(0,0,500,500);
    KisCubicCurve pointsCurve;
    pointsCurve.fromString(QString("0,1;1,0"));
    {
    KisCurveCircleMaskGenerator circVectr(500, 1.0, 0.5, 0.5, 2, pointsCurve,true);
    circVectr.setDiameter(500);
    // circVectr.setSoftness(1.0);
    KisCurveCircleMaskGenerator circScalar(circVectr);

    circScalar.resetMaskApplicator(true); // Force usage of scalar backend
    KisMaskSimilarityTester(circScalar.applicator(), circVectr.applicator(), bounds, CIRC_SOFT);
    }

    // Exahustive test
    for (size_t i = 0; i <= 100; i += 3){
        for (size_t j = 0; j <= 100; j += 3){
            for (size_t k = 0; k <= 100; k += 15){
        {
            KisCurveCircleMaskGenerator circVectr(500, k/100.f, i/100.f, j/100.f, 2,pointsCurve, true);
            circVectr.setDiameter(500);
            KisCurveCircleMaskGenerator circScalar(circVectr);

            circScalar.resetMaskApplicator(true); // Force usage of scalar backend
            KisMaskSimilarityTester(circScalar.applicator(), circVectr.applicator(), bounds,CIRC_SOFT,false);
        }
    } } } // end for
}

QTEST_MAIN(KisMaskSimilarityTest)
