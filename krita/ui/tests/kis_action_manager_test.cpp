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
#include <kdebug.h>

#include <KoPart.h>
#include <KoMainWindow.h>
#include <kis_doc2.h>
#include <kis_part2.h>
#include <kis_view2.h>
#include <util.h>
#include <kis_action.h>
#include <kis_action_manager.h>

void KisActionManagerTest::testUpdateGUI()
{
    KisDoc2* doc = createEmptyDocument();
    KoMainWindow* mainWindow = doc->documentPart()->createMainWindow();
    KisView2* view = new KisView2(static_cast<KisPart2*>(doc->documentPart()), static_cast<KisDoc2*>(doc), mainWindow);
    doc->documentPart()->addView(view, doc);

    KisAction* action = new KisAction("dummy", this);
    action->setActivationFlags(KisAction::ACTIVE_DEVICE);
    view->actionManager()->addAction("dummy", action, view->actionCollection());

    KisAction* action2 = new KisAction("dummy", this);
    action2->setActivationFlags(KisAction::ACTIVE_SHAPE_LAYER);
    view->actionManager()->addAction("dummy", action2, view->actionCollection());
    
    view->actionManager()->updateGUI();
    QVERIFY(!action->isEnabled());
    QVERIFY(!action2->isEnabled());

    KisPaintLayerSP paintLayer1 = new KisPaintLayer(doc->image(), "paintlayer1", OPACITY_OPAQUE_U8);
    doc->image()->addNode(paintLayer1);

    view->actionManager()->updateGUI();
    QVERIFY(action->isEnabled());
    QVERIFY(!action2->isEnabled());
}

void KisActionManagerTest::testCondition()
{
    KisDoc2* doc = createEmptyDocument();
    KoMainWindow* mainWindow =  doc->documentPart()->createMainWindow();
    KisView2* view = new KisView2(static_cast<KisPart2*>(doc->documentPart()), static_cast<KisDoc2*>(doc), mainWindow);
    doc->documentPart()->addView(view, doc);

    KisAction* action = new KisAction("dummy", this);
    action->setActivationFlags(KisAction::ACTIVE_DEVICE);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    view->actionManager()->addAction("dummy", action, view->actionCollection());

    KisPaintLayerSP paintLayer1 = new KisPaintLayer(doc->image(), "paintlayer1", OPACITY_OPAQUE_U8);
    doc->image()->addNode(paintLayer1);

    view->actionManager()->updateGUI();
    QVERIFY(action->isEnabled());

    // visible
    paintLayer1->setVisible(false);
    view->actionManager()->updateGUI();
    QVERIFY(!action->isEnabled());

    paintLayer1->setVisible(true);
    view->actionManager()->updateGUI();
    QVERIFY(action->isEnabled());

    // locked
    paintLayer1->setUserLocked(true);
    view->actionManager()->updateGUI();
    QVERIFY(!action->isEnabled());

    paintLayer1->setUserLocked(false);
    view->actionManager()->updateGUI();
    QVERIFY(action->isEnabled());
}

void KisActionManagerTest::testTakeAction()
{
    KisDoc2* doc = createEmptyDocument();
    KoMainWindow* mainWindow =  doc->documentPart()->createMainWindow();
    KisView2* view = new KisView2(static_cast<KisPart2*>(doc->documentPart()), static_cast<KisDoc2*>(doc), mainWindow);
    doc->documentPart()->addView(view, doc);

    KisAction* action = new KisAction("dummy", this);
    view->actionManager()->addAction("dummy", action, view->actionCollection());
    QVERIFY(view->actionManager()->actionByName("dummy") != 0);

    view->actionManager()->takeAction(action);
    QVERIFY(view->actionManager()->actionByName("dummy") == 0);
}


QTEST_KDEMAIN(KisActionManagerTest, GUI)
