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

#include <QTest>
#include <QPointF>
#include <KoColor.h>
#include <QElapsedTimer>

#include "KisMaskGeneratorTest.h"

#include "kis_mask_similarity_test.h"

#include "kis_brush_mask_applicator_base.h"
#include "kis_mask_generator.h"
#include "kis_cubic_curve.h"
#include "krita_utils.h"

#include "testutil.h"

class KisMaskGeneratorTestTester
{
public:
    KisMaskGeneratorTestTester(KisBrushMaskApplicatorBase *_applicatorBase, QRect _bounds)
        : m_bounds(_bounds),
          applicatorBase(_applicatorBase)
    {
        KisFixedPaintDeviceSP m_paintDev = new KisFixedPaintDevice(m_colorSpace);
        m_paintDev->setRect(m_bounds);
        m_paintDev->initialize(255);

        MaskProcessingData data(m_paintDev, m_colorSpace,
                                0.0, 1.0,
                                m_bounds.width() / 2.0, m_bounds.height() / 2.0,0);

        // Start Benchmark
        applicatorBase->initializeData(&data);

        QElapsedTimer maskGenerationTime;
        maskGenerationTime.start();

        QBENCHMARK {

            applicatorBase->process(m_bounds);
        }
    }

protected:
    const KoColorSpace *m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    QRect m_bounds;

    KisBrushMaskApplicatorBase *applicatorBase;
    KisFixedPaintDeviceSP m_paintDev;
};

void KisMaskGeneratorTest::testDefaultScalarMask()
{
    QRect bounds(0,0,1000,1000);
    {
        KisCircleMaskGenerator circScalar(1000, 1.0, 0.5, 0.5, 2, true);
        circScalar.resetMaskApplicator(true); // Force usage of scalar backend

        KisMaskGeneratorTestTester(circScalar.applicator(), bounds);
    }
}

void KisMaskGeneratorTest::testDefaultVectorMask()
{
    QRect bounds(0,0,1000,1000);
    {
        KisCircleMaskGenerator circVectr(1000, 1.0, 0.5, 0.5, 2, true);
        KisMaskGeneratorTestTester(circVectr.applicator(), bounds);
    }
}

void KisMaskGeneratorTest::testCircularGaussScalarMask()
{
    QRect bounds(0,0,1000,1000);
    {
    KisGaussCircleMaskGenerator circScalar(1000, 1.0, 0.5, 0.5, 2, true);
    circScalar.setDiameter(1000);
    circScalar.resetMaskApplicator(true); // Force usage of scalar backend

    KisMaskGeneratorTestTester(circScalar.applicator(), bounds);
    }
}

void KisMaskGeneratorTest::testCircularGaussVectorMask()
{
    QRect bounds(0,0,1000,1000);
    {
    KisGaussCircleMaskGenerator circVectr(1000, 1.0, 0.5, 0.5, 2, true);
    circVectr.setDiameter(1000);
    KisMaskGeneratorTestTester(circVectr.applicator(), bounds);
    }
}

void KisMaskGeneratorTest::testCircularSoftScalarMask()
{
    QRect bounds(0,0,1000,1000);
    KisCubicCurve pointsCurve;
    pointsCurve.fromString(QString("0,1;1,0"));
    {
    KisCurveCircleMaskGenerator circScalar(1000, 1.0, 0.5, 0.5, 2, pointsCurve, true);
    circScalar.setSoftness(0.5);
    circScalar.resetMaskApplicator(true); // Force usage of scalar backend

    KisMaskGeneratorTestTester(circScalar.applicator(), bounds);
    }
}

void KisMaskGeneratorTest::testCircularSoftVectorMask()
{
    QRect bounds(0,0,1000,1000);
    KisCubicCurve pointsCurve;
    pointsCurve.fromString(QString("0,1;1,0"));
    {
    KisCurveCircleMaskGenerator circVectr(1000, 1.0, 0.5, 0.5, 2, pointsCurve, true);
    circVectr.setSoftness(0.5);
    KisMaskGeneratorTestTester(circVectr.applicator(), bounds);
    }
}

void KisMaskGeneratorTest::testRectangularScalarMask(){
    QRect bounds(0,0,1000,1000);
    {
        KisRectangleMaskGenerator rectScalar(1000, 1.0, 0.5, 0.5, 2, true);
        rectScalar.resetMaskApplicator(true); // Force usage of scalar backend

        KisMaskGeneratorTestTester(rectScalar.applicator(), bounds);
    }
}

void KisMaskGeneratorTest::testRectangularVectorMask(){
    QRect bounds(0,0,1000,1000);
    {
        KisRectangleMaskGenerator rectScalar(1000, 1.0, 0.5, 0.5, 2, true);
        KisMaskGeneratorTestTester(rectScalar.applicator(), bounds);
    }
}

void KisMaskGeneratorTest::testRectangularGaussScalarMask()
{
    QRect bounds(0,0,1000,1000);
    {
    KisGaussRectangleMaskGenerator circScalar(1000, 1.0, 0.5, 0.5, 2, true);
//    circScalar.setDiameter(1000);
    circScalar.resetMaskApplicator(true); // Force usage of scalar backend

    KisMaskGeneratorTestTester(circScalar.applicator(), bounds);
    }
}
void KisMaskGeneratorTest::testRectangularGaussVectorMask()
{
    QRect bounds(0,0,1000,1000);
    {
    KisGaussRectangleMaskGenerator circVectr(1000, 1.0, 0.5, 0.5, 2, true);
//    circVectr.setDiameter(1000);
    KisMaskGeneratorTestTester(circVectr.applicator(), bounds);
    }
}

void KisMaskGeneratorTest::testRectangularSoftScalarMask()
{
    QRect bounds(0,0,1000,1000);
    KisCubicCurve pointsCurve;
    pointsCurve.fromString(QString("0,1;1,0"));
    {
    KisCurveRectangleMaskGenerator circScalar(1000, 1.0, 0.5, 0.5, 2, pointsCurve, true);

    circScalar.resetMaskApplicator(true); // Force usage of scalar backend

    KisMaskGeneratorTestTester(circScalar.applicator(), bounds);
    }
}
void KisMaskGeneratorTest::testRectangularSoftVectorMask()
{
    QRect bounds(0,0,1000,1000);
    KisCubicCurve pointsCurve;
    pointsCurve.fromString(QString("0,1;1,0"));
    {
    KisCurveRectangleMaskGenerator circVectr(1000, 1.0, 0.5, 0.5, 2, pointsCurve, true);

    KisMaskGeneratorTestTester(circVectr.applicator(), bounds);
    }
}

QTEST_MAIN(KisMaskGeneratorTest)
