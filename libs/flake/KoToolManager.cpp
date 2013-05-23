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
#include "KoCanvasBase.h"
#include "KoInputDeviceHandlerRegistry.h"
#include "KoInputDeviceHandlerEvent.h"
#include "KoPointerEvent.h"
#include "tools/KoCreateShapesTool.h"
#include "tools/KoZoomTool.h"
#include "tools/KoPanTool.h"

// Qt + kde
#include <QWidget>
#include <QEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTabletEvent>
#include <QKeyEvent>
#include <QGridLayout>
#include <QDockWidget>
#include <QGraphicsWidget>
#include <QStringList>
#include <QAbstractButton>
#include <QApplication>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kaction.h>
#include <QStack>
#include <QLabel>

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
        // we do several things here
        // 1. enable the actions of the active tool
        // 2. disable conflicting actions
        // 3. replace conflicting actions in the action collection
        KActionCollection *ac = canvas->actionCollection();

        QHash<QString, KAction*> toolActions = activeTool->actions();
        QHash<QString, KAction*>::const_iterator it(toolActions.constBegin());

        for (; it != toolActions.constEnd(); ++it) {
            if (ac) {
                KAction* action = qobject_cast<KAction*>(ac->action(it.key()));
                if (action) {
                    ac->takeAction(action);
                    if (action != it.value()) {
                        if (action->isEnabled()) {
                            action->setEnabled(false);
                            disabledActions.append(action);
                        } else  {
                            disabledDisabledActions.append(action);
                        }
                    }
                }
                ac->addAction(it.key(), it.value());
            }
            it.value()->setEnabled(true);
        }
    }

    void deactivateToolActions()
    {

        if (!activeTool)
            return;
        // disable actions of active tool
        foreach(KAction *action, activeTool->actions()) {
            action->setEnabled(false);
        }

        // enable actions which where disabled on activating the active tool
        // and re-add them to the action collection
        KActionCollection *ac = canvas->actionCollection();
        foreach(KAction *action, disabledDisabledActions) {
            if(ac) {
                ac->addAction(action->objectName(), action);
            }
        }
        disabledDisabledActions.clear();
        foreach(QAction *action, disabledActions) {
            action->setEnabled(true);
            if(ac) {
                ac->addAction(action->objectName(), action);
            }
        }
        disabledActions.clear();
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
    QList<KAction*> disabledActions; ///< disabled conflicting actions
    QList<KAction*> disabledDisabledActions; ///< disabled conflicting actions that were already disabled
};

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
CanvasData *KoToolManager::Private::createCanvasData(KoCanvasController *controller, KoInputDevice device)
{
    QHash<QString, KoToolBase*> toolsHash;
    foreach(ToolHelper *tool, tools) {
        QPair<QString, KoToolBase*> toolPair = q->createTools(controller, tool);
        if (toolPair.second) { // only if a real tool was created
            toolsHash.insert(toolPair.first, toolPair.second);
        }
    }
    KoCreateShapesTool *createShapesTool = dynamic_cast<KoCreateShapesTool*>(toolsHash.value(KoCreateShapesTool_ID));
    Q_ASSERT(createShapesTool);
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
    foreach(const QString & id, registry->keys()) {
        ToolHelper *t = new ToolHelper(registry->value(id));
        tools.append(t);
    }

    // connect to all tools so we can hear their button-clicks
    foreach(ToolHelper *tool, tools)
        connect(tool, SIGNAL(toolActivated(ToolHelper*)), q, SLOT(toolActivated(ToolHelper*)));

    // load pluggable input devices
    KoInputDeviceHandlerRegistry::instance();
}

void KoToolManager::Private::connectActiveTool()
{
    if (canvasData->activeTool) {
        connect(canvasData->activeTool, SIGNAL(cursorChanged(const QCursor &)),
                q, SLOT(updateCursor(const QCursor &)));
        connect(canvasData->activeTool, SIGNAL(activateTool(const QString &)),
                q, SLOT(switchToolRequested(const QString &)));
        connect(canvasData->activeTool, SIGNAL(activateTemporary(const QString &)),
                q, SLOT(switchToolTemporaryRequested(const QString &)));
        connect(canvasData->activeTool, SIGNAL(done()), q, SLOT(switchBackRequested()));
        connect(canvasData->activeTool, SIGNAL(statusTextChanged(const QString &)),
                q, SIGNAL(changedStatusText(const QString &)));
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
        canvasData->activeTool->deactivate();
        disconnect(canvasData->activeTool, SIGNAL(cursorChanged(const QCursor&)),
                   q, SLOT(updateCursor(const QCursor&)));
        disconnect(canvasData->activeTool, SIGNAL(activateTool(const QString &)),
                   q, SLOT(switchToolRequested(const QString &)));
        disconnect(canvasData->activeTool, SIGNAL(activateTemporary(const QString &)),
                   q, SLOT(switchToolTemporaryRequested(const QString &)));
        disconnect(canvasData->activeTool, SIGNAL(done()), q, SLOT(switchBackRequested()));
        disconnect(canvasData->activeTool, SIGNAL(statusTextChanged(const QString &)),
                   q, SIGNAL(changedStatusText(const QString &)));
    }

    // emit a empty status text to clear status text from last active tool
    emit q->changedStatusText(QString());
}

void KoToolManager::Private::switchTool(KoToolBase *tool, bool temporary)
{
    Q_UNUSED(temporary);
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
        kWarning(30006) << "KoToolManager::switchTool() " << (temporary ? "temporary" : "") << " got request to unknown tool: '" << id << "'";
        return;
    }

    foreach(ToolHelper *th, tools) {
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
    foreach(QList<CanvasData*> list, canvasses) {
        bool first = true;
        foreach(CanvasData *data, list) {
            if (first) {
                kDebug(30006) << "Canvas" << canvasCount++;
            }
            kDebug(30006) << "  +- Tool:" << data->activeToolId  << (data == canvasData ? " *" : "");
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

        foreach(KoShape *shape, selection->selectedShapes()) {
            QSet<KoShape*> delegates = shape->toolDelegates();
            if (delegates.isEmpty()) { // no delegates, just the orig shape
                shapesToOperateOn << shape;
            } else {
                shapesToOperateOn += delegates;
            }
        }
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

    QList<QWidget *> optionWidgetList = canvasData->activeTool->optionWidgets();
    if (optionWidgetList.empty()) { // no option widget.
        QWidget *toolWidget;
        QString title;
        foreach(ToolHelper *tool, tools) {
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

    KoCanvasControllerWidget *canvasControllerWidget = dynamic_cast<KoCanvasControllerWidget*>(canvasData->canvas);
    if (canvasControllerWidget) {
        canvasControllerWidget->setToolOptionWidgets(optionWidgetList);
    }
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

    KoCanvasControllerWidget *canvasControllerWidget = dynamic_cast<KoCanvasControllerWidget*>(canvasData->canvas);
    if (canvasControllerWidget) {
        canvasControllerWidget->activate();
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
        foreach(KoCanvasController* canvas, canvasses.keys()) {
            if (canvas != controller) {
                // yay found one
                newCanvas = canvas;
                break;
            }
        }
        if (newCanvas) {
            switchCanvasData(canvasses.value(newCanvas).first());
        } else {
            KoCanvasControllerWidget *canvasControllerWidget = dynamic_cast<KoCanvasControllerWidget*>(canvasData->canvas);
            if (canvasControllerWidget) {
                canvasControllerWidget->setToolOptionWidgets(QList<QWidget *>());
            }
            // as a last resort just set a blank one
            canvasData = 0;
            // and stop the event filter
            QApplication::instance()->removeEventFilter(q);
        }
    }

    KoToolProxy *proxy = proxies.value(controller->canvas());
    if (proxy)
        proxy->setActiveTool(0);

    QList<KoToolBase *> tools;
    foreach(CanvasData *canvasData, canvasses.value(controller)) {
        foreach(KoToolBase *tool, canvasData->allTools) {
            if (! tools.contains(tool)) {
                tools.append(tool);
            }
        }
        delete canvasData;
    }
    foreach(KoToolBase *tool, tools) {
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
    if (canvasData == 0) {
        QApplication::instance()->installEventFilter(q);
    }
    canvasData = cd;
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
        foreach(ToolHelper * th, tools) {
            if (th->toolType() == KoToolFactoryBase::mainToolType()) {
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
    connect(controller->canvas()->shapeManager()->selection(),
            SIGNAL(currentLayerChanged(const KoShapeLayer*)),
            q, SLOT(currentLayerChanged(const KoShapeLayer*)));

    KoCanvasControllerWidget *canvasControllerWidget = dynamic_cast<KoCanvasControllerWidget*>(canvasData->canvas);
    if (canvasControllerWidget) {
        canvasControllerWidget->activate();
    }

    emit q->changedCanvas(canvasData ? canvasData->canvas->canvas() : 0);
}

void KoToolManager::Private::movedFocus(QWidget *from, QWidget *to)
{
    Q_UNUSED(from);
    // XXX: Focus handling for non-qwidget based canvases!
    if (!canvasData) {
        return;
    }

    KoCanvasControllerWidget *canvasControllerWidget = dynamic_cast<KoCanvasControllerWidget*>(canvasData->canvas);
    if (!canvasControllerWidget) {
        return;
    }

    if (to == 0 || to == canvasControllerWidget) {
        return;
    }

    KoCanvasController *newCanvas = 0;
    // if the 'to' is one of our canvasses, or one of its children, then switch.
    foreach(KoCanvasController* canvas, canvasses.keys()) {
        if (canvasControllerWidget == to || canvas->canvas()->canvasWidget() == to) {
            newCanvas = canvas;
            break;
        }
    }

    if (newCanvas == 0)
        return;
    if (canvasData && newCanvas == canvasData->canvas)
        return;

    if (! canvasses.contains(newCanvas))
        return;
    foreach(CanvasData *data, canvasses.value(newCanvas)) {
        if (data->inputDevice == inputDevice) {
            switchCanvasData(data);
            return;
        }
    }
    // no such inputDevice for this canvas...
    switchCanvasData(canvasses.value(newCanvas).first());
}

void KoToolManager::Private::updateCursor(const QCursor &cursor)
{
    Q_ASSERT(canvasData);
    Q_ASSERT(canvasData->canvas);
    Q_ASSERT(canvasData->canvas->canvas());
    canvasData->canvas->canvas()->setCursor(cursor);
}

void KoToolManager::Private::selectionChanged(QList<KoShape*> shapes)
{
    QList<QString> types;
    foreach(KoShape *shape, shapes) {
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
    if (!(canvasData->activationShapeId.isNull() && shapes.size() > 0)
        && canvasData->activationShapeId != "flake/always"
        && canvasData->activationShapeId != "flake/edit"
        && ! types.contains(canvasData->activationShapeId)) {
        switchTool(KoInteractionTool_ID, false);
    }

    emit q->toolCodesSelected(canvasData->canvas, types);
}

void KoToolManager::Private::currentLayerChanged(const KoShapeLayer *layer)
{
    emit q->currentLayerChanged(canvasData->canvas, layer);
    layerExplicitlyDisabled = layer && !layer->isEditable();
    updateToolForProxy();

    kDebug(30006) << "Layer changed to" << layer << "explicitly disabled:" << layerExplicitlyDisabled;
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
    if (device.isMouse() && !inputDevice.isMouse()) {
        // we never switch back to mouse from a tablet input device, so the user can use the
        // mouse to edit the settings for a tool activated by a tablet. See bugs
        // https://bugs.kde.org/show_bug.cgi?id=283130 and https://bugs.kde.org/show_bug.cgi?id=285501.
        // We do continue to switch between tablet devices, thought.
        return;
    }

    QList<CanvasData*> items = canvasses[canvasData->canvas];

    // disable all actions for all tools in the all canvasdata objects for this canvas.
    foreach(CanvasData *cd, items) {
        foreach(KoToolBase* tool, cd->allTools) {
            foreach(KAction* action, tool->actions()) {
                action->setEnabled(false);
            }
        }
    }

    // search for a canvasdata object for the current input device
    foreach(CanvasData *cd, items) {
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
    foreach(KoCanvasController *controller, canvasses.keys()) {
        if (controller->canvas() == canvas) {
            proxy->priv()->setCanvasController(controller);
            break;
        }
    }
}

void KoToolManager::Private::switchToolByShortcut(QKeyEvent *event)
{
    QKeySequence item(event->key() | ((Qt::ControlModifier | Qt::AltModifier) & event->modifiers()));

    if (event->key() == Qt::Key_Space && event->modifiers() == 0) {
        switchTool(KoPanTool_ID, true);
    } else if (event->key() == Qt::Key_Escape && event->modifiers() == 0) {
        switchTool(KoInteractionTool_ID, false);
    }
}

// ******** KoToolManager **********
KoToolManager::KoToolManager()
    : QObject(),
    d(new Private(this))
{
    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*, QWidget*)),
            this, SLOT(movedFocus(QWidget*, QWidget*)));
}

KoToolManager::~KoToolManager()
{
    delete d;
}

QList<KoToolButton> KoToolManager::createToolList(KoCanvasBase *canvas) const
{
    QList<KoToolButton> answer;
    foreach(ToolHelper *tool, d->tools) {
        if (tool->id() == KoCreateShapesTool_ID)
            continue; // don't show this one.
        if (!tool->canCreateTool(canvas))
            continue;
        KoToolButton button;
        button.button = tool->createButton();
        button.section = tool->toolType();
        button.priority = tool->priority();
        button.buttonGroupId = tool->uniqueId();
        button.visibilityCode = tool->activationShapeId();
        answer.append(button);
    }
    return answer;
}

void KoToolManager::requestToolActivation(KoCanvasController * controller)
{
    if (d->canvasses.contains(controller)) {
        QString activeToolId = d->canvasses.value(controller).first()->activeToolId;
        foreach(ToolHelper * th, d->tools) {
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

void KoToolManager::registerTools(KActionCollection *ac, KoCanvasController *controller)
{
    Q_ASSERT(controller);
    Q_ASSERT(ac);

    d->setup();

    if (! d->canvasses.contains(controller)) {
        kWarning(30006) << "registerTools called on a canvasController that has not been registered (yet)!";
        return;
    }
    CanvasData *cd = d->canvasses.value(controller).first();
    foreach(KoToolBase *tool, cd->allTools) {
        QHash<QString, KAction*> actions = tool->actions();
        QHash<QString, KAction*>::const_iterator it(actions.constBegin());
        for (; it != actions.constEnd(); ++it) {
            if (!ac->action(it.key()))
                ac->addAction(it.key(), it.value());
        }
    }
    foreach(ToolHelper * th, d->tools) {
        ToolAction* action = new ToolAction(this, th->id(), th->toolTip(), ac);
        action->setShortcut(th->shortcut());
        ac->addAction(th->id(), action);
    }
}

void KoToolManager::addController(KoCanvasController *controller)
{
    Q_ASSERT(controller);
    if (d->canvasses.keys().contains(controller))
        return;
    d->setup();
    d->attachCanvas(controller);
    connect(controller->proxyObject, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    connect(controller->proxyObject, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::removeCanvasController(KoCanvasController *controller)
{
    Q_ASSERT(controller);
    d->detachCanvas(controller);
    disconnect(controller->proxyObject, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    disconnect(controller->proxyObject, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::updateShapeControllerBase(KoShapeBasedDocumentBase *shapeController, KoCanvasController *canvasController)
{
    if (!d->canvasses.keys().contains(canvasController))
        return;

    QList<CanvasData *> canvasses = d->canvasses[canvasController];
    foreach(CanvasData *canvas, canvasses) {
        foreach(KoToolBase *tool, canvas->allTools.values()) {
            tool->updateShapeController(shapeController);
        }
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
    foreach(KoCanvasController *controller, d->canvasses.keys()) {
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
    foreach(KoCanvasController *controller, d->canvasses.keys()) {
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
    QList<QString> types;
    foreach(KoShape *shape, shapes)
        if (! types.contains(shape->shapeId()))
            types.append(shape->shapeId());

    QString toolType = KoInteractionTool_ID;
    int prio = INT_MAX;
    foreach(ToolHelper *helper, d->tools) {
        if (helper->priority() >= prio)
            continue;
        if (helper->toolType() == KoToolFactoryBase::mainToolType())
            continue;
        if (types.contains(helper->activationShapeId())) {
            toolType = helper->id();
            prio = helper->priority();
        }
    }
    return toolType;
}

bool KoToolManager::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
    case QEvent::TabletEnterProximity:
    case QEvent::TabletLeaveProximity: {
        QTabletEvent *tabletEvent = static_cast<QTabletEvent *>(event);

        KoInputDevice id(tabletEvent->device(), tabletEvent->pointerType(), tabletEvent->uniqueId());
        d->switchInputDevice(id);
        break;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
        d->switchInputDevice(KoInputDevice::mouse());
        break;
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

void KoToolManager::injectDeviceEvent(KoInputDeviceHandlerEvent * event)
{
    if (d->canvasData && d->canvasData->canvas->canvas()) {
        if (static_cast<KoInputDeviceHandlerEvent::Type>(event->type()) == KoInputDeviceHandlerEvent::ButtonPressed)
            d->canvasData->activeTool->customPressEvent(event->pointerEvent());
        else if (static_cast<KoInputDeviceHandlerEvent::Type>(event->type()) == KoInputDeviceHandlerEvent::ButtonReleased)
            d->canvasData->activeTool->customReleaseEvent(event->pointerEvent());
        else if (static_cast<KoInputDeviceHandlerEvent::Type>(event->type()) ==  KoInputDeviceHandlerEvent::PositionChanged)
            d->canvasData->activeTool->customMoveEvent(event->pointerEvent());
    }
}

void KoToolManager::addDeferredToolFactory(KoToolFactoryBase *toolFactory)
{
    ToolHelper *tool = new ToolHelper(toolFactory);
    // make sure all plugins are loaded as otherwise we will not load them
    d->setup();
    d->tools.append(tool);

    // connect to all tools so we can hear their button-clicks
    connect(tool, SIGNAL(toolActivated(ToolHelper*)), this, SLOT(toolActivated(ToolHelper*)));

    // now create tools for all existing canvases
    foreach(KoCanvasController *controller, d->canvasses.keys()) {

        // this canvascontroller is unknown, which is weird
        if (!d->canvasses.contains(controller)) {
            continue;
        }

        // create a tool for all canvasdata objects (i.e., all input devices on this canvas)
        foreach (CanvasData *cd, d->canvasses[controller]) {
            QPair<QString, KoToolBase*> toolPair = createTools(controller, tool);
            if (toolPair.second) {
                cd->allTools.insert(toolPair.first, toolPair.second);
            }
        }

        // Then create a button for the toolbox for this canvas
        if (tool->id() == KoCreateShapesTool_ID || !tool->canCreateTool(controller->canvas())) {
            continue;
        }

        KoToolButton button;
        button.button = tool->createButton();
        button.section = tool->toolType();
        button.priority = tool->priority();
        button.buttonGroupId = tool->uniqueId();
        button.visibilityCode = tool->activationShapeId();

        emit addedTool(button, controller);
    }

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

    if (!tool->canCreateTool(controller->canvas())) {
        kDebug(30006) << "Skipping the creation of tool" << tool->id();
        return  QPair<QString, KoToolBase*>(QString(), 0);
    }

    kDebug(30006) << "Creating tool" << tool->id() << ". Activated on:" << tool->activationShapeId() << ", prio:" << tool->priority();

    KoToolBase *tl = tool->createTool(controller->canvas());
    Q_ASSERT(tl);
    d->uniqueToolIds.insert(tl, tool->uniqueId());

    tl->setObjectName(tool->id());

    foreach(KAction *action, tl->actions()) {
        action->setEnabled(false);
    }

    KoZoomTool *zoomTool = dynamic_cast<KoZoomTool*>(tl);
    if (zoomTool) {
        zoomTool->setCanvasController(controller);
    }

    KoPanTool *panTool = dynamic_cast<KoPanTool*>(tl);
    if (panTool) {
        panTool->setCanvasController(controller);
    }

    return QPair<QString, KoToolBase*>(tool->id(), tl);
}


KoToolManager* KoToolManager::instance()
{
    K_GLOBAL_STATIC(KoToolManager, s_instance)
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

#include <KoToolManager.moc>
