/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
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
#include "tools/KoCreatePathToolFactory.h"
#include "tools/KoCreateShapesToolFactory.h"
#include "tools/KoCreateShapesTool.h"
#include "tools/KoPathToolFactory.h"
#include "KoCanvasController.h"
#include "KoShape.h"
#include "KoShapeRegistry.h"
#include "KoShapeManager.h"
#include "KoCanvasBase.h"
#include "KoDeviceRegistry.h"
#include "KoDeviceEvent.h"
#include "KoPointerEvent.h"
#include "tools/KoZoomTool.h"
#include "tools/KoZoomToolFactory.h"
#include "tools/KoPanTool.h"
#include "tools/KoPanToolFactory.h"
#include "tools/KoGuidesToolFactory.h"
#include "tools/KoGuidesTool.h"

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
#include <QStringList>
#include <QAbstractButton>
#include <QApplication>
#include <QTimer>
#include <kactioncollection.h>
#include <kdebug.h>
#include <k3staticdeleter.h>
#include <kaction.h>
#include <QStack>
#include <QLabel>

class CanvasData {
public:
    CanvasData(KoCanvasController *cc, const KoInputDevice &id)
        : activeTool(0),
        canvas(cc),
        inputDevice(id),
        dummyToolWidget(0),
        dummyToolLabel(0)
    {
    }

    KoTool *activeTool;     // active Tool
    QString activeToolId;   // the id of the active Tool
    QString activationShapeId; // the shape-type (KoShape::shapeId()) the activeTool 'belongs' to.
    QHash<QString, KoTool*> allTools; // all the tools that are created for this canvas.
    QStack<QString> stack; // stack of temporary tools
    KoCanvasController *const canvas;
    const KoInputDevice inputDevice;
    QWidget *dummyToolWidget;  // the widget shown in the toolDocker.
    QLabel *dummyToolLabel;
};

class KoToolManager::Private {
public:
    Private() : canvasData(0)
    {
        tabletEventTimer.setSingleShot(true);
    }
    ~Private()
    {
        qDeleteAll(tools);
    }

    QList<ToolHelper*> tools; // list of all available tools via their factories.

    QHash<KoTool*, int> uniqueToolIds; // for the changedTool signal
    QHash<KoCanvasController*, QList<CanvasData*> > canvasses;
    QHash<KoCanvasBase*, KoToolProxy*> proxies;

    CanvasData *canvasData; // data about the active canvas.

    KoInputDevice inputDevice;
    QTimer tabletEventTimer; // Runs for a short while after any tablet event is
                             // received to help correct input device detection.

    // helper method.
    CanvasData *createCanvasData(KoCanvasController *controller, KoInputDevice device) {
        QHash<QString, KoTool*> origHash;
        if(canvasses.contains(controller))
            origHash = canvasses.value(controller).first()->allTools;

        QHash<QString, KoTool*> toolsHash;
        foreach(ToolHelper *tool, tools) {
            if(tool->inputDeviceAgnostic() && origHash.contains(tool->id())) {
                // reuse ones that are marked as inputDeviceAgnostic();
                toolsHash.insert(tool->id(), origHash.value(tool->id()));
                continue;
            }
            kDebug(30006) <<"Creating tool" << tool->id() <<". Activated on:" << tool->activationShapeId() <<", prio:" << tool->priority();
            KoTool *tl = tool->createTool(controller->canvas());
            uniqueToolIds.insert(tl, tool->uniqueId());
            toolsHash.insert(tool->id(), tl);
            tl->setObjectName(tool->id());
            foreach(QAction *action, tl->actions().values())
                action->setEnabled(false);
            KoZoomTool *zoomTool = dynamic_cast<KoZoomTool*> (tl);
            if(zoomTool)
                zoomTool->setCanvasController(controller);
            KoPanTool *panTool = dynamic_cast<KoPanTool*> (tl);
            if(panTool)
                panTool->setCanvasController(controller);
        }
        KoCreateShapesTool *createTool = dynamic_cast<KoCreateShapesTool*>(toolsHash.value(KoCreateShapesTool_ID));
        Q_ASSERT(createTool);
        QString id = KoShapeRegistry::instance()->keys()[0];
        createTool->setShapeId(id);

        CanvasData *cd = new CanvasData(controller, device);
        cd->allTools = toolsHash;
        return cd;
    }
};

// ******** KoToolManager **********
KoToolManager::KoToolManager()
    : QObject(),
    d(new Private())
{
    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(movedFocus(QWidget*,QWidget*)));
    QApplication::instance()->installEventFilter(this);
}

KoToolManager::~KoToolManager() {
    delete d;
}

void KoToolManager::setup() {
    if (d->tools.size() > 0)
        return;

    d->tools.append( new ToolHelper(new KoCreatePathToolFactory(this)) );
    d->tools.append( new ToolHelper(new KoCreateShapesToolFactory(this)) );
    d->tools.append( new ToolHelper(new KoPathToolFactory(this)) );
    d->tools.append( new ToolHelper(new KoZoomToolFactory(this)) );
    d->tools.append( new ToolHelper(new KoPanToolFactory(this)) );
    d->tools.append( new ToolHelper(new KoGuidesToolFactory(this)) );

    KoShapeRegistry::instance();
    KoToolRegistry *registry = KoToolRegistry::instance();
    foreach(QString id, registry->keys()) {
        ToolHelper *t = new ToolHelper(registry->value(id));
        d->tools.append(t);
    }

    // connect to all tools so we can hear their button-clicks
    foreach(ToolHelper *tool, d->tools)
        connect(tool, SIGNAL(toolActivated(ToolHelper*)), this, SLOT(toolActivated(ToolHelper*)));

    // load pluggable input devices
    KoDeviceRegistry::instance();
}

QList<KoToolManager::Button> KoToolManager::createToolList() const {
    QList<KoToolManager::Button> answer;
    foreach(ToolHelper *tool, d->tools) {
        if(tool->id() == KoCreateShapesTool_ID)
            continue; // don't show this one.
        if(tool->id() == KoGuidesTool_ID)
            continue; // don't show this one.
        Button button;
        button.button = tool->createButton();
        button.section = tool->toolType();
        button.priority = tool->priority();
        button.buttonGroupId = tool->uniqueId();
        button.visibilityCode = tool->activationShapeId();
        answer.append(button);
    }
    return answer;
}

void KoToolManager::requestToolActivation( KoCanvasController * controller )
{
    if( d->canvasses.contains( controller ) )
    {
        QString activeToolId = d->canvasses.value( controller ).first()->activeToolId;
        foreach( ToolHelper * th, d->tools )
        {
            if( th->id() == activeToolId )
            {
                toolActivated( th );
                break;
            }
        }
    }
}

KoInputDevice KoToolManager::currentInputDevice() const {
    return d->inputDevice;
}

void KoToolManager::registerTools(KActionCollection *ac, KoCanvasController *controller) {
    setup();

    if(! d->canvasses.contains(controller)) {
        kWarning(30006) << "registerTools called on a canvasController that has not been registered (yet)!\n";
        return;
    }
    CanvasData *cd = d->canvasses.value(controller).first();
    foreach(KoTool *tool, cd->allTools) {
        QHash<QString, QAction*> actions = tool->actions();
        foreach(QString name, actions.keys())
            ac->addAction(name, actions[name]);
    }
}

void KoToolManager::addController(KoCanvasController *controller ) {
    if(d->canvasses.keys().contains(controller))
        return;
    setup();
    attachCanvas(controller);
    connect(controller, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    connect(controller, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::removeCanvasController(KoCanvasController *controller) {
    detachCanvas(controller);
    disconnect(controller, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    disconnect(controller, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::toolActivated(ToolHelper *tool) {
    Q_ASSERT(d->canvasData);
    KoTool *t = d->canvasData->allTools.value(tool->id());
    Q_ASSERT(t);

    d->canvasData->activeToolId = tool->id();
    d->canvasData->activationShapeId = tool->activationShapeId();

    switchTool(t, false);
}

void KoToolManager::switchTool(const QString &id, bool temporary) {
    if (d->canvasData->activeTool && temporary)
        d->canvasData->stack.push(d->canvasData->activeToolId);
    d->canvasData->activeToolId = id;
    KoTool *tool = d->canvasData->allTools.value(id);
    if (! tool) {
        kWarning(30006) << "KoToolManager::switchTool() " << (temporary?"temporary":"") << " got request to unknown tool: '" << id << "'\n";
        return;
    }

    foreach(ToolHelper *th, d->tools) {
        if(th->id() == id) {
            d->canvasData->activationShapeId = th->activationShapeId();
            break;
        }
    }

    switchTool(tool, temporary);
}

void KoToolManager::switchTool(KoTool *tool, bool temporary) {
    Q_ASSERT(tool);
    if (d->canvasData == 0)
        return;

    bool newActiveTool = d->canvasData->activeTool != 0;

    if (newActiveTool) {
        d->canvasData->activeTool->repaintDecorations();
        // check if this tool is inputDeviceAgnostic and used by other devices, in which case we should not deactivate.
        QList<CanvasData*> items = d->canvasses[d->canvasData->canvas];
        foreach(CanvasData *cd, items) {
            if(cd == d->canvasData) continue;
            if(cd->activeTool == d->canvasData->activeTool) {
                newActiveTool = false;
                break;
            }
        }
    }

    if(newActiveTool) {
        foreach(QAction *action, d->canvasData->activeTool->actions().values())
            action->setEnabled(false);
        // repaint the decorations before we deactivate the tool as it might deleted
        // data needed for the repaint
        d->canvasData->activeTool->deactivate();
        disconnect(d->canvasData->activeTool, SIGNAL(cursorChanged(QCursor)),
                this, SLOT(updateCursor(QCursor)));
        disconnect(d->canvasData->activeTool, SIGNAL(activateTool(const QString &)),
                this, SLOT(switchToolRequested(const QString &)));
        disconnect(d->canvasData->activeTool, SIGNAL(activateTemporary(const QString &)),
                this, SLOT(switchToolTemporaryRequested(const QString &)));
        disconnect(d->canvasData->activeTool, SIGNAL(done()), this, SLOT(switchBackRequested()));
    }

    d->canvasData->activeTool = tool;
    connect(d->canvasData->activeTool, SIGNAL(cursorChanged(QCursor)),
            this, SLOT(updateCursor(QCursor)));
    connect(d->canvasData->activeTool, SIGNAL(activateTool(const QString &)),
            this, SLOT(switchToolRequested(const QString &)));
    connect(d->canvasData->activeTool, SIGNAL(activateTemporary(const QString &)),
            this, SLOT(switchToolTemporaryRequested(const QString &)));
    connect(d->canvasData->activeTool, SIGNAL(done()), this, SLOT(switchBackRequested()));

    // we expect the tool to emit a cursor on activation.  This is for quick-fail :)
    d->canvasData->canvas->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    foreach(QAction *action, d->canvasData->activeTool->actions().values()) {
        action->setEnabled(true);
        d->canvasData->canvas->addAction(action);
    }
    d->canvasData->activeTool->activate(temporary);

    postSwitchTool();
}

void KoToolManager::postSwitchTool() {
#ifndef NDEBUG
    int canvasCount = 1;
    foreach(QList<CanvasData*> list, d->canvasses) {
        bool first = true;
        foreach(CanvasData *data, list) {
            if(first)
                kDebug(30006) <<"Canvas" << canvasCount++;
            kDebug(30006) <<"  +- Tool:" << data->activeToolId  << (data == d->canvasData?" *":"");
            first = false;
        }
    }
#endif
    if(d->canvasData->canvas->canvas()) {
        KoCanvasBase *canvas = d->canvasData->canvas->canvas();
        KoToolProxy *tp = d->proxies.value(canvas);
        if(tp)
            tp->setActiveTool(d->canvasData->activeTool);
        canvas->updateInputMethodInfo();
    }

    QMap<QString, QWidget *> optionWidgetMap = d->canvasData->activeTool->optionWidgets();
    if(optionWidgetMap.empty()) { // no option widget.
        QWidget *toolWidget;
        QString name;
        foreach( ToolHelper * tool, d->tools ) {
            if ( tool->id() == d->canvasData->activeTool->toolId() ) {
                name = tool->name();
                break;
            }
        }
        toolWidget = d->canvasData->dummyToolWidget;
        if(toolWidget == 0) {
            toolWidget = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(toolWidget);
            layout->setMargin(3);
            d->canvasData->dummyToolLabel = new QLabel(toolWidget);
            layout->addWidget(d->canvasData->dummyToolLabel);
            layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
            toolWidget->setLayout(layout);
            d->canvasData->dummyToolWidget = toolWidget;
        }
        d->canvasData->dummyToolLabel->setText(i18n("Active tool: %1", name));
        optionWidgetMap.insert(i18n("Tool Options"), toolWidget);
    }
    d->canvasData->canvas->setToolOptionWidgets(optionWidgetMap);
    emit changedTool(d->canvasData->canvas, d->uniqueToolIds.value(d->canvasData->activeTool));
}


void KoToolManager::attachCanvas(KoCanvasController *controller) {
    CanvasData *cd = d->createCanvasData(controller, KoInputDevice::mouse());
    // switch to new canvas as the active one.
    d->canvasData = cd;
    d->inputDevice = cd->inputDevice;
    QList<CanvasData*> canvasses;
    canvasses.append(cd);
    d->canvasses[controller] = canvasses;

    KoToolProxy *tp = d->proxies[controller->canvas()];
    if(tp)
        tp->setCanvasController(controller);

    if (cd->activeTool == 0)
    {
        // no active tool, so we activate the highest priority main tool
        int highestPriority = INT_MAX;
        ToolHelper * helper = 0;
        foreach( ToolHelper * th, d->tools )
        {
            if( th->toolType() == KoToolFactory::mainToolType() )
            {
                highestPriority = qMin( highestPriority, th->priority() );
                helper = th;
            }
        }
        if( helper )
            toolActivated( helper );
    }

    Connector *connector = new Connector(controller->canvas()->shapeManager());
    connect(connector, SIGNAL(selectionChanged(QList<KoShape*>)), this,
            SLOT(selectionChanged(QList<KoShape*>)));

    d->canvasData->canvas->activate();
    emit changedCanvas( d->canvasData ? d->canvasData->canvas->canvas() : 0 );
}

void KoToolManager::movedFocus(QWidget *from, QWidget *to) {
    Q_UNUSED(from);
    if (to == 0 || d->canvasData && to == d->canvasData->canvas)
        return;

    KoCanvasController *newCanvas = 0;
    // if the 'to' is one of our canvasses, or one of its children, then switch.
    foreach(KoCanvasController* canvas, d->canvasses.keys()) {
        if (canvas == to || canvas->canvas()->canvasWidget() == to) {
            newCanvas = canvas;
            break;
        }
    }

    if (newCanvas == 0)
        return;
    if (d->canvasData && newCanvas == d->canvasData->canvas)
        return;

    if(! d->canvasses.contains(newCanvas))
        return;
    foreach(CanvasData *data, d->canvasses.value(newCanvas)) {
        if(data->inputDevice == d->inputDevice) {
            d->canvasData = data;
            postSwitchTool();
            d->canvasData->canvas->canvas()->canvasWidget()->setCursor(d->canvasData->activeTool->cursor());
            d->canvasData->canvas->activate();
            emit changedCanvas( d->canvasData ? d->canvasData->canvas->canvas() : 0 );
            return;
        }
    }
    // no such inputDevice for this canvas...
    d->canvasData = d->canvasses.value(newCanvas).first();
    d->inputDevice = d->canvasData->inputDevice;
    d->canvasData->canvas->activate();
    emit changedCanvas( d->canvasData ? d->canvasData->canvas->canvas() : 0 );
}

void KoToolManager::detachCanvas(KoCanvasController *controller) {
    if (d->canvasData && d->canvasData->canvas == controller) {
        d->canvasData = 0; // replace with a blank one
    }

    QList<KoTool *> tools;
    foreach(CanvasData *cd, d->canvasses.value(controller)) {
        foreach(KoTool *tool, cd->allTools.values())
        if(! tools.contains(tool))
            tools.append(tool);
        delete cd;
    }
    foreach(KoTool *tool, tools) {
        d->uniqueToolIds.remove(tool);
        delete tool;
    }
    d->canvasses.remove(controller);
    emit changedCanvas( d->canvasData ? d->canvasData->canvas->canvas() : 0 );
}

void KoToolManager::updateCursor(QCursor cursor) {
    Q_ASSERT(d->canvasData);
    Q_ASSERT(d->canvasData->canvas);
    Q_ASSERT(d->canvasData->canvas->canvas());
    d->canvasData->canvas->canvas()->canvasWidget()->setCursor(cursor);
}

void KoToolManager::switchToolRequested(const QString & id) {
    while (!d->canvasData->stack.isEmpty()) // switching means to flush the stack
        d->canvasData->stack.pop();
    switchTool(id, false);
}

void KoToolManager::switchToolTemporaryRequested(const QString &id) {
    switchTool(id, true);
}

void KoToolManager::switchBackRequested() {
    if (d->canvasData->stack.isEmpty()) {
        // default to changing to the interactionTool
        switchTool(KoInteractionTool_ID, false);
        return;
    }
    switchTool(d->canvasData->stack.pop(), false);
}

KoCreateShapesTool * KoToolManager::shapeCreatorTool(KoCanvasBase *canvas) const
{
    foreach(KoCanvasController *controller, d->canvasses.keys()) {
        if (controller->canvas() == canvas) {
            KoCreateShapesTool *createTool = dynamic_cast<KoCreateShapesTool*>
                (d->canvasData->allTools.value(KoCreateShapesTool_ID));
            Q_ASSERT(createTool /* ID changed? */);
            return createTool;
        }
    }
    Q_ASSERT( 0 ); // this should not happen
    return 0;
}

KoGuidesTool * KoToolManager::guidesTool( KoCanvasBase * canvas ) const
{
    foreach(KoCanvasController *controller, d->canvasses.keys()) {
        if (controller->canvas() == canvas) {
            KoGuidesTool * guidesTool = dynamic_cast<KoGuidesTool*>
                    (d->canvasData->allTools.value(KoGuidesTool_ID));
            Q_ASSERT(guidesTool /* ID changed? */);
            return guidesTool;
        }
    }
    Q_ASSERT( 0 ); // this should not happen
    return 0;
}

void KoToolManager::selectionChanged(QList<KoShape*> shapes) {
    QList<QString> types;
    foreach(KoShape *shape, shapes) {
        if (! types.contains(shape->shapeId())) {
            types.append(shape->shapeId());
        }
    }

    // check if there is still a shape selected the active tool can work on
    // there needs to be at least one shape for a tool without an activationShapeId
    // to work
    // if not change the current tool to the default tool
    if ( ! ( d->canvasData->activationShapeId.isNull() && shapes.size() > 0 ) && d->canvasData->activationShapeId != "flake/always" &&
            ! types.contains( d->canvasData->activationShapeId ) ) {
        switchTool(KoInteractionTool_ID, false);
    }

    emit toolCodesSelected(d->canvasData->canvas, types);
}

KoCanvasController *KoToolManager::activeCanvasController() const {
    if( ! d->canvasData )
        return 0;
    return d->canvasData->canvas;
}

void KoToolManager::switchToolByShortcut(QKeyEvent *event)
{
    QKeySequence item(event->key() | ((Qt::ControlModifier | Qt::AltModifier) & event->modifiers()));

    foreach(ToolHelper *th, d->tools) {
        if (th->shortcut().contains(item)) {
            event->accept();
            switchTool(th->id(), false);
        }
    }
}

QString KoToolManager::preferredToolForSelection(const QList<KoShape*> &shapes) {
    QList<QString> types;
    foreach(KoShape *shape, shapes)
        if (! types.contains(shape->shapeId()))
            types.append(shape->shapeId());

    QString toolType = KoInteractionTool_ID;
    int prio = INT_MAX;
    foreach(ToolHelper *helper, d->tools) {
        if(helper->priority() >= prio)
            continue;
        if(helper->toolType() == KoToolFactory::mainToolType())
            continue;
        if(types.contains(helper->activationShapeId())) {
            toolType = helper->id();
            prio = helper->priority();
        }
    }
    return toolType;
}

#define MSECS_TO_IGNORE_SWITCH_TO_MOUSE_AFTER_TABLET_EVENT_RECEIVED 100

void KoToolManager::switchInputDevice(const KoInputDevice &device) {
    if (!device.isMouse()) {
        d->tabletEventTimer.start(MSECS_TO_IGNORE_SWITCH_TO_MOUSE_AFTER_TABLET_EVENT_RECEIVED);
    }
    if(d->inputDevice == device) return;
    if (device.isMouse() && d->tabletEventTimer.isActive()) {
        // Ignore switch to mouse for a short time after a tablet event
        // is received, as this is likely to be either the mouse event sent
        // to a widget that doesn't accept the tablet event, or, on X11,
        // a core event sent after the tablet event.
        return;
    }
    d->inputDevice = device;
    QList<CanvasData*> items = d->canvasses[d->canvasData->canvas];
    foreach(CanvasData *cd, items) {
        if(cd->inputDevice == device) {
            d->canvasData = cd;
            if(cd->activeTool == 0)
                switchTool(KoInteractionTool_ID, false);
            else {
                postSwitchTool();
                d->canvasData->canvas->canvas()->canvasWidget()->setCursor(d->canvasData->activeTool->cursor());
            }
            d->canvasData->canvas->activate();
            emit inputDeviceChanged(device);
            emit changedCanvas( d->canvasData ? d->canvasData->canvas->canvas() : 0 );
            return;
        }
    }

    // still here?  That means we need to create a new CanvasData instance with the current InputDevice.
    CanvasData *cd = d->createCanvasData(d->canvasData->canvas, device);
    // switch to new canvas as the active one.
    QString oldTool = d->canvasData->activeToolId;

    d->canvasData = cd;
    items.append(cd);
    d->canvasses[d->canvasData->canvas] = items;

    switchToolRequested(oldTool);
    emit inputDeviceChanged(device);
    d->canvasData->canvas->activate();
    emit changedCanvas( d->canvasData ? d->canvasData->canvas->canvas() : 0 );
}

bool KoToolManager::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
    case QEvent::TabletEnterProximity:
    case QEvent::TabletLeaveProximity:
    {
        QTabletEvent *tabletEvent = static_cast<QTabletEvent *>(event);

        KoInputDevice id(tabletEvent->device(), tabletEvent->pointerType(), tabletEvent->uniqueId());
        switchInputDevice(id);
        break;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
        switchInputDevice(KoInputDevice::mouse());
        break;
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

void KoToolManager::registerToolProxy(KoToolProxy *proxy, KoCanvasBase *canvas) {
    d->proxies.insert(canvas, proxy);
    foreach(KoCanvasController *controller, d->canvasses.keys()) {
        if(controller->canvas() == canvas) {
            proxy->setCanvasController(controller);
            break;
        }
    }
}

void KoToolManager::injectDeviceEvent( KoDeviceEvent * event )
{
    if( d->canvasData->canvas->canvas() )
    {
        if( event->type() == KoDeviceEvent::ButtonPressed )
            d->canvasData->activeTool->customPressEvent( event->pointerEvent() );
        else if( event->type() == KoDeviceEvent::ButtonReleased )
            d->canvasData->activeTool->customReleaseEvent( event->pointerEvent() );
        else if( event->type() ==  KoDeviceEvent::PositionChanged )
            d->canvasData->activeTool->customMoveEvent( event->pointerEvent() );
    }
}

//static
KoToolManager* KoToolManager::s_instance = 0;
static K3StaticDeleter<KoToolManager> staticToolManagerDeleter;

KoToolManager* KoToolManager::instance() {
    if (s_instance == 0)
        staticToolManagerDeleter.setObject(s_instance, new KoToolManager());
    return s_instance;
}

#include "KoToolManager.moc"
