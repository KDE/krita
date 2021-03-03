/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_manager_test.h"
#include <operations/kis_operation_configuration.h>

#include <simpletest.h>

#include <sdk/tests/testutil.h>
#include <sdk/tests/testui.h>
#include "ui_manager_test.h"

class SelectionManagerTester : public TestUtil::UiManagerTest
{
public:
    SelectionManagerTester(bool useSelection)
        : UiManagerTest(useSelection, false,  "selection_manager_test")
    {
        Q_ASSERT(selectionManager);
    }

};


void KisSelectionManagerTest::testFillForegroundWithoutSelection()
{
    SelectionManagerTester t(false);

    t.selectionManager->fillForegroundColor();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("fill_foreground_without_selection"));

    t.checkUndo();
    t.startConcurrentTask();

    t.selectionManager->fillForegroundColor();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("fill_foreground_without_selection"));
}

void KisSelectionManagerTest::testFillForegroundWithSelection()
{
    SelectionManagerTester t(true);

    t.selectionManager->fillForegroundColor();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("fill_foreground_with_selection"));

    t.checkUndo();
    t.startConcurrentTask();

    t.selectionManager->fillForegroundColor();
    t.image->waitForDone();

    QEXPECT_FAIL("", "Fix some race condition on clone layers!", Continue);
    QVERIFY(t.checkLayers("fill_foreground_with_selection"));
}

void KisSelectionManagerTest::testFillBackgroundWithSelection()
{
    SelectionManagerTester t(true);

    t.selectionManager->fillBackgroundColor();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("fill_background_with_selection"));

    t.checkUndo();
    t.startConcurrentTask();

    t.selectionManager->fillBackgroundColor();
    t.image->waitForDone();
    QEXPECT_FAIL("", "Fix some race condition on clone layers!", Continue);
    QVERIFY(t.checkLayers("fill_background_with_selection"));
}

void KisSelectionManagerTest::testFillPatternWithSelection()
{
    SelectionManagerTester t(true);

    t.selectionManager->fillPattern();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("fill_pattern_with_selection"));

    t.checkUndo();
    t.startConcurrentTask();

    t.selectionManager->fillPattern();
    t.image->waitForDone();
    QEXPECT_FAIL("", "Fix some race condition on clone layers!", Continue);
    QVERIFY(t.checkLayers("fill_pattern_with_selection"));
}

void KisSelectionManagerTest::testResizeToSelection()
{
    SelectionManagerTester t(true);

    t.selectionManager->imageResizeToSelection();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("resize_to_selection"));

    QEXPECT_FAIL("", "Fix some race condition on clone layers!", Continue);
    t.checkUndo();
    t.startConcurrentTask();

    t.selectionManager->imageResizeToSelection();
    t.image->waitForDone();

    QEXPECT_FAIL("", "The user may run Resize to Selection concurrently. It will cause wrong image/selection size fetched for the crop. There is some barrier needed. At least it doesn't crash.", Continue);
    QVERIFY(t.checkLayers("resize_to_selection"));
}

void KisSelectionManagerTest::testSelectAll()
{
    SelectionManagerTester t(true);

    t.selectionManager->selectAll();
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("select_all"));

    t.checkUndo();
    t.startConcurrentTask();

    t.selectionManager->selectAll();
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("select_all"));
}

void KisSelectionManagerTest::testDeselectReselect()
{
    SelectionManagerTester t(true);

    t.selectionManager->deselect();
    t.image->waitForDone();
    QVERIFY(t.checkNoSelection());

    t.checkUndo();
    t.startConcurrentTask();

    t.selectionManager->deselect();
    t.image->waitForDone();
    QVERIFY(t.checkNoSelection());

    t.selectionManager->reselect();
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("initial"));

    t.undoStore->undo();
    t.image->waitForDone();
    QVERIFY(t.checkNoSelection());

    t.startConcurrentTask();

    t.selectionManager->reselect();
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("initial"));
}

void KisSelectionManagerTest::testCopyPaste()
{
    SelectionManagerTester t(true);

    t.selectionManager->copy();
    t.selectionManager->paste();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("copy_paste"));

    t.checkUndo();
    t.startConcurrentTask();

    QEXPECT_FAIL("", "Fix some race condition on clone layers!", Continue);
    t.selectionManager->copy();
    t.selectionManager->paste();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("copy_paste"));

    QEXPECT_FAIL("", "Fix some race condition on clone layers!", Continue);
    t.checkUndo();
    t.startConcurrentTask();


    QEXPECT_FAIL("", "Fix some race condition on clone layers!", Continue);
    t.selectionManager->paste();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("copy_paste"));
}

void KisSelectionManagerTest::testCopyPasteMerged()
{
    SelectionManagerTester t(true);

    t.selectionManager->copyMerged();
    t.selectionManager->paste();
    t.image->waitForDone();
    QVERIFY(t.checkLayersFuzzy("copy_paste_merged"));

    t.checkUndo();
    t.startConcurrentTask();

    QEXPECT_FAIL("", "Fix some race condition on clone layers!", Continue);
    t.selectionManager->copyMerged();
    t.selectionManager->paste();
    t.image->waitForDone();
    QVERIFY(t.checkLayersFuzzy("copy_paste_merged"));
}

void KisSelectionManagerTest::testCutPaste()
{
    SelectionManagerTester t(true);

    t.selectionManager->cut();
    t.selectionManager->paste();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("cut_paste"));

    t.checkDoubleUndo();
    t.startConcurrentTask();

    QEXPECT_FAIL("", "Fix some race condition on clone layers!", Continue);
    t.selectionManager->cut();
    t.selectionManager->paste();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("cut_paste"));
}

void KisSelectionManagerTest::testInvertSelection()
{
    SelectionManagerTester t(true);

    KisOperationConfigurationSP config = new KisOperationConfiguration("invertselection");
    t.actionManager->runOperationFromConfiguration(config);
    t.image->waitForDone();
    QVERIFY(t.checkLayers("invert_selection"));

    t.checkUndo();
    t.startConcurrentTask();

    QEXPECT_FAIL("", "Fix some race condition on clone layers!", Continue);
    config = new KisOperationConfiguration("invertselection");
    t.actionManager->runOperationFromConfiguration(config);
    t.image->waitForDone();
    QVERIFY(t.checkLayers("invert_selection"));
}

void KisSelectionManagerTest::testFeatherSelection()
{
    SelectionManagerTester t(true);

    KisOperationConfigurationSP config = new KisOperationConfiguration("featherselection");
    config->setProperty("radius", 10);
    t.actionManager->runOperationFromConfiguration(config);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("feather_selection"));

    t.checkUndo();
    t.startConcurrentTask();

    config = new KisOperationConfiguration("featherselection");
    config->setProperty("radius", 10);
    t.actionManager->runOperationFromConfiguration(config);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("feather_selection"));
}

void KisSelectionManagerTest::testGrowSelectionSimplified()
{
    SelectionManagerTester t(true);

    KisOperationConfigurationSP config = new KisOperationConfiguration("growselection");
    config->setProperty("x-radius", 10);
    config->setProperty("y-radius", 5);
    t.actionManager->runOperationFromConfiguration(config);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("grow_selection"));
}

void KisSelectionManagerTest::testShrinkSelectionUnlockedSimplified()
{
    SelectionManagerTester t(true);

    KisOperationConfigurationSP config = new KisOperationConfiguration("shrinkselection");
    config->setProperty("x-radius", 10);
    config->setProperty("y-radius", 5);
    config->setProperty("edgeLock", false);
    t.actionManager->runOperationFromConfiguration(config);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("shrink_selection_unlocked"));
}

void KisSelectionManagerTest::testShrinkSelectionLockedSimplified()
{
    SelectionManagerTester t(true);

    KisOperationConfigurationSP config = new KisOperationConfiguration("shrinkselection");
    config->setProperty("x-radius", 10);
    config->setProperty("y-radius", 5);
    config->setProperty("edgeLock", true);
    t.actionManager->runOperationFromConfiguration(config);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("shrink_selection_locked"));
}

void KisSelectionManagerTest::testSmoothSelectionSimplified()
{
    SelectionManagerTester t(true);

    KisOperationConfigurationSP config = new KisOperationConfiguration("smoothselection");
    t.actionManager->runOperationFromConfiguration(config);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("smooth_selection"));
}

void KisSelectionManagerTest::testErodeSelectionSimplified()
{
//     SelectionManagerTester t(true);
// 
//     t.selectionManager->erode();
//     t.image->waitForDone();
//     QVERIFY(t.checkSelectionOnly("erode_selection"));
}

void KisSelectionManagerTest::testDilateSelectionSimplified()
{
//     SelectionManagerTester t(true);
// 
//     t.selectionManager->dilate();
//     t.image->waitForDone();
//     QVERIFY(t.checkSelectionOnly("dilate_selection"));
}

void KisSelectionManagerTest::testBorderSelectionSimplified()
{
    SelectionManagerTester t(true);

    KisOperationConfigurationSP config = new KisOperationConfiguration("borderselection");
    config->setProperty("x-radius", 10);
    config->setProperty("y-radius", 5);
    t.actionManager->runOperationFromConfiguration(config);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("border_selection"));
}

#include <floodfill/kis_scanline_fill.h>

void KisSelectionManagerTest::testScanline16bit()
{
    const int THRESHOLD = 20;

    QString fileName = TestUtil::fetchDataFileLazy("flood_fill_16bit.kra");
    QVERIFY(QFile::exists(fileName));

    KisDocument *doc = KisPart::instance()->createDocument();
    doc->loadNativeFormat(fileName);

    KisPaintDeviceSP dev = doc->image()->root()->firstChild()->paintDevice();
    QVERIFY(dev);

    dbgKrita << ppVar(dev->colorSpace());

    QRect imageRect = doc->image()->bounds();

    dbgKrita << ppVar(imageRect);

    QPoint startPoint = imageRect.center();

    dbgKrita << ppVar(startPoint);

    KisPixelSelectionSP pixelSelection = new KisPixelSelection();

    {
        KisScanlineFill gc(dev, startPoint, imageRect);
        gc.setThreshold(THRESHOLD);
        gc.fillSelection(pixelSelection);

        QImage resultImage =
            pixelSelection->convertToQImage(0,
                                            imageRect);

        QVERIFY(TestUtil::checkQImage(resultImage,
                                      "selection_manager_test",
                                      "scanline",
                                      "16bit_thres_20"));
    }

    const KoColorSpace *rgb8CS = KoColorSpaceRegistry::instance()->rgb8();
    pixelSelection->clear();
    dev->convertTo(rgb8CS);

    {
        KisScanlineFill gc(dev, startPoint, imageRect);
        gc.setThreshold(THRESHOLD);
        gc.fillSelection(pixelSelection);

        QImage resultImage =
            pixelSelection->convertToQImage(0,
                                            imageRect);

        QVERIFY(TestUtil::checkQImage(resultImage,
                                      "selection_manager_test",
                                      "scanline",
                                      "8bit_thres_20"));
    }

}

KISTEST_MAIN(KisSelectionManagerTest)
