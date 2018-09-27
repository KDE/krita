/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006-2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
// flake
#include "KoToolManager.h"
#include "KoToolManager_p.h"
#include "KoToolRegistry.h"
#include "KoToolProxy.h"
#include "KoToolProxy_p.h"
#include "KoSelection.h"
#include "KoCanvasController.h"
#include "KoCanvasControllerWidget.h"
#include "KoShape.h"
#include "KoShapeLayer.h"
#include "KoShapeRegistry.h"
#include "KoShapeManager.h"
#include "KoSelectedShapesProxy.h"
#include "KoCanvasBase.h"
#include "KoInputDeviceHandlerRegistry.h"
#include "KoInputDeviceHandlerEvent.h"
#include "KoPointerEvent.h"
#include "tools/KoCreateShapesTool.h"
#include "tools/KoZoomTool.h"
#include "kis_action_registry.h"
#include "KoToolFactoryBase.h"
#include "kis_assert.h"

#include <krita_container_utils.h>

// Qt + kde
#include <QWidget>
#include <QEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTabletEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QStringList>
#include <QApplication>
#include <kactioncollection.h>
#include <kactioncategory.h>
#include <FlakeDebug.h>

#include <QAction>
#include <klocalizedstring.h>
#include <QKeySequence>
#include <QStack>
#include <QLabel>
#include <QGlobalStatic>

Q_GLOBAL_STATIC(KoToolManager, s_instance)


class CanvasData
{
public:
    CanvasData(KoCanvasController *cc, const KoInputDevice &id)
            : activeTool(0),
            canvas(cc),
            inputDevice(id),
            dummyToolWidget(0),
            dummyToolLabel(0)
    {
    }

    ~CanvasData()
    {
        // the dummy tool widget does not necessarily have a parent and we create it, so we delete it.
        delete dummyToolWidget;
    }

    void activateToolActions()
    {
        disabledDisabledActions.clear();
        disabledActions.clear();
        disabledCanvasShortcuts.clear();
        // we do several things here
        // 1. enable the actions of the active tool
        // 2. disable conflicting actions
        // 3. replace conflicting actions in the action collection
        KActionCollection *canvasActionCollection = canvas->actionCollection();

        QHash<QString, QAction *> toolActions = activeTool->actions();
        QHash<QString, QAction *>::const_iterator it(toolActions.constBegin());

        for (; it != toolActions.constEnd(); ++it) {
            if (canvasActionCollection) {

                QString toolActionID = it.key();
                QAction *toolAction = it.value();

                QAction * action = qobject_cast<QAction*>(canvasActionCollection->action(it.key()));
                if (action) {
                    canvasActionCollection->takeAction(action);
                    if (action != it.value()) {
                        if (action->isEnabled()) {
                            action->setEnabled(false);
                            disabledActions.append(action);
                        } else  {
                            disabledDisabledActions.append(action);
                        }
                    }
                }
                Q_FOREACH (QAction *a, canvasActionCollection->actions()) {
                    QAction *canvasAction = dynamic_cast<QAction*>(a);
                    if (canvasAction && canvasAction->shortcut().toString() != "" && canvasAction->shortcut() == toolAction->shortcut()) {
                        warnFlake << activeToolId << ": action" << toolActionID << "conflicts with canvas action" << canvasAction->objectName() << "shortcut:" << canvasAction->shortcut().toString();
                        disabledCanvasShortcuts[canvasAction] = canvasAction->shortcut().toString();
                        canvasAction->setShortcut(QKeySequence());
                    }
                }
                canvasActionCollection->addAction(toolActionID, toolAction);
            }
            it.value()->setEnabled(true);
        }
        canvasActionCollection->readSettings(); // The shortcuts might have been configured in the meantime.
    }

    void deactivateToolActions()
    {

        if (!activeTool)
            return;
        // disable actions of active tool
        Q_FOREACH (QAction *action, activeTool->actions()) {
            action->setEnabled(false);
        }

        // enable actions which where disabled on activating the active tool
        // and re-add them to the action collection
        KActionCollection *ac = canvas->actionCollection();
        Q_FOREACH (QPointer<QAction> action, disabledDisabledActions) {
            if (action) {
                if (ac) {
                    ac->addAction(action->objectName(), action);
                }
            }
        }
        disabledDisabledActions.clear();

        Q_FOREACH (QPointer<QAction> action, disabledActions) {
            if (action) {
                action->setEnabled(true);
                if(ac) {
                    ac->addAction(action->objectName(), action);
                }
            }
        }
        disabledActions.clear();

        QMap<QPointer<QAction>, QString>::const_iterator it(disabledCanvasShortcuts.constBegin());
        for (; it != disabledCanvasShortcuts.constEnd(); ++it) {
            QAction *action = it.key();
            QString shortcut = it.value();
            action->setShortcut(shortcut);
        }
        disabledCanvasShortcuts.clear();
    }

    KoToolBase *activeTool;     // active Tool
    QString activeToolId;   // the id of the active Tool
    QString activationShapeId; // the shape-type (KoShape::shapeId()) the activeTool 'belongs' to.
    QHash<QString, KoToolBase*> allTools; // all the tools that are created for this canvas.
    QStack<QString> stack; // stack of temporary tools
    KoCanvasController *const canvas;
    const KoInputDevice inputDevice;
    QWidget *dummyToolWidget;  // the widget shown in the toolDocker.
    QLabel *dummyToolLabel;
    QList<QPointer<QAction> > disabledActions; ///< disabled conflicting actions
    QList<QPointer<QAction> > disabledDisabledActions; ///< disabled conflicting actions that were already disabled
    QMap<QPointer<QAction>, QString> disabledCanvasShortcuts; ///< Shortcuts that were temporarily removed from canvas actions because the tool overrides
};


// ******** KoToolManager **********
KoToolManager::KoToolManager()
    : QObject(),
    d(new Private(this))
{
    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(movedFocus(QWidget*,QWidget*)));
}

KoToolManager::~KoToolManager()
{
    delete d;
}

QList<KoToolAction*> KoToolManager::toolActionList() const
{
    QList<KoToolAction*> answer;
    answer.reserve(d->tools.count());
    Q_FOREACH (ToolHelper *tool, d->tools) {
        if (tool->id() == KoCreateShapesTool_ID)
            continue; // don't show this one.
        answer.append(tool->toolAction());
    }
    return answer;
}

void KoToolManager::requestToolActivation(KoCanvasController * controller)
{
    if (d->canvasses.contains(controller)) {
        QString activeToolId = d->canvasses.value(controller).first()->activeToolId;
        Q_FOREACH (ToolHelper * th, d->tools) {
            if (th->id() == activeToolId) {
                d->toolActivated(th);
                break;
            }
        }
    }
}

KoInputDevice KoToolManager::currentInputDevice() const
{
    return d->inputDevice;
}

void KoToolManager::registerToolActions(KActionCollection *ac, KoCanvasController *controller)
{
    Q_ASSERT(controller);
    Q_ASSERT(ac);

    d->setup();

    if (!d->canvasses.contains(controller)) {
        return;
    }

    // Actions available during the use of individual tools
    CanvasData *cd = d->canvasses.value(controller).first();
    Q_FOREACH (KoToolBase *tool, cd->allTools) {
        QHash<QString, QAction*> actions = tool->actions();
        QHash<QString, QAction*>::const_iterator action(actions.constBegin());
        for (; action != actions.constEnd(); ++action) {
            if (!ac->action(action.key()))
                ac->addAction(action.key(), action.value());
        }
    }

    // Actions used to switch tools via shortcuts
    Q_FOREACH (ToolHelper * th, d->tools) {
        if (ac->action(th->id())) {
            continue;
        }
        ShortcutToolAction* action = th->createShortcutToolAction(ac);
        ac->addCategorizedAction(th->id(), action, "tool-shortcuts");
    }
}

void KoToolManager::addController(KoCanvasController *controller)
{
    Q_ASSERT(controller);
    if (d->canvasses.contains(controller))
        return;
    d->setup();
    d->attachCanvas(controller);
    connect(controller->proxyObject, SIGNAL(destroyed(QObject*)), this, SLOT(attemptCanvasControllerRemoval(QObject*)));
    connect(controller->proxyObject, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    connect(controller->proxyObject, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::removeCanvasController(KoCanvasController *controller)
{
    Q_ASSERT(controller);
    disconnect(controller->proxyObject, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    disconnect(controller->proxyObject, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
    d->detachCanvas(controller);
}

void KoToolManager::attemptCanvasControllerRemoval(QObject* controller)
{
    KoCanvasControllerProxyObject* controllerActual = qobject_cast<KoCanvasControllerProxyObject*>(controller);
    if (controllerActual) {
        removeCanvasController(controllerActual->canvasController());
    }
}

void KoToolManager::switchToolRequested(const QString & id)
{
    Q_ASSERT(d->canvasData);
    if (!d->canvasData) return;

    while (!d->canvasData->stack.isEmpty()) // switching means to flush the stack
        d->canvasData->stack.pop();
    d->switchTool(id, false);
}

void KoToolManager::switchInputDeviceRequested(const KoInputDevice &id)
{
    if (!d->canvasData) return;
    d->switchInputDevice(id);
}

void KoToolManager::switchToolTemporaryRequested(const QString &id)
{
    d->switchTool(id, true);
}

void KoToolManager::switchBackRequested()
{
    if (!d->canvasData) return;

    if (d->canvasData->stack.isEmpty()) {
        // default to changing to the interactionTool
        d->switchTool(KoInteractionTool_ID, false);
        return;
    }
    d->switchTool(d->canvasData->stack.pop(), false);
}

KoCreateShapesTool * KoToolManager::shapeCreatorTool(KoCanvasBase *canvas) const
{
    Q_ASSERT(canvas);
    Q_FOREACH (KoCanvasController *controller, d->canvasses.keys()) {
        if (controller->canvas() == canvas) {
            KoCreateShapesTool *createTool = dynamic_cast<KoCreateShapesTool*>
                                             (d->canvasData->allTools.value(KoCreateShapesTool_ID));
            Q_ASSERT(createTool /* ID changed? */);
            return createTool;
        }
    }
    Q_ASSERT(0);   // this should not happen
    return 0;
}

KoToolBase *KoToolManager::toolById(KoCanvasBase *canvas, const QString &id) const
{
    Q_ASSERT(canvas);
    Q_FOREACH (KoCanvasController *controller, d->canvasses.keys()) {
        if (controller->canvas() == canvas)
            return d->canvasData->allTools.value(id);
    }
    return 0;
}

KoCanvasController *KoToolManager::activeCanvasController() const
{
    if (! d->canvasData) return 0;
    return d->canvasData->canvas;
}

QString KoToolManager::preferredToolForSelection(const QList<KoShape*> &shapes)
{
    QSet<QString> shapeTypes;
    Q_FOREACH (KoShape *shape, shapes) {
        shapeTypes << shape->shapeId();
    }
    //KritaUtils::makeContainerUnique(types);

    QString toolType = KoInteractionTool_ID;
    int prio = INT_MAX;
    Q_FOREACH (ToolHelper *helper, d->tools) {
        if (helper->id() == KoCreateShapesTool_ID) continue;
        if (helper->priority() >= prio)
            continue;

        bool toolWillWork = false;
        foreach (const QString &type, shapeTypes) {
            if (helper->activationShapeId().split(',').contains(type)) {
                toolWillWork = true;
                break;
            }
        }

        if (toolWillWork) {
            toolType = helper->id();
            prio = helper->priority();
        }
    }
    return toolType;
}


QPair<QString, KoToolBase*> KoToolManager::createTools(KoCanvasController *controller, ToolHelper *tool)
{
    // XXX: maybe this method should go into the private class?

    QHash<QString, KoToolBase*> origHash;

    if (d->canvasses.contains(controller)) {
        origHash = d->canvasses.value(controller).first()->allTools;
    }

    if (origHash.contains(tool->id())) {
        return QPair<QString, KoToolBase*>(tool->id(), origHash.value(tool->id()));
    }

    debugFlake << "Creating tool" << tool->id() << ". Activated on:" << tool->activationShapeId() << ", prio:" << tool->priority();

    KoToolBase *tl = tool->createTool(controller->canvas());
    if (tl) {
        d->uniqueToolIds.insert(tl, tool->uniqueId());

        tl->setObjectName(tool->id());

        Q_FOREACH (QAction *action, tl->actions()) {
            action->setEnabled(false);
        }

    }

    KoZoomTool *zoomTool = dynamic_cast<KoZoomTool*>(tl);
    if (zoomTool) {
        zoomTool->setCanvasController(controller);
    }

    return QPair<QString, KoToolBase*>(tool->id(), tl);
}


// NOT IMPLEMENTED
void KoToolManager::updateToolShortcuts()
{
    // auto actionRegistry = KisActionRegistry::instance();
    // foreach (KoToolBase *t, allTools) {
    //     for (auto it = t->actions().constBegin();
    //          it != t->actions().constEnd();
    //          ++it;) {
    //         actionRegistry->updateShortcut(it.key(), it.value());
    //     }
    // }
}

void KoToolManager::initializeCurrentToolForCanvas()
{
    d->postSwitchTool(false);
}

KoToolManager* KoToolManager::instance()
{
    return s_instance;
}

QString KoToolManager::activeToolId() const
{
    if (!d->canvasData) return QString();
    return d->canvasData->activeToolId;
}


KoToolManager::Private *KoToolManager::priv()
{
    return d;
}


/**** KoToolManager::Private ****/

KoToolManager::Private::Private(KoToolManager *qq)
    : q(qq),
    canvasData(0),
    layerExplicitlyDisabled(false)
{
}

KoToolManager::Private::~Private()
{
    qDeleteAll(tools);
}

// helper method.
CanvasData *KoToolManager::Private::createCanvasData(KoCanvasController *controller, const KoInputDevice &device)
{
    QHash<QString, KoToolBase*> toolsHash;
    Q_FOREACH (ToolHelper *tool, tools) {
        QPair<QString, KoToolBase*> toolPair = q->createTools(controller, tool);
        if (toolPair.second) { // only if a real tool was created
            toolsHash.insert(toolPair.first, toolPair.second);
        }
    }
    KoCreateShapesTool *createShapesTool = dynamic_cast<KoCreateShapesTool*>(toolsHash.value(KoCreateShapesTool_ID));
    KIS_ASSERT(createShapesTool);
    QString id = KoShapeRegistry::instance()->keys()[0];
    createShapesTool->setShapeId(id);

    CanvasData *cd = new CanvasData(controller, device);
    cd->allTools = toolsHash;
    return cd;
}

void KoToolManager::Private::setup()
{
    if (tools.size() > 0)
        return;

    KoShapeRegistry::instance();
    KoToolRegistry *registry = KoToolRegistry::instance();
    Q_FOREACH (const QString & id, registry->keys()) {
        ToolHelper *t = new ToolHelper(registry->value(id));
        tools.append(t);
    }

    // connect to all tools so we can hear their button-clicks
    Q_FOREACH (ToolHelper *tool, tools)
        connect(tool, SIGNAL(toolActivated(ToolHelper*)), q, SLOT(toolActivated(ToolHelper*)));

    // load pluggable input devices
    KoInputDeviceHandlerRegistry::instance();
}

void KoToolManager::Private::connectActiveTool()
{
    if (canvasData->activeTool) {
        connect(canvasData->activeTool, SIGNAL(cursorChanged(QCursor)),
                q, SLOT(updateCursor(QCursor)));
        connect(canvasData->activeTool, SIGNAL(activateTool(QString)),
                q, SLOT(switchToolRequested(QString)));
        connect(canvasData->activeTool, SIGNAL(activateTemporary(QString)),
                q, SLOT(switchToolTemporaryRequested(QString)));
        connect(canvasData->activeTool, SIGNAL(done()), q, SLOT(switchBackRequested()));
        connect(canvasData->activeTool, SIGNAL(statusTextChanged(QString)),
                q, SIGNAL(changedStatusText(QString)));
    }

    // we expect the tool to emit a cursor on activation.
    updateCursor(Qt::ForbiddenCursor);
}



void KoToolManager::Private::disconnectActiveTool()
{
    if (canvasData->activeTool) {
        canvasData->deactivateToolActions();
        // repaint the decorations before we deactivate the tool as it might deleted
        // data needed for the repaint
        emit q->aboutToChangeTool(canvasData->canvas);
        canvasData->activeTool->deactivate();
        disconnect(canvasData->activeTool, SIGNAL(cursorChanged(QCursor)),
                   q, SLOT(updateCursor(QCursor)));
        disconnect(canvasData->activeTool, SIGNAL(activateTool(QString)),
                   q, SLOT(switchToolRequested(QString)));
        disconnect(canvasData->activeTool, SIGNAL(activateTemporary(QString)),
                   q, SLOT(switchToolTemporaryRequested(QString)));
        disconnect(canvasData->activeTool, SIGNAL(done()), q, SLOT(switchBackRequested()));
        disconnect(canvasData->activeTool, SIGNAL(statusTextChanged(QString)),
                   q, SIGNAL(changedStatusText(QString)));
    }

    // emit a empty status text to clear status text from last active tool
    emit q->changedStatusText(QString());
}


void KoToolManager::Private::switchTool(KoToolBase *tool, bool temporary)
{

    Q_ASSERT(tool);
    if (canvasData == 0)
        return;

    if (canvasData->activeTool == tool && tool->toolId() != KoInteractionTool_ID)
        return;

    disconnectActiveTool();
    canvasData->activeTool = tool;
    connectActiveTool();
    postSwitchTool(temporary);
}



void KoToolManager::Private::switchTool(const QString &id, bool temporary)
{
    Q_ASSERT(canvasData);
    if (!canvasData) return;

    if (canvasData->activeTool && temporary)
        canvasData->stack.push(canvasData->activeToolId);
    canvasData->activeToolId = id;
    KoToolBase *tool = canvasData->allTools.value(id);
    if (! tool) {
        return;
    }

    Q_FOREACH (ToolHelper *th, tools) {
        if (th->id() == id) {
            canvasData->activationShapeId = th->activationShapeId();
            break;
        }
    }

    switchTool(tool, temporary);
}

void KoToolManager::Private::postSwitchTool(bool temporary)
{
#ifndef NDEBUG
    int canvasCount = 1;
    Q_FOREACH (QList<CanvasData*> list, canvasses) {
        bool first = true;
        Q_FOREACH (CanvasData *data, list) {
            if (first) {
                debugFlake << "Canvas" << canvasCount++;
            }
            debugFlake << "  +- Tool:" << data->activeToolId  << (data == canvasData ? " *" : "");
            first = false;
        }
    }
#endif
    Q_ASSERT(canvasData);
    if (!canvasData) return;

    KoToolBase::ToolActivation toolActivation;
    if (temporary)
        toolActivation = KoToolBase::TemporaryActivation;
    else
        toolActivation = KoToolBase::DefaultActivation;
    QSet<KoShape*> shapesToOperateOn;
    if (canvasData->activeTool
            && canvasData->activeTool->canvas()
            && canvasData->activeTool->canvas()->shapeManager()) {
        KoSelection *selection = canvasData->activeTool->canvas()->shapeManager()->selection();
        Q_ASSERT(selection);

        shapesToOperateOn = QSet<KoShape*>::fromList(selection->selectedEditableShapesAndDelegates());
    }

    if (canvasData->canvas->canvas()) {
        // Caller of postSwitchTool expect this to be called to update the selected tool
        updateToolForProxy();
        canvasData->activeTool->activate(toolActivation, shapesToOperateOn);
        KoCanvasBase *canvas = canvasData->canvas->canvas();
        canvas->updateInputMethodInfo();
    } else {
        canvasData->activeTool->activate(toolActivation, shapesToOperateOn);
    }

    QList<QPointer<QWidget> > optionWidgetList = canvasData->activeTool->optionWidgets();
    if (optionWidgetList.empty()) { // no option widget.
        QWidget *toolWidget;
        QString title;
        Q_FOREACH (ToolHelper *tool, tools) {
            if (tool->id() == canvasData->activeTool->toolId()) {
                title = tool->toolTip();
                break;
            }
        }
        toolWidget = canvasData->dummyToolWidget;
        if (toolWidget == 0) {
            toolWidget = new QWidget();
            toolWidget->setObjectName("DummyToolWidget");
            QVBoxLayout *layout = new QVBoxLayout(toolWidget);
            layout->setMargin(3);
            canvasData->dummyToolLabel = new QLabel(toolWidget);
            layout->addWidget(canvasData->dummyToolLabel);
            layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
            toolWidget->setLayout(layout);
            canvasData->dummyToolWidget = toolWidget;
        }
        canvasData->dummyToolLabel->setText(i18n("Active tool: %1", title));
        optionWidgetList.append(toolWidget);
    }

    // Activate the actions for the currently active tool
    canvasData->activateToolActions();

    emit q->changedTool(canvasData->canvas, uniqueToolIds.value(canvasData->activeTool));

    emit q->toolOptionWidgetsChanged(canvasData->canvas, optionWidgetList);
}


void KoToolManager::Private::switchCanvasData(CanvasData *cd)
{
    Q_ASSERT(cd);

    KoCanvasBase *oldCanvas = 0;
    KoInputDevice oldInputDevice;

    if (canvasData) {
        oldCanvas = canvasData->canvas->canvas();
        oldInputDevice = canvasData->inputDevice;

        if (canvasData->activeTool) {
            disconnectActiveTool();
        }

        KoToolProxy *proxy = proxies.value(oldCanvas);
        Q_ASSERT(proxy);
        proxy->setActiveTool(0);
    }

    canvasData = cd;
    inputDevice = canvasData->inputDevice;

    if (canvasData->activeTool) {
        connectActiveTool();
        postSwitchTool(false);
    }

    if (oldInputDevice != canvasData->inputDevice) {
        emit q->inputDeviceChanged(canvasData->inputDevice);
    }

    if (oldCanvas != canvasData->canvas->canvas()) {
        emit q->changedCanvas(canvasData->canvas->canvas());
    }
}


void KoToolManager::Private::toolActivated(ToolHelper *tool)
{
    Q_ASSERT(tool);

    Q_ASSERT(canvasData);
    if (!canvasData) return;
    KoToolBase *t = canvasData->allTools.value(tool->id());
    Q_ASSERT(t);

    canvasData->activeToolId = tool->id();
    canvasData->activationShapeId = tool->activationShapeId();

    switchTool(t, false);
}

void KoToolManager::Private::detachCanvas(KoCanvasController *controller)
{
    Q_ASSERT(controller);
    // check if we are removing the active canvas controller
    if (canvasData && canvasData->canvas == controller) {
        KoCanvasController *newCanvas = 0;
        // try to find another canvas controller beside the one we are removing
        Q_FOREACH (KoCanvasController* canvas, canvasses.keys()) {
            if (canvas != controller) {
                // yay found one
                newCanvas = canvas;
                break;
            }
        }
        if (newCanvas) {
            switchCanvasData(canvasses.value(newCanvas).first());
        } else {
            emit q->toolOptionWidgetsChanged(controller, QList<QPointer<QWidget> >());
            // as a last resort just set a blank one
            canvasData = 0;
        }
    }

    KoToolProxy *proxy = proxies.value(controller->canvas());
    if (proxy)
        proxy->setActiveTool(0);

    QList<KoToolBase *> tools;
    Q_FOREACH (CanvasData *canvasData, canvasses.value(controller)) {
        Q_FOREACH (KoToolBase *tool, canvasData->allTools) {
            if (! tools.contains(tool)) {
                tools.append(tool);
            }
        }
        delete canvasData;
    }
    Q_FOREACH (KoToolBase *tool, tools) {
        uniqueToolIds.remove(tool);
        delete tool;
    }
    canvasses.remove(controller);
    emit q->changedCanvas(canvasData ? canvasData->canvas->canvas() : 0);
}

void KoToolManager::Private::attachCanvas(KoCanvasController *controller)
{
    Q_ASSERT(controller);
    CanvasData *cd = createCanvasData(controller, KoInputDevice::mouse());

    // switch to new canvas as the active one.
    switchCanvasData(cd);

    inputDevice = cd->inputDevice;
    QList<CanvasData*> canvasses_;
    canvasses_.append(cd);
    canvasses[controller] = canvasses_;

    KoToolProxy *tp = proxies[controller->canvas()];
    if (tp)
        tp->priv()->setCanvasController(controller);

    if (cd->activeTool == 0) {
        // no active tool, so we activate the highest priority main tool
        int highestPriority = INT_MAX;
        ToolHelper * helper = 0;
        Q_FOREACH (ToolHelper * th, tools) {
            if (th->section() == KoToolFactoryBase::mainToolType()) {
                if (th->priority() < highestPriority) {
                    highestPriority = qMin(highestPriority, th->priority());
                    helper = th;
                }
            }
        }
        if (helper)
            toolActivated(helper);
    }

    Connector *connector = new Connector(controller->canvas()->shapeManager());
    connect(connector, SIGNAL(selectionChanged(QList<KoShape*>)), q,
            SLOT(selectionChanged(QList<KoShape*>)));
    connect(controller->canvas()->selectedShapesProxy(),
            SIGNAL(currentLayerChanged(const KoShapeLayer*)),
            q, SLOT(currentLayerChanged(const KoShapeLayer*)));

    emit q->changedCanvas(canvasData ? canvasData->canvas->canvas() : 0);
}

void KoToolManager::Private::movedFocus(QWidget *from, QWidget *to)
{
    Q_UNUSED(from);
    // no canvas anyway or no focus set anyway?
    if (!canvasData || to == 0) {
        return;
    }

    // Check if this app is about QWidget-based KoCanvasControllerWidget canvasses
    // XXX: Focus handling for non-qwidget based canvases!
    KoCanvasControllerWidget *canvasControllerWidget = dynamic_cast<KoCanvasControllerWidget*>(canvasData->canvas);
    if (!canvasControllerWidget) {
        return;
    }

    // canvasWidget is set as focusproxy for KoCanvasControllerWidget,
    // so all focus checks are to be done against canvasWidget objects

    // focus returned to current canvas?
    if (to == canvasData->canvas->canvas()->canvasWidget()) {
        // nothing to do
        return;
    }

    // if the 'to' is one of our canvasWidgets, then switch.

    // for code simplicity the current canvas will be checked again,
    // but would have been caught already in the lines above, so no issue
    KoCanvasController *newCanvas = 0;
    Q_FOREACH (KoCanvasController* canvas, canvasses.keys()) {
        if (canvas->canvas()->canvasWidget() == to) {
            newCanvas = canvas;
            break;
        }
    }

    // none of our canvasWidgets got focus?
    if (newCanvas == 0) {
        return;
    }

    // switch to canvasdata matching inputdevice used last with this app instance
    Q_FOREACH (CanvasData *data, canvasses.value(newCanvas)) {
        if (data->inputDevice == inputDevice) {
            switchCanvasData(data);
            return;
        }
    }
    // if no such inputDevice for this canvas, then simply fallback to first one
    switchCanvasData(canvasses.value(newCanvas).first());
}

void KoToolManager::Private::updateCursor(const QCursor &cursor)
{
    Q_ASSERT(canvasData);
    Q_ASSERT(canvasData->canvas);
    Q_ASSERT(canvasData->canvas->canvas());
    canvasData->canvas->canvas()->setCursor(cursor);
}

void KoToolManager::Private::selectionChanged(const QList<KoShape*> &shapes)
{
    QList<QString> types;
    Q_FOREACH (KoShape *shape, shapes) {
        QSet<KoShape*> delegates = shape->toolDelegates();
        if (delegates.isEmpty()) { // no delegates, just the orig shape
            delegates << shape;
        }

        foreach (KoShape *shape2, delegates) {
            Q_ASSERT(shape2);
            if (! types.contains(shape2->shapeId())) {
                types.append(shape2->shapeId());
            }
        }
    }

    // check if there is still a shape selected the active tool can work on
    // there needs to be at least one shape for a tool without an activationShapeId
    // to work
    // if not change the current tool to the default tool

    const QStringList activationShapeIds = canvasData->activationShapeId.split(',');

    if (!(canvasData->activationShapeId.isNull() && shapes.size() > 0)
        && !activationShapeIds.contains("flake/always")
        && !activationShapeIds.contains("flake/edit")) {

        bool currentToolWorks = false;
        foreach (const QString &type, types) {
            if (activationShapeIds.contains(type)) {
                currentToolWorks = true;
                break;
            }
        }
        if (!currentToolWorks) {
            switchTool(KoInteractionTool_ID, false);
        }
    }

    emit q->toolCodesSelected(types);
}

void KoToolManager::Private::currentLayerChanged(const KoShapeLayer *layer)
{
    emit q->currentLayerChanged(canvasData->canvas, layer);
    layerExplicitlyDisabled = layer && !layer->isShapeEditable();
    updateToolForProxy();

    debugFlake << "Layer changed to" << layer << "explicitly disabled:" << layerExplicitlyDisabled;
}

void KoToolManager::Private::updateToolForProxy()
{
    KoToolProxy *proxy = proxies.value(canvasData->canvas->canvas());
    if(!proxy) return;

    bool canUseTool = !layerExplicitlyDisabled || canvasData->activationShapeId.endsWith(QLatin1String("/always"));
    proxy->setActiveTool(canUseTool ? canvasData->activeTool : 0);
}

void KoToolManager::Private::switchInputDevice(const KoInputDevice &device)
{
    Q_ASSERT(canvasData);
    if (!canvasData) return;
    if (inputDevice == device) return;
    if (inputDevice.isMouse() && device.isMouse()) return;
    if (device.isMouse() && !inputDevice.isMouse()) {
        // we never switch back to mouse from a tablet input device, so the user can use the
        // mouse to edit the settings for a tool activated by a tablet. See bugs
        // https://bugs.kde.org/show_bug.cgi?id=283130 and https://bugs.kde.org/show_bug.cgi?id=285501.
        // We do continue to switch between tablet devices, thought.
        return;
    }

    QList<CanvasData*> items = canvasses[canvasData->canvas];

    // disable all actions for all tools in the all canvasdata objects for this canvas.
    Q_FOREACH (CanvasData *cd, items) {
        Q_FOREACH (KoToolBase* tool, cd->allTools) {
            Q_FOREACH (QAction * action, tool->actions()) {
                action->setEnabled(false);
            }
        }
    }

    // search for a canvasdata object for the current input device
    Q_FOREACH (CanvasData *cd, items) {
        if (cd->inputDevice == device) {
            switchCanvasData(cd);

            if (!canvasData->activeTool) {
                switchTool(KoInteractionTool_ID, false);
            }

            return;
        }
    }

    // still here?  That means we need to create a new CanvasData instance with the current InputDevice.
    CanvasData *cd = createCanvasData(canvasData->canvas, device);
    // switch to new canvas as the active one.
    QString oldTool = canvasData->activeToolId;

    items.append(cd);
    canvasses[cd->canvas] = items;

    switchCanvasData(cd);

    q->switchToolRequested(oldTool);
}

void KoToolManager::Private::registerToolProxy(KoToolProxy *proxy, KoCanvasBase *canvas)
{
    proxies.insert(canvas, proxy);
    Q_FOREACH (KoCanvasController *controller, canvasses.keys()) {
        if (controller->canvas() == canvas) {
            proxy->priv()->setCanvasController(controller);
            break;
        }
    }
}

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoToolManager.cpp"
