
/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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


#include "kis_action_manager_test.h"
#include <kis_debug.h>

#include <KisPart.h>
#include <KisMainWindow.h>
#include <KisDocument.h>
#include <KisView.h>
#include <util.h>
#include <kis_action.h>
#include <kis_action_manager.h>
#include <KisViewManager.h>

#include "kis_node_manager.h"
#include <kistest.h>

void KisActionManagerTest::testUpdateGUI()
{
    KisDocument* doc = createEmptyDocument();
    KisMainWindow* mainWindow = KisPart::instance()->createMainWindow();
    QPointer<KisView> view = new KisView(doc, mainWindow->viewManager(), mainWindow);
    KisViewManager *viewManager = new KisViewManager(mainWindow, mainWindow->actionCollection());
    KisPart::instance()->addView(view);
    mainWindow->showView(view);

    view->setViewManager(viewManager);
    viewManager->setCurrentView(view);

    KisAction* action = new KisAction("dummy", this);
    action->setActivationFlags(KisAction::ACTIVE_DEVICE);
    view->viewManager()->actionManager()->addAction("dummy", action);

    KisAction* action2 = new KisAction("dummy", this);
    action2->setActivationFlags(KisAction::ACTIVE_SHAPE_LAYER);
    view->viewManager()->actionManager()->addAction("dummy", action2);
    
    view->viewManager()->actionManager()->updateGUI();
    QVERIFY(!action->isEnabled());
    QVERIFY(!action2->isEnabled());

    KisPaintLayerSP paintLayer1 = new KisPaintLayer(doc->image(), "paintlayer1", OPACITY_OPAQUE_U8);
    doc->image()->addNode(paintLayer1);

    viewManager->nodeManager()->slotUiActivatedNode(paintLayer1);

    view->viewManager()->actionManager()->updateGUI();
    QVERIFY(action->isEnabled());
    QVERIFY(!action2->isEnabled());
}

void KisActionManagerTest::testCondition()
{
    KisDocument* doc = createEmptyDocument();
    KisMainWindow* mainWindow = KisPart::instance()->createMainWindow();
    QPointer<KisView> view = new KisView(doc, mainWindow->viewManager(), mainWindow);
    KisViewManager *viewManager = new KisViewManager(mainWindow, mainWindow->actionCollection());
    KisPart::instance()->addView(view);
    mainWindow->showView(view);

    view->setViewManager(viewManager);
    viewManager->setCurrentView(view);

    KisAction* action = new KisAction("dummy", this);
    action->setActivationFlags(KisAction::ACTIVE_DEVICE);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    view->viewManager()->actionManager()->addAction("dummy", action);

    KisPaintLayerSP paintLayer1 = new KisPaintLayer(doc->image(), "paintlayer1", OPACITY_OPAQUE_U8);
    doc->image()->addNode(paintLayer1);

    viewManager->nodeManager()->slotUiActivatedNode(paintLayer1);

    view->viewManager()->actionManager()->updateGUI();
    QVERIFY(action->isEnabled());

    // visible
//     paintLayer1->setVisible(false);
//     view->viewManager()->actionManager()->updateGUI();
//     QVERIFY(!action->isEnabled());

    paintLayer1->setVisible(true);
    view->viewManager()->actionManager()->updateGUI();
    QVERIFY(action->isEnabled());

    // locked
    paintLayer1->setUserLocked(true);
    view->viewManager()->actionManager()->updateGUI();
    QVERIFY(!action->isEnabled());

    paintLayer1->setUserLocked(false);
    view->viewManager()->actionManager()->updateGUI();
    QVERIFY(action->isEnabled());
}

void KisActionManagerTest::testTakeAction()
{
    KisDocument* doc = createEmptyDocument();
    KisMainWindow* mainWindow = KisPart::instance()->createMainWindow();
    QPointer<KisView> view = new KisView(doc, mainWindow->viewManager(), mainWindow);
    KisViewManager *viewManager = new KisViewManager(mainWindow, mainWindow->actionCollection());
    KisPart::instance()->addView(view);
    mainWindow->showView(view);

    view->setViewManager(viewManager);
    viewManager->setCurrentView(view);

    KisAction* action = new KisAction("dummy", this);
    view->viewManager()->actionManager()->addAction("dummy", action);
    QVERIFY(view->viewManager()->actionManager()->actionByName("dummy") != 0);

    view->viewManager()->actionManager()->takeAction(action);
    QVERIFY(view->viewManager()->actionManager()->actionByName("dummy") == 0);
}


KISTEST_MAIN(KisActionManagerTest)
