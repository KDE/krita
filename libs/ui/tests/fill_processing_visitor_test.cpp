/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "fill_processing_visitor_test.h"

#include <simpletest.h>

#include "kis_undo_stores.h"
#include "kis_processing_applicator.h"

#include <testutil.h>
#include "qimage_based_test.h"
#include "stroke_testing_utils.h"
#include <resources/KoPattern.h>
#include "kis_canvas_resource_provider.h"

#include <processing/fill_processing_visitor.h>
#include <KisGlobalResourcesInterface.h>

class FillProcessingVisitorTester : public TestUtil::QImageBasedTest
{
public:
    FillProcessingVisitorTester()
        : QImageBasedTest("fill_processing")
    {
    }

    void test(const QString &testname, bool haveSelection, bool usePattern, bool selectionOnly) {
        KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
        KisImageSP image = createImage(undoStore);

        if (haveSelection) {
            addGlobalSelection(image);
        }

        image->initialRefreshGraph();

        QVERIFY(checkLayersInitial(image));

        KisNodeSP fillNode = findNode(image->root(), "paint1");

        KoCanvasResourceProvider *manager = utils::createResourceManager(image, fillNode);

        KoPatternSP newPattern(new KoPattern(TestUtil::fetchDataFileLazy("HR_SketchPaper_01.pat")));
        newPattern->load(KisGlobalResourcesInterface::instance());
        Q_ASSERT(newPattern->valid());

        QVariant v;
        v.setValue<KoPatternSP>(newPattern);
        manager->setResource(KoCanvasResource::CurrentPattern, v);

        KisResourcesSnapshotSP resources =
            new KisResourcesSnapshot(image,
                                     fillNode,
                                     manager);

        KisProcessingVisitorSP visitor =
            new FillProcessingVisitor(0,
                                      QPoint(100,100),
                                      image->globalSelection(),
                                      resources,
                                      false, // useFastMode
                                      usePattern,
                                      selectionOnly,
                                      false,
                                      10, 10, 10,
                                      true /* use the current device (unmerged) */,
                                      false);


        KisProcessingApplicator applicator(image, fillNode,
                                           KisProcessingApplicator::NONE);

        applicator.applyVisitor(visitor);
        applicator.end();
        image->waitForDone();

        QVERIFY(checkOneLayer(image, fillNode, testname, 500));

        undoStore->undo();
        image->waitForDone();

        QVERIFY(checkLayersInitial(image));
    }
};

void FillProcessingVisitorTest::testFillColorNoSelection()
{
    FillProcessingVisitorTester tester;
    tester.test("fill_color_no_selection", false, false, false);
}

void FillProcessingVisitorTest::testFillPatternNoSelection()
{
    FillProcessingVisitorTester tester;
    tester.test("fill_pattern_no_selection", false, true, false);
}

void FillProcessingVisitorTest::testFillColorHaveSelection()
{
    FillProcessingVisitorTester tester;
    tester.test("fill_color_have_selection", true, false, false);
}

void FillProcessingVisitorTest::testFillPatternHaveSelection()
{
    FillProcessingVisitorTester tester;
    tester.test("fill_pattern_have_selection", true, true, false);
}

void FillProcessingVisitorTest::testFillColorNoSelectionSelectionOnly()
{
    FillProcessingVisitorTester tester;
    tester.test("fill_color_no_selection_selection_only", false, false, true);
}

void FillProcessingVisitorTest::testFillPatternNoSelectionSelectionOnly()
{
    FillProcessingVisitorTester tester;
    tester.test("fill_pattern_no_selection_selection_only", false, true, true);
}

void FillProcessingVisitorTest::testFillColorHaveSelectionSelectionOnly()
{
    FillProcessingVisitorTester tester;
    tester.test("fill_color_have_selection_selection_only", true, false, true);
}

void FillProcessingVisitorTest::testFillPatternHaveSelectionSelectionOnly()
{
    FillProcessingVisitorTester tester;
    tester.test("fill_pattern_have_selection_selection_only", true, true, true);
}

SIMPLE_TEST_MAIN(FillProcessingVisitorTest)
