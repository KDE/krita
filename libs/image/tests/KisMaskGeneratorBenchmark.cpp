/*
 *  SPDX-FileCopyrightText: 2018 Iván Santa María <ghevan@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <simpletest.h>
#include <KoColor.h>
#include <QElapsedTimer>

#include "KisMaskGeneratorBenchmark.h"

#include <kis_mask_generator.h>
#include "kis_brush_mask_applicator_base.h"
#include "krita_utils.h"

#include <testutil.h>

class KisMaskGeneratorBenchmarkTester
{
public:
    KisMaskGeneratorBenchmarkTester(KisBrushMaskApplicatorBase *_applicatorBase, QRect _bounds)
        : m_bounds(_bounds),
          applicatorBase(_applicatorBase)
    {
        KisFixedPaintDeviceSP m_paintDev = new KisFixedPaintDevice(m_colorSpace);
        m_paintDev->setRect(m_bounds);
        m_paintDev->initialize(255);

        MaskProcessingData data(m_paintDev, m_colorSpace, nullptr,
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

void KisMaskGeneratorBenchmark::testDefaultScalarMask()
{
    QRect bounds(0,0,1000,1000);
    {
        KisCircleMaskGenerator circScalar(1000, 1.0, 0.5, 0.5, 2, true);
        circScalar.setMaskScalarApplicator(); // Force usage of scalar backend

        KisMaskGeneratorBenchmarkTester(circScalar.applicator(), bounds);
    }
}

void KisMaskGeneratorBenchmark::testDefaultVectorMask()
{
    QRect bounds(0,0,1000,1000);
    {
        KisCircleMaskGenerator circVectr(1000, 1.0, 0.5, 0.5, 2, true);
        KisMaskGeneratorBenchmarkTester(circVectr.applicator(), bounds);
    }
}

void KisMaskGeneratorBenchmark::testCircularGaussScalarMask()
{
    QRect bounds(0,0,1000,1000);
    {
    KisGaussCircleMaskGenerator circScalar(1000, 1.0, 0.5, 0.5, 2, true);
    circScalar.setDiameter(1000);
    circScalar.setMaskScalarApplicator(); // Force usage of scalar backend

    KisMaskGeneratorBenchmarkTester(circScalar.applicator(), bounds);
    }
}

void KisMaskGeneratorBenchmark::testCircularGaussVectorMask()
{
    QRect bounds(0,0,1000,1000);
    {
    KisGaussCircleMaskGenerator circVectr(1000, 1.0, 0.5, 0.5, 2, true);
    circVectr.setDiameter(1000);
    KisMaskGeneratorBenchmarkTester(circVectr.applicator(), bounds);
    }
}

void KisMaskGeneratorBenchmark::testCircularSoftScalarMask()
{
    QRect bounds(0,0,1000,1000);
    const KisCubicCurve pointsCurve(QString("0,1;1,0"));
    {
    KisCurveCircleMaskGenerator circScalar(1000, 1.0, 0.5, 0.5, 2, pointsCurve, true);
    circScalar.setSoftness(0.5);
    circScalar.setMaskScalarApplicator(); // Force usage of scalar backend

    KisMaskGeneratorBenchmarkTester(circScalar.applicator(), bounds);
    }
}

void KisMaskGeneratorBenchmark::testCircularSoftVectorMask()
{
    QRect bounds(0,0,1000,1000);
    const KisCubicCurve pointsCurve(QString("0,1;1,0"));
    {
    KisCurveCircleMaskGenerator circVectr(1000, 1.0, 0.5, 0.5, 2, pointsCurve, true);
    circVectr.setSoftness(0.5);
    KisMaskGeneratorBenchmarkTester(circVectr.applicator(), bounds);
    }
}

void KisMaskGeneratorBenchmark::testRectangularScalarMask(){
    QRect bounds(0,0,1000,1000);
    {
        KisRectangleMaskGenerator rectScalar(1000, 1.0, 0.5, 0.5, 2, true);
        rectScalar.setMaskScalarApplicator(); // Force usage of scalar backend

        KisMaskGeneratorBenchmarkTester(rectScalar.applicator(), bounds);
    }
}

void KisMaskGeneratorBenchmark::testRectangularVectorMask(){
    QRect bounds(0,0,1000,1000);
    {
        KisRectangleMaskGenerator rectScalar(1000, 1.0, 0.5, 0.5, 2, true);
        KisMaskGeneratorBenchmarkTester(rectScalar.applicator(), bounds);
    }
}

void KisMaskGeneratorBenchmark::testRectangularGaussScalarMask()
{
    QRect bounds(0,0,1000,1000);
    {
    KisGaussRectangleMaskGenerator circScalar(1000, 1.0, 0.5, 0.5, 2, true);
//    circScalar.setDiameter(1000);
    circScalar.setMaskScalarApplicator(); // Force usage of scalar backend

    KisMaskGeneratorBenchmarkTester(circScalar.applicator(), bounds);
    }
}
void KisMaskGeneratorBenchmark::testRectangularGaussVectorMask()
{
    QRect bounds(0,0,1000,1000);
    {
    KisGaussRectangleMaskGenerator circVectr(1000, 1.0, 0.5, 0.5, 2, true);
//    circVectr.setDiameter(1000);
    KisMaskGeneratorBenchmarkTester(circVectr.applicator(), bounds);
    }
}

void KisMaskGeneratorBenchmark::testRectangularSoftScalarMask()
{
    QRect bounds(0,0,1000,1000);
    const KisCubicCurve pointsCurve(QString("0,1;1,0"));
    {
    KisCurveRectangleMaskGenerator circScalar(1000, 1.0, 0.5, 0.5, 2, pointsCurve, true);

    circScalar.setMaskScalarApplicator(); // Force usage of scalar backend

    KisMaskGeneratorBenchmarkTester(circScalar.applicator(), bounds);
    }
}
void KisMaskGeneratorBenchmark::testRectangularSoftVectorMask()
{
    QRect bounds(0,0,1000,1000);
    const KisCubicCurve pointsCurve(QString("0,1;1,0"));
    {
    KisCurveRectangleMaskGenerator circVectr(1000, 1.0, 0.5, 0.5, 2, pointsCurve, true);

    KisMaskGeneratorBenchmarkTester(circVectr.applicator(), bounds);
    }
}

SIMPLE_TEST_MAIN(KisMaskGeneratorBenchmark)
