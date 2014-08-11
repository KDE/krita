/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_sync_lod_cache_processing_visitor_test.h"

#include <qtest_kde.h>

#include "kis_undo_stores.h"
#include "kis_processing_applicator.h"
#include "testutil.h"
#include "qimage_based_test.h"

#include "processing/kis_sync_lod_cache_processing_visitor.h"


class LodProcessingTest : public TestUtil::QImageBasedTest
{
public:
    LodProcessingTest()
        : QImageBasedTest("lod_processing")
    {
    }

    void test(const QString &testname, KisProcessingVisitorSP visitor) {
        KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
        KisImageSP image = createImage(undoStore);
        image->initialRefreshGraph();

        QVERIFY(checkLayersInitial(image));

        image->setDesiredLevelOfDetail(1);
        image->testingSetLevelOfDetailsEnabled(true);

        KisProcessingApplicator applicator(image, image->root(),
                                           KisProcessingApplicator::RECURSIVE/* |
                                           KisProcessingApplicator::SUPPORTS_LOD |
                                           KisProcessingApplicator::DISABLE_AUTOMATIC_UPDATES*/);

        applicator.applyVisitor(visitor);
        applicator.end();
        image->waitForDone();

        QVERIFY(checkLayers(image, testname));

        image->testingSetLevelOfDetailsEnabled(false);

        QVERIFY(checkLayersInitial(image));
    }
};

void KisSyncLodCacheProcessingVisitorTest::test()
{
    KisProcessingVisitorSP visitor =
        new KisSyncLodCacheProcessingVisitor(1);

    LodProcessingTest tester;
    tester.test("lod1", visitor);
}

QTEST_KDEMAIN(KisSyncLodCacheProcessingVisitorTest, GUI)
