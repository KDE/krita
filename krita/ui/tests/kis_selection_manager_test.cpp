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

#include "kis_selection_manager_test.h"

#include <qtest_kde.h>

#include "ui_manager_test.h"

class SelectionManagerTester : public TestUtil::UiManagerTest
{
public:
    SelectionManagerTester(bool useSelection)
        : UiManagerTest(useSelection, false,  "selection_manager_test")
    {
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
    QVERIFY(t.checkLayers("fill_pattern_with_selection"));
}

void KisSelectionManagerTest::testResizeToSelection()
{
    SelectionManagerTester t(true);

    t.selectionManager->imageResizeToSelection();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("resize_to_selection"));

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

    t.selectionManager->copy();
    t.selectionManager->paste();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("copy_paste"));

    t.checkUndo();
    t.startConcurrentTask();

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

    t.selectionManager->cut();
    t.selectionManager->paste();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("cut_paste"));
}

void KisSelectionManagerTest::testInvertSelection()
{
    SelectionManagerTester t(true);

    t.selectionManager->invert();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("invert_selection"));

    t.checkUndo();
    t.startConcurrentTask();

    t.selectionManager->invert();
    t.image->waitForDone();
    QVERIFY(t.checkLayers("invert_selection"));
}

void KisSelectionManagerTest::testFeatherSelection()
{
    SelectionManagerTester t(true);

    t.selectionManager->feather(10);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("feather_selection"));

    t.checkUndo();
    t.startConcurrentTask();

    t.selectionManager->feather(10);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("feather_selection"));
}

void KisSelectionManagerTest::testGrowSelectionSimplified()
{
//     SelectionManagerTester t(true);
// 
//     t.selectionManager->grow(10,5);
//     t.image->waitForDone();
//     QVERIFY(t.checkSelectionOnly("grow_selection"));
}

void KisSelectionManagerTest::testShrinkSelectionUnlockedSimplified()
{
    SelectionManagerTester t(true);

    t.selectionManager->shrink(10, 5, false);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("shrink_selection_unlocked"));
}

void KisSelectionManagerTest::testShrinkSelectionLockedSimplified()
{
    SelectionManagerTester t(true);

    t.selectionManager->shrink(10, 5, true);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("shrink_selection_locked"));
}

void KisSelectionManagerTest::testSmoothSelectionSimplified()
{
    SelectionManagerTester t(true);

    t.selectionManager->smooth();
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("smooth_selection"));
}

void KisSelectionManagerTest::testErodeSelectionSimplified()
{
    SelectionManagerTester t(true);

    t.selectionManager->erode();
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("erode_selection"));
}

void KisSelectionManagerTest::testDilateSelectionSimplified()
{
    SelectionManagerTester t(true);

    t.selectionManager->dilate();
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("dilate_selection"));
}

void KisSelectionManagerTest::testBorderSelectionSimplified()
{
    SelectionManagerTester t(true);

    t.selectionManager->border(10,5);
    t.image->waitForDone();
    QVERIFY(t.checkSelectionOnly("border_selection"));
}

QTEST_KDEMAIN(KisSelectionManagerTest, GUI)
