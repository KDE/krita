/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_processings_test.h"

#include <QTest>

#include "kis_undo_stores.h"
#include "kis_processing_applicator.h"
#include "processing/kis_crop_processing_visitor.h"

#include "testutil.h"

#define USE_DOCUMENT 0
#include "qimage_based_test.h"

#include "kis_filter_strategy.h"
#include "kis_transform_worker.h"
#include "processing/kis_transform_processing_visitor.h"


class BaseProcessingTest : public TestUtil::QImageBasedTest
{
public:
    BaseProcessingTest()
        : QImageBasedTest("processings")
    {
    }

    void test(const QString &testname, KisProcessingVisitorSP visitor) {
        KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
        KisImageSP image = createImage(undoStore);
        image->initialRefreshGraph();

        QVERIFY(checkLayersInitial(image));

        KisProcessingApplicator applicator(image, image->root(),
                                           KisProcessingApplicator::RECURSIVE);

        applicator.applyVisitor(visitor);
        applicator.end();
        image->waitForDone();

        /**
         * NOTE: after a change in KisLayer::changeRect(), which now
         * crops change rect for layers with COMPOSITE_COPY
         * composition, the clone layer will have some ghost pixels
         * outside main projection rect. That is ok, because these
         * pixels will never be painted due to a Filter Layer above,
         * which crops the change rect.
         */
        QVERIFY(checkLayers(image, testname));

        undoStore->undo();
        image->waitForDone();

        if (!checkLayersInitial(image)) {
            warnKrita << "NOTE: undo is not completely identical to the original image. Falling back to projection comparison";
            QVERIFY(checkLayersInitialRootOnly(image));
        }
    }
};

void KisProcessingsTest::testCropVisitor()
{
    KisProcessingVisitorSP visitor =
        new KisCropProcessingVisitor(QRect(45,45,410,410), true, true);

    BaseProcessingTest tester;
    tester.test("crop", visitor);
}

void KisProcessingsTest::testTransformVisitorScale()
{
    BaseProcessingTest tester;

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisProcessingVisitorSP visitor =
        new KisTransformProcessingVisitor(0.5, 0.5,
                                          0,0,QPointF(),
                                          0,
                                          0,0,
                                          filter);

    tester.test("transform_scale", visitor);
}

void KisProcessingsTest::testTransformVisitorScaleRotate()
{
    BaseProcessingTest tester;

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisProcessingVisitorSP visitor =
        new KisTransformProcessingVisitor(0.5, 0.5,
                                          0,0,QPointF(),
                                          M_PI,
                                          320,221,
                                          filter);

    tester.test("transform_scale_rotate", visitor);
}

QTEST_MAIN(KisProcessingsTest)
