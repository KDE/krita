/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_filter_weights_buffer_test.h"

#include <qtest_kde.h>

#include "kis_filter_strategy.h"

//#define DEBUG_ENABLED
#define SANITY_CHECKS_ENABLED
#include "kis_filter_weights_buffer.h"

#include "kis_debug.h"

void checkWeightsBuffer(KisFilterStrategy *filter, qreal scale)
{
    qDebug() << "Testing:" << filter->name() << "Scale:" << scale;

    KisFilterWeightsBuffer buf(filter, scale);

    KisFixedPoint fp1;
    KisFixedPoint fp2;

    const int startIndex = 1;
    const int endIndex = 255 * qMin(scale, qreal(1.0));

    fp1.from256Frac(startIndex);
    fp2.from256Frac(endIndex);

    while (fp2 > fp1) {
        KisFilterWeightsBuffer::FilterWeights *w1 = buf.weights(fp1);
        KisFilterWeightsBuffer::FilterWeights *w2 = buf.weights(fp2);

        QCOMPARE(w1->span, w2->span);

        int span = w1->span;

        // Check for symmetry around center offset
        for (int i = 0; i < span; i++) {
            int idx2 = span - i - 1;

            int v1 = w1->weight[i];
            int v2 = w2->weight[idx2];

            if (v1 != v2) {

#ifdef DEBUG_ENABLED
                qDebug() << "*******";
                qDebug() << "Weight" << fp1 << "|" << i << ":" << v1;
                qDebug() << "Weight" << fp2 << "|" << idx2 << ":" << v2;
#endif /* DEBUG_ENABLED */

                if (!(span & 0x1) && (qAbs(v1 - v2) <= (0.5 * span))) {
#ifdef DEBUG_ENABLED
                    qDebug() << "Symmetry is wrong due to evenly-sized kernel or rounding. It's ok. Accepting.";
#endif /* DEBUG_ENABLED */
                } else {
                    QFAIL("Wrong weight symmetry");
                }
            }
        }

        fp1.inc256Frac();
        fp2.dec256Frac();
    }
}

void checkOneFilter(KisFilterStrategy *filter)
{
    checkWeightsBuffer(filter, 1.0);
    checkWeightsBuffer(filter, 2.0);
    checkWeightsBuffer(filter, 0.5);
    checkWeightsBuffer(filter, 0.25);
    checkWeightsBuffer(filter, 0.125);
    delete filter;
    checkForAsymmetricZeros = false;
}


void KisFilterWeightsBufferTest::testTriangle()
{
    checkForAsymmetricZeros = true;
    checkOneFilter(new KisBilinearFilterStrategy());
}

void KisFilterWeightsBufferTest::testHermite()
{
    checkOneFilter(new KisHermiteFilterStrategy());
}

void KisFilterWeightsBufferTest::testBicubic()
{
    checkOneFilter(new KisBicubicFilterStrategy());
}

void KisFilterWeightsBufferTest::testBox()
{
    checkOneFilter(new KisBoxFilterStrategy());
}

void KisFilterWeightsBufferTest::testBell()
{
    checkOneFilter(new KisBellFilterStrategy());
}

void KisFilterWeightsBufferTest::testBSpline()
{
    checkOneFilter(new KisBSplineFilterStrategy());
}

void KisFilterWeightsBufferTest::testLanczos3()
{
    checkOneFilter(new KisLanczos3FilterStrategy());
}

void KisFilterWeightsBufferTest::testMitchell()
{
    checkForAsymmetricZeros = true;
    checkOneFilter(new KisMitchellFilterStrategy());
}



QTEST_KDEMAIN(KisFilterWeightsBufferTest, GUI)
