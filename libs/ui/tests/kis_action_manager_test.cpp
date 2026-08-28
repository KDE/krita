
/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "kis_action_manager_test.h"
#include <kis_debug.h>

#include <QMenu>
#include "KisPortingUtils.h"
#include <KisPart.h>
#include <KisMainWindow.h>
#include <KisDocument.h>
#include <KisView.h>
#include <util.h>
#include <kis_action.h>
#include <kis_action_manager.h>
#include <KisViewManager.h>

#include "kis_node_manager.h"
#include <testui.h>

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

namespace
{

void printActionHierarchyImpl(const QList<QAction *> actions, int indentation)
{
    Q_FOREACH (QAction *action, actions) {
        qDebug().nospace().noquote() << QString(" ").repeated(indentation) << action->objectName();
        if (action->menu()) {
            printActionHierarchyImpl(action->menu()->actions(), indentation + 2);
        }
    }
}

void printActionHierarchy(const QList<QAction *> actions)
{
    printActionHierarchyImpl(actions, 0);
};

std::pair<QString, int> findActionLocationImpl(QAction *action, const QList<QAction*> &menuBarActions, const QString &prefix)
{
    auto calcPrefix = [&] (const QAction *action) {
        return !prefix.isEmpty() ? prefix + '/' + action->objectName() : action->objectName();
    };

    Q_FOREACH(QAction *node, menuBarActions) {
        if (node == action) {
            return {calcPrefix(node), menuBarActions.indexOf(node)};
        } else if (node->menu()) {
            auto result = findActionLocationImpl(action, node->menu()->actions(), calcPrefix(node));
            if (result.second >= 0) return result;
        }
    }

    return {{}, -1};
}

// returns a pair of "full path" and an "index in the submenu it belongs to"
std::pair<QString, int> findActionLocation(QAction *action, const QList<QAction*> &menuBarActions)
{
    return findActionLocationImpl(action, menuBarActions, "");
}

} // namespace

void KisActionManagerTest::testDynamicActionUpdate_data()
{
    QTest::addColumn<QString>("newActionPath");
    QTest::addColumn<QString>("newActionName");
    QTest::addColumn<QString>("expectedNewPath");
    QTest::addColumn<int>("expectedFinalIndex");

    QTest::addRow("addScriptBefore") << "tools/scripts" << "scriptA" << "tools/scripts/scriptA" << 0;
    QTest::addRow("addScriptAfter") << "tools/scripts" << "scriptD" << "tools/scripts/scriptD" << 2;
    QTest::addRow("addScriptAfterWeirdCase") << "ToOlS/sCrIpTs" << "scriptD" << "tools/scripts/scriptD" << 2;

    QTest::addRow("addScriptAfterTrailingSlash") << "tools/scripts/" << "scriptD" << "tools/scripts/scriptD" << 2;
    QTest::addRow("addScriptAfterDoubleSlash") << "tools//scripts" << "scriptD" << "tools/scripts/scriptD" << 2;

    QTest::addRow("addScriptOverwriteFirst") << "tools/scripts" << "scriptB" << "tools/scripts/scriptB" << 0;
    QTest::addRow("addScriptOverwriteLast") << "tools/scripts" << "scriptC" << "tools/scripts/scriptC" << 1;

    QTest::addRow("addNonScriptToolBefore") << "tools" << "a-tool" << "tools/a-tool" << 0;
    QTest::addRow("addNonScriptToolAfter") << "tools" << "z-tool" << "tools/z-tool" << 2;
    QTest::addRow("addToEmptyMenu") << "file" << "new" << "file/new" << 0;
    QTest::addRow("addToActionWithoutMenu") << "exit" << "something" << "tools/fallback/something" << 0;

    QTest::addRow("addToInexistentToplevelMenu") << "help" << "something" << "tools/fallback/something" << 0;
    QTest::addRow("addToInexistentSubmenu") << "tools/non-existent" << "something" << "tools/fallback/something" << 0;
}

void KisActionManagerTest::testDynamicActionUpdate()
{
    QFETCH(QString, newActionPath);
    QFETCH(QString, newActionName);
    QFETCH(QString, expectedNewPath);
    QFETCH(int, expectedFinalIndex);

    auto createAction = [] (const QString &name) {
        QAction *action = new QAction(name);
        action->setObjectName(name);
        return action;
    };

    QList<QAction*> menuBarActions;

    QAction *fileAction = createAction(QLatin1String("file"));
    QAction *exitAction = createAction(QLatin1String("exit"));
    QAction *toolsAction = createAction(QLatin1String("tools"));

    menuBarActions.append(fileAction);
    menuBarActions.append(exitAction);
    menuBarActions.append(toolsAction);

    QAction *scriptsAction = createAction(QLatin1String("scripts"));
    QAction *existingScriptBAction = createAction(QLatin1String("scriptB"));
    QAction *existingScriptCAction = createAction(QLatin1String("scriptC"));

    QAction *fallbackAction = createAction(QLatin1String("fallback"));

    // just an empty "File" menu
    fileAction->setMenu(new QMenu());

    toolsAction->setMenu(new QMenu());
    toolsAction->menu()->addAction(fallbackAction);
    toolsAction->menu()->addAction(scriptsAction);

    scriptsAction->setMenu(new QMenu());
    scriptsAction->menu()->addAction(existingScriptBAction);
    scriptsAction->menu()->addAction(existingScriptCAction);

    // just an empty fallback menu
    fallbackAction->setMenu(new QMenu());

    QAction *newAction = createAction(newActionName);
    newAction->setProperty("menulocation", newActionPath);
    newAction->setProperty("defaultmenulocation", "tools/fallback");

    KisActionManager::synchronizeDynamicActions({newAction}, menuBarActions);

#if 0
    qDebug() << "Result menu bar actions:";
    printActionHierarchy(menuBarActions);
#else
    Q_UNUSED(printActionHierarchy)
#endif

    auto [path, finalIndex] = findActionLocation(newAction, menuBarActions);

    QCOMPARE(path, expectedNewPath);
    QCOMPARE(finalIndex, expectedFinalIndex);
}


KISTEST_MAIN(KisActionManagerTest)
