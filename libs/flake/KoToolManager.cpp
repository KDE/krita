/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoCreatePathToolFactory.h"
#include "KoCreateShapesToolFactory.h"
#include "KoCreateShapesTool.h"
#include "KoInteractionToolFactory.h"
#include "KoCanvasController.h"
#include "KoShapeRegistry.h"
#include "KoShapeManager.h"

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
#include <kactioncollection.h>
#include <kdebug.h>
#include <kstaticdeleter.h>
#include <kaction.h>
#include <QStack>
#include <QLabel>

/*
 TODO
    * We now have one d->activeTool, we should allow a different active tool per canvas.
    * We now have one set of all tools per canvas, but this should be per canvas, per input device.
*/

class KoToolManager::Private {
public:
    Private() : activeCanvas(0), activeTool(0), defaultTool(0) {}

    QList<ToolHelper*> tools;
    QList<KoCanvasController*> canvases;
    KoCanvasController *activeCanvas;
    KoTool *activeTool;
    ToolHelper *defaultTool; // the pointer thingy
    QString activeToolId;
    QString activationShapeId;

    QHash<KoTool*, int> uniqueToolIds; // for the changedTool signal
    QHash<KoCanvasController*, QHash<QString, KoTool*> > allTools;
    QHash<KoCanvasBase*, KoToolProxy*> proxies;
    QStack<QString> stack; // stack of temporary tools
};

// ******** KoToolManager **********
KoToolManager::KoToolManager()
    : QObject(),
    d(new Private())
{
    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(movedFocus(QWidget*,QWidget*)));
}

KoToolManager::~KoToolManager() {
    delete d;
}

void KoToolManager::setup() {
    if (d->tools.size() > 0)
        return;

    d->tools.append( new ToolHelper(new KoCreatePathToolFactory(this, QStringList())) );
    d->tools.append( new ToolHelper(new KoCreateShapesToolFactory(this, QStringList())) );
    d->defaultTool = new ToolHelper(new KoInteractionToolFactory(this, QStringList()));
    d->tools.append(d->defaultTool);

    KoShapeRegistry::instance();
    KoToolRegistry *registry = KoToolRegistry::instance();
    foreach(QString id, registry->keys()) {
        ToolHelper *t = new ToolHelper(registry->get(id));
        d->tools.append(t);
    }

    // connect to all tools so we can hear their button-clicks
    foreach(ToolHelper *tool, d->tools)
        connect(tool, SIGNAL(toolActivated(ToolHelper*)), this, SLOT(toolActivated(ToolHelper*)));
}

QList<KoToolManager::Button> KoToolManager::createToolList() const {
    QList<KoToolManager::Button> answer;
    foreach(ToolHelper *tool, d->tools) {
        if(tool->id() == KoCreateShapesTool_ID)
            continue; // don't show this one.
        Button button;
        button.button = tool->createButton();
        button.section = tool->toolType();
        button.priority = tool->priority();
        button.buttonGroupId = tool->uniqueId();
        if (tool->toolType() == KoToolFactory::dynamicToolType())
            button.visibilityCode = tool->activationShapeId();
        answer.append(button);
    }
    return answer;
}

void KoToolManager::registerTools(KActionCollection *ac, KoCanvasController *controller) {
    setup();

    class ToolSwitchAction : public KAction {
      public:
        ToolSwitchAction(KActionCollection *parent, const QString &toolId) : KAction(parent) {
            m_toolId = toolId;
        }
      private:
        void slotTriggered() {
            KoToolManager::instance()->switchToolRequested(m_toolId);
        }
        QString m_toolId;
    };

    foreach(ToolHelper *th, d->tools) {
        ToolSwitchAction *tsa = new ToolSwitchAction(ac, th->id());
        ac->addAction("tool_"+ th->name(), tsa);
        tsa->setShortcut(th->shortcut());
        tsa->setText(i18n("Activate %1", th->name()));
    }

    QHash<QString, KoTool*> toolsHash = d->allTools.value(controller);
    foreach(KoTool *tool, toolsHash.values()) {
        QHash<QString, QAction*> actions = tool->actions();
        foreach(QString name, actions.keys())
            ac->addAction(name, actions[name]);
    }
}

void KoToolManager::addController(KoCanvasController *controller ) {
    if (d->canvases.contains(controller))
        return;
    setup();
    d->canvases.append(controller);
    if (d->activeCanvas == 0)
        d->activeCanvas = controller;
    if (controller->canvas())
        attachCanvas(controller);
    connect(controller, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    connect(controller, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::removeCanvasController(KoCanvasController *controller) {
    d->canvases.removeAll(controller);
    QHash<QString, KoTool*> toolsHash = d->allTools.value(controller);
    foreach(KoTool *tool, toolsHash.values()) {
        d->uniqueToolIds.remove(tool);
        delete tool;
    }
    d->allTools.remove(controller);
    if (controller->canvas()) {
        detachCanvas(controller);
        d->proxies.remove(controller->canvas());
    }
    disconnect(controller, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    disconnect(controller, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::toolActivated(ToolHelper *tool) {
    Q_ASSERT(d->activeCanvas);
    QHash<QString, KoTool*> toolsHash = d->allTools.value(d->activeCanvas);
    KoTool *t = toolsHash.value(tool->id());

    d->activeToolId = tool->id();
    d->activationShapeId = tool->activationShapeId();

    switchTool(t);
}

void KoToolManager::switchTool(const QString &id, bool temporary) {

    if (!d->activeCanvas) kDebug(30004) << kBacktrace();
    Q_ASSERT(d->activeCanvas);
    if (d->activeTool && temporary)
        d->stack.push(d->activeToolId);
    d->activeToolId = id;
    QHash<QString, KoTool*> toolsHash = d->allTools.value(d->activeCanvas);
    KoTool *tool = toolsHash.value(id);
    if (! tool) {
        kWarning(30004) << "Tool requested " << (temporary?"temporary":"") << "switch to unknown tool: '" << id << "'\n";
        return;
    }
    switchTool(tool);
}

void KoToolManager::switchTool(KoTool *tool) {
    Q_ASSERT(tool);
    if (d->activeCanvas == 0) {
        return;
    }
    if (d->activeTool) {
        foreach(QAction *action, d->activeTool->actions().values())
            action->setEnabled(false);
        d->activeTool->deactivate();
        disconnect(d->activeTool, SIGNAL(sigCursorChanged(QCursor)),
                this, SLOT(updateCursor(QCursor)));
        disconnect(d->activeTool, SIGNAL(sigActivateTool(const QString &)),
                this, SLOT(switchToolRequested(const QString &)));
        disconnect(d->activeTool, SIGNAL(sigActivateTemporary(const QString &)),
                this, SLOT(switchToolTemporaryRequested(const QString &)));
        disconnect(d->activeTool, SIGNAL(sigDone()), this, SLOT(switchBackRequested()));
    }
    d->activeTool = tool;
    connect(d->activeTool, SIGNAL(sigCursorChanged(QCursor)),
            this, SLOT(updateCursor(QCursor)));
    connect(d->activeTool, SIGNAL(sigActivateTool(const QString &)),
            this, SLOT(switchToolRequested(const QString &)));
    connect(d->activeTool, SIGNAL(sigActivateTemporary(const QString &)),
            this, SLOT(switchToolTemporaryRequested(const QString &)));
    connect(d->activeTool, SIGNAL(sigDone()), this, SLOT(switchBackRequested()));

    // and set it.
    foreach(KoCanvasController *controller, d->canvases) {
        if (!controller->canvas())
            continue;
        // we expect the tool to emit a cursor on activation.  This is for quick-fail :)
        controller->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }
    foreach(QAction *action, d->activeTool->actions().values())
        action->setEnabled(true);
    d->activeTool->activate();

    if(d->activeCanvas->canvas()) {
        KoToolProxy *tp = d->proxies.value(d->activeCanvas->canvas());
        if(tp)
            tp->setActiveTool(d->activeTool);
    }

    QWidget *toolWidget = d->activeTool->optionWidget();
    if(toolWidget == 0) { // no option widget.
        QString name;
        foreach( ToolHelper * tool, d->tools ) {
            if ( tool->id() == d->activeTool->toolId() ) {
                name = tool->name();
                break;
            }
        }
        toolWidget = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(toolWidget);
        layout->setMargin(3);
        QLabel *label = new QLabel(i18n("Active tool: %1", name), toolWidget);
        layout->addWidget(label);
        layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
        toolWidget->setLayout(layout);
    }
    d->activeCanvas->setToolOptionWidget(toolWidget);
    emit changedTool(d->uniqueToolIds.value(d->activeTool));
}

void KoToolManager::attachCanvas(KoCanvasController *controller) {
    //detachCanvas(controller); // so we don't end up with a lot of unused instances
    QHash<QString, KoTool*> toolsHash;
    foreach(ToolHelper *tool, d->tools) {
        kDebug(30004) << "Creating tool " << tool->id() << ", " << tool->activationShapeId() << endl;
        KoTool *tl = tool->createTool(controller->canvas());
        d->uniqueToolIds.insert(tl, tool->uniqueId());
        toolsHash.insert(tool->id(), tl);
        tl->setObjectName(tool->id());
        foreach(QAction *action, tl->actions().values())
            action->setEnabled(false);
    }
    KoCreateShapesTool *createTool = dynamic_cast<KoCreateShapesTool*>(toolsHash.value(KoCreateShapesTool_ID));
    Q_ASSERT(createTool);
    QString id = KoShapeRegistry::instance()->keys()[0];
    createTool->setShapeId(id);

    d->allTools.insert(controller, toolsHash);

    if (d->activeTool == 0) {
        toolActivated(d->defaultTool);
    }
    else {
        controller->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }

    Connector *connector = new Connector(controller->canvas()->shapeManager());
    connect(connector, SIGNAL(selectionChanged(QList<KoShape*>)), this,
            SLOT(selectionChanged(QList<KoShape*>)));
}

void KoToolManager::movedFocus(QWidget *from, QWidget *to) {
    Q_UNUSED(from);
    if (to == 0 || to == d->activeCanvas)
        return;

    KoCanvasController *newCanvas = 0;
    // if the 'to' is one of our canvasses, or one of its children, then switch.
    foreach(KoCanvasController* canvas, d->canvases) {
        if (canvas == to || canvas->canvas()->canvasWidget() == to) {
            newCanvas = canvas;
            break;
        }
    }

    if (newCanvas == 0)
        return;
    if (newCanvas == d->activeCanvas)
        return;
    if (d->activeCanvas) {
        d->activeCanvas->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }
    d->activeCanvas = newCanvas;

    switchTool(d->activeToolId, false);
    selectionChanged(d->activeCanvas->canvas()->shapeManager()->selection()->selectedShapes());
}

void KoToolManager::detachCanvas(KoCanvasController *controller) {
    if(controller == 0)
        return;
    if (d->activeCanvas == controller)
        d->activeCanvas = 0;
    d->activeTool = 0;
    QHash<QString, KoTool*> toolsHash = d->allTools.value(controller);
    qDeleteAll(toolsHash);
    d->allTools.remove(controller);
}

void KoToolManager::updateCursor(QCursor cursor) {
    Q_ASSERT(d->activeCanvas);
    Q_ASSERT(d->activeCanvas->canvas());
    d->activeCanvas->canvas()->canvasWidget()->setCursor(cursor);
}

void KoToolManager::switchToolRequested(const QString & id) {
    while (!d->stack.isEmpty()) // switching means to flush the stack
        d->stack.pop();
    switchTool(id, false);
}

void KoToolManager::switchToolTemporaryRequested(const QString &id) {
    switchTool(id, true);
}

void KoToolManager::switchBackRequested() {
    if (d->stack.isEmpty()) {
        // default to changing to the interactionTool
        switchTool(KoInteractionTool_ID, false);
        return;
    }
    switchTool(d->stack.pop(), false);
}

KoCreateShapesTool * KoToolManager::shapeCreatorTool(KoCanvasBase *canvas) const
{
    foreach(KoCanvasController *controller, d->canvases) {
        if (controller->canvas() == canvas) {
            QHash<QString, KoTool*> tools = d->allTools.value(controller);

            KoCreateShapesTool *createTool = dynamic_cast<KoCreateShapesTool*>(tools.value(KoCreateShapesTool_ID));
            Q_ASSERT(createTool /* ID changed? */);
            return createTool;
        }
    }
    Q_ASSERT( 0 ); // this should not happen
    return 0;
}

KoInputDevice KoToolManager::currentInputDevice() const
{
    return KoInputDevice::mouse();
}

void KoToolManager::selectionChanged(QList<KoShape*> shapes) {
    QList<QString> types;
    foreach(KoShape *shape, shapes) {
        if (! types.contains(shape->shapeId())) {
            types.append(shape->shapeId());
        }
    }

    // check if there is still a shape selected the active tool can work on
    // if not change the current tool to the default tool
    if ( ! d->activationShapeId.isNull() && ! types.contains( d->activationShapeId ) )
    {
        switchTool(KoInteractionTool_ID, false);
    }

    emit toolCodesSelected(types);
}

KoCanvasController *KoToolManager::activeCanvasController() const {
    return d->activeCanvas;
}

KoToolProxy *KoToolManager::createToolProxy(KoCanvasBase *parentCanvas) {
    KoToolProxy *tp = new KoToolProxy(parentCanvas);
    d->proxies.insert(parentCanvas, tp);
    return tp;
}

QString KoToolManager::preferredToolForSelection(const QList<KoShape*> &shapes) {
    QList<QString> types;
    foreach(KoShape *shape, shapes)
        if (! types.contains(shape->shapeId()))
            types.append(shape->shapeId());

    QString toolType = KoInteractionTool_ID;
    int prio = INT_MIN;
    foreach(ToolHelper *helper, d->tools) {
        if(helper->priority() <= prio)
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

//static
KoToolManager* KoToolManager::s_instance = 0;
static KStaticDeleter<KoToolManager> staticToolManagerDeleter;

KoToolManager* KoToolManager::instance() {
    if (s_instance == 0)
        staticToolManagerDeleter.setObject(s_instance, new KoToolManager());
    return s_instance;
}

#include "KoToolManager.moc"
