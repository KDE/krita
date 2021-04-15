/*
 *  SPDX-FileCopyrightText: 2018 Iván Santa María <ghevan@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mask_similarity_test.h"

#include <simpletest.h>
#include <QPointF>

#include <KoColor.h>
#include <testutil.h>

#include "kis_brush_mask_applicator_base.h"
#include "kis_mask_generator.h"
#include "kis_cubic_curve.h"
#include "krita_utils.h"

#include "config-limit-long-tests.h"

#ifdef LIMIT_LONG_TESTS
#define RATIO_STEP 50
#define FADE_STEP 25
#else /* LIMIT_LONG_TESTS */
#define RATIO_STEP 20
#define FADE_STEP 5
#endif /* LIMIT_LONG_TESTS */


enum MaskType {
    DEFAULT, CIRC_GAUSS, CIRC_SOFT, RECT, RECT_GAUSS, RECT_SOFT, STAMP
};

class KisMaskSimilarityTester
{

public:
    KisMaskSimilarityTester(KisBrushMaskApplicatorBase *_legacy, KisBrushMaskApplicatorBase *_vectorized,  QRect _bounds, MaskType type)
        : legacy(_legacy)
        , vectorized(_vectorized)
        , m_bounds(_bounds)
    {
        KisFixedPaintDeviceSP m_paintDev = new KisFixedPaintDevice(m_colorSpace);
        m_paintDev->setRect(m_bounds);
        m_paintDev->initialize(255);

        MaskProcessingData data(m_paintDev, m_colorSpace,
                                nullptr,
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

        // Check for differences, max errors: 0
        QPoint tmpPt;
        if (!TestUtil::compareQImages(tmpPt,scalarImage, vectorImage, 0, 2, 0)) {
            scalarImage.save(QString(getTypeName(type) + "_scalar_mask.png"),"PNG");
            vectorImage.save(QString(getTypeName(type) + "_vector_mask.png"),"PNG");

            QFAIL(QString("Masks differ! first different pixel: %1,%2 \n").arg(tmpPt.x()).arg(tmpPt.y()).toLatin1());
        }

    }


    static bool exahustiveTest(QRect bounds, MaskType type) {
        // Exahustive test

        for (size_t i = 0; i <= 100; i += FADE_STEP){
            for (size_t j = 0; j <= 100; j += FADE_STEP){
                for (size_t k = 0; k <= 100; k += RATIO_STEP){

                switch (type) {
                case CIRC_GAUSS:
                    {
                    KisGaussCircleMaskGenerator bCircVectr(499.5, k/100.f, i/100.f, j/100.f, 2, true);
                    bCircVectr.setDiameter(499.5);
                    KisGaussCircleMaskGenerator bCircScalar(bCircVectr);
                    bCircScalar.resetMaskApplicator(true); // Force usage of scalar backend

                    KisMaskSimilarityTester(bCircScalar.applicator(), bCircVectr.applicator(), bounds,type);
                    break;
                    }
                case CIRC_SOFT:
                    {
                    KisCubicCurve pointsCurve;
                    pointsCurve.fromString(QString("0,1;1,0"));
                    KisCurveCircleMaskGenerator bCircVectr(499.5, k/100.f, i/100.f, j/100.f, 2, pointsCurve, true);
                    bCircVectr.setDiameter(499.5);
                    KisCurveCircleMaskGenerator bCircScalar(bCircVectr);
                    bCircScalar.resetMaskApplicator(true); // Force usage of scalar backend

                    KisMaskSimilarityTester(bCircScalar.applicator(), bCircVectr.applicator(), bounds,type);
                    break;
                    }
                case RECT:
                    {
                    KisRectangleMaskGenerator bCircVectr(499.5, k/100.f, i/100.f, j/100.f, 2, true);
                    KisRectangleMaskGenerator bCircScalar(bCircVectr);
                    bCircScalar.resetMaskApplicator(true); // Force usage of scalar backend

                    KisMaskSimilarityTester(bCircScalar.applicator(), bCircVectr.applicator(), bounds,type);
                    break;

                    }
                case RECT_GAUSS:
                    {
                    KisGaussRectangleMaskGenerator bCircVectr(499.5, k/100.f, i/100.f, j/100.f, 2, true);
                    KisGaussRectangleMaskGenerator bCircScalar(bCircVectr);
                    bCircScalar.resetMaskApplicator(true); // Force usage of scalar backend

                    KisMaskSimilarityTester(bCircScalar.applicator(), bCircVectr.applicator(), bounds,type);
                    break;

                    }
                case RECT_SOFT:
                    {
                    KisCubicCurve pointsCurve;
                    pointsCurve.fromString(QString("0,1;1,0"));
                    KisCurveRectangleMaskGenerator bCircVectr(499.5, k/100.f, i/100.f, j/100.f, 2, pointsCurve, true);
                    KisCurveRectangleMaskGenerator bCircScalar(bCircVectr);
                    bCircScalar.resetMaskApplicator(true); // Force usage of scalar backend

                    KisMaskSimilarityTester(bCircScalar.applicator(), bCircVectr.applicator(), bounds,type);
                    break;
                    }
                default:
                    {
                    break;
                    }
                }

                if (QTest::currentTestFailed()) {
                    QWARN(QString("Mask features: Ratio=%1, hfade=%2, vfade=%3 \n")
                          .arg(k/100.f,0,'g',2).arg(i/100.f,0,'g',2).arg(j/100.f,0,'g',2).toLatin1());
                    return false;
                }

        } } } // end for
        return true;
    }

    template <typename MaskGenerator>
    static void runMaskGenTest(MaskGenerator& generator, MaskType type) {
        QRect bounds(0,0,700,700);
        generator.setDiameter(499.5);
        MaskGenerator scalarGenerator(generator);

        scalarGenerator.resetMaskApplicator(true); // Force usage of scalar backend
        KisMaskSimilarityTester(scalarGenerator.applicator(), generator.applicator(), bounds, type);

        // KisMaskSimilarityTester::exahustiveTest(bounds,type);
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
        case RECT:
            strName = "Rect";
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
    KisCircleMaskGenerator generator(499.5, 0.2, 0.5, 0.5, 2, true);
    qDebug() << generator.id() << generator.name();
    KisMaskSimilarityTester::runMaskGenTest(generator,DEFAULT);
}

void KisMaskSimilarityTest::testGaussCircleMask()
{
    KisGaussCircleMaskGenerator generator(499.5, 0.2, 1, 1, 2, true);
    KisMaskSimilarityTester::runMaskGenTest(generator,CIRC_GAUSS);
}

void KisMaskSimilarityTest::testSoftCircleMask()
{
    KisCubicCurve pointsCurve;
    pointsCurve.fromString(QString("0,1;1,0"));
    KisCurveCircleMaskGenerator generator(499.5, 0.2, 0.5, 0.5, 2, pointsCurve,true);
    KisMaskSimilarityTester::runMaskGenTest(generator,CIRC_SOFT);
}

void KisMaskSimilarityTest::testRectMask()
{
    KisRectangleMaskGenerator generator(499.5, 0.1, 0.5, 0.5, 2, false);
    KisMaskSimilarityTester::runMaskGenTest(generator,RECT);
}

void KisMaskSimilarityTest::testGaussRectMask()
{
    KisGaussRectangleMaskGenerator generator(499.5, 0.2, 0.5, 0.2, 2, true);
    KisMaskSimilarityTester::runMaskGenTest(generator,RECT_GAUSS);
}

void KisMaskSimilarityTest::testSoftRectMask()
{
    KisCubicCurve pointsCurve;
    pointsCurve.fromString(QString("0,1;1,0"));
    KisCurveRectangleMaskGenerator generator(499.5, 0.2, 0.5, 0.2, 2, pointsCurve, true);
    KisMaskSimilarityTester::runMaskGenTest(generator,RECT_SOFT);
}

SIMPLE_TEST_MAIN(KisMaskSimilarityTest)
