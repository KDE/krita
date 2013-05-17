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

#ifndef __UI_MANAGER_TEST_H
#define __UI_MANAGER_TEST_H

#include "testutil.h"
#include "qimage_based_test.h"

#include "kis_pattern.h"
#include "kis_resource_server_provider.h"
#include "kis_canvas_resource_provider.h"
#include "kis_filter_strategy.h"
#include "kis_selection_manager.h"
#include "kis_node_manager.h"
#include "kis_view2.h"
#include <KoDocument.h>
#include <KoPart.h>
#include <kis_action_manager.h>
#include "KoMainWindow.h"


namespace TestUtil
{

class UiManagerTest : public TestUtil::QImageBasedTest
{
public:
    UiManagerTest(bool useSelection, bool useShapeLayer, const QString &testName)
        : QImageBasedTest(testName) // "selection_manager_test"
    {
        undoStore = new KisSurrogateUndoStore();
        image = createImage(undoStore);

        doc = new KisDoc2();
        doc->setCurrentImage(image);

        if(useSelection) addGlobalSelection(image);
        if(useShapeLayer) addShapeLayer(doc, image);
        image->initialRefreshGraph();

        QVERIFY(checkLayersInitial());

        shell = new KoMainWindow(doc->documentPart()->componentData());
        view = new KisView2(doc->documentPart(), doc, shell);

        KisPattern *newPattern = new KisPattern(fetchDataFileLazy("HR_SketchPaper_01.pat"));
        newPattern->load();
        Q_ASSERT(newPattern->valid());
        view->resourceProvider()->slotPatternActivated(newPattern);

        KoColor fgColor(Qt::black, image->colorSpace());
        KoColor bgColor(Qt::white, image->colorSpace());
        view->resourceProvider()->setBGColor(bgColor);
        view->resourceProvider()->setFGColor(fgColor);

        KisNodeSP paint1 = findNode(image->root(), "paint1");
        Q_ASSERT(paint1);

        view->nodeManager()->slotNonUiActivatedNode(paint1);
        selectionManager = view->selectionManager();
        actionManager = view->actionManager();
    }

    ~UiManagerTest() {
        /**
         * Here is a weird way of precessing pending events.
         * This is needed for the dummies facade could process
         * all the queued events telling it some nodes were
         * added/deleted
         */
        QApplication::processEvents();
        QTest::qSleep(500);
        QApplication::processEvents();

        delete shell;
        delete doc;

        /**
         * The event queue may have up to 200k events
         * by the time all the tests are finished. Removing
         * all of them may last forever, so clear them after
         * every single test is finished
         */
        QApplication::removePostedEvents(0);
    }

    void checkUndo() {
        undoStore->undo();
        image->waitForDone();
        QVERIFY(checkLayersInitial());
    }

    void checkDoubleUndo() {
        undoStore->undo();
        undoStore->undo();
        image->waitForDone();
        QVERIFY(checkLayersInitial());
    }

    void startConcurrentTask() {
        KisFilterStrategy * filter = new KisBoxFilterStrategy();
        QSize initialSize = image->size();

        image->scaleImage(2 * initialSize, image->xRes(), image->yRes(), filter);
        image->waitForDone();

        image->scaleImage(initialSize, image->xRes(), image->yRes(), filter);
    }

    using QImageBasedTest::checkLayers;

    bool checkLayers(const QString &name) {
        return checkLayers(image, name);
    }

    using QImageBasedTest::checkLayersInitial;

    bool checkLayersInitial() {
        return checkLayersInitial(image);
    }

    bool checkLayersFuzzy(const QString &name) {
        return checkLayers(image, name, 1);
    }

    bool checkSelectionOnly(const QString &name) {
        KisNodeSP mask = findNode(image->root(), "selection");
        return checkOneLayer(image, mask, name);
    }

    bool checkNoSelection() {
        KisNodeSP mask = findNode(image->root(), "selection");
        return !mask && !image->globalSelection();
    }

    KisImageSP image;
    KisSelectionManager *selectionManager;
    KisActionManager *actionManager;
    KisSurrogateUndoStore *undoStore;

protected:
    KisView2 *view;
    KisDoc2 *doc;
    KoMainWindow *shell;
};

}

#endif /* __UI_MANAGER_TEST_H */
