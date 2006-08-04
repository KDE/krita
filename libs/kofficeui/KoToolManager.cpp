/* This file is part of the KDE project
 *  Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include "KoToolManager.h"
#include "KoToolManager_p.h"


#include "KoToolRegistry.h"
#include <KoTool.h>
#include <KoToolBox.h>
#include <KoCreateShapesToolFactory.h>
#include <KoCreateShapesTool.h>
#include <KoInteractionToolFactory.h>
#include <KoShapeControllerBase.h>
#include <KoCanvasController.h>
#include <KoShapeRegistry.h>
#include <KoID.h>

#include <kactioncollection.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include <QStringList>
#include <QAbstractButton>
#include <QApplication>

// ******** DummyTool **********
class DummyTool : public KoTool {
public:
    DummyTool() : KoTool(0) {}
    ~DummyTool() {}
    void paint( QPainter &, KoViewConverter &) {}
    void mousePressEvent( KoPointerEvent *) {}
    void mouseMoveEvent( KoPointerEvent *) {}
    void mouseReleaseEvent( KoPointerEvent *) {}
};

// ******** KoToolManager **********
KoToolManager::KoToolManager()
: QObject()
, m_activeCanvas(0)
, m_activeTool(0)
{
    m_dummyTool = new DummyTool();
    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)),
        this, SLOT(movedFocus(QWidget*,QWidget*)));
}

KoToolManager::~KoToolManager() {
    delete m_dummyTool;
}

void KoToolManager::setup() {
    if(m_tools.size() > 0)
        return;
    // add defaults
    m_tools.append( new ToolHelper(new KoCreateShapesToolFactory(this, QStringList())) );
    m_defaultTool = new ToolHelper(new KoInteractionToolFactory(this, QStringList()));
    m_tools.append(m_defaultTool);

    KoShapeRegistry::instance();
    KoToolRegistry *registry = KoToolRegistry::instance();
    foreach(KoID id, registry->listKeys()) {
        ToolHelper *t = new ToolHelper(registry->get(id));
        connect(t, SIGNAL(toolActivated(ToolHelper*)), this, SLOT(toolActivated(ToolHelper*)));
        m_tools.append(t);
    }

    // connect to all tools so we can hear their button-clicks
    foreach(ToolHelper *tool, m_tools)
        connect(tool, SIGNAL(toolActivated(ToolHelper*)), this, SLOT(toolActivated(ToolHelper*)));
}

KoToolBox *KoToolManager::toolBox(const QString &applicationName) {
    setup();
    KoToolBox *toolBox = new KoToolBox();
    foreach(ToolHelper *tool, m_tools) {
        QAbstractButton *but = tool->createButton(toolBox);
        toolBox->addButton(but, tool->toolType(), tool->priority(), tool->uniqueId());
        if(tool->toolType() == KoToolFactory::dynamicToolType())
            toolBox->setVisibilityCode(but, tool->activationShapeId());
    }
    toolBox->setup();
    toolBox->setWindowTitle(applicationName);
    toolBox->setObjectName("ToolBox_"+ applicationName);
    connect(this, SIGNAL(changedTool(int)), toolBox, SLOT(setActiveTool(int)));
    connect(this, SIGNAL(toolCodesSelected(QList<QString>)),
            toolBox, SLOT(setButtonsVisible(QList<QString>)));
    return toolBox;
}

void KoToolManager::registerTools(KActionCollection *ac) {
    setup();
    // TODO
}

void KoToolManager::addControllers(KoCanvasController *controller, KoShapeControllerBase *sc) {
    if(m_canvases.contains(controller))
        return;
    setup();
    kDebug(30004) << "KoToolManager::addControllers called, setting up..." << endl;
    m_canvases.append(controller);
    m_shapeControllers.insert(controller, sc);
    if(m_activeCanvas == 0)
        m_activeCanvas = controller;
    if(controller->canvas())
        attachCanvas(controller);
    connect(controller, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    connect(controller, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::removeCanvasController(KoCanvasController *controller) {
    m_canvases.removeAll(controller);
    m_shapeControllers.remove(controller);
    QMap<QString, KoTool*> toolsMap = m_allTools.value(controller);
    foreach(KoTool *tool, toolsMap.values()) {
        m_uniqueToolIds.remove(tool);
        delete tool;
    }
    m_allTools.remove(controller);
    if(controller->canvas())
        detachCanvas(controller);
    disconnect(controller, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    disconnect(controller, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::toolActivated(ToolHelper *tool) {
    kDebug(30004) << "ToolActivated: '" << tool->name() << "'\n";
    QMap<QString, KoTool*> toolsMap = m_allTools.value(m_activeCanvas);
    KoTool *t = toolsMap.value(tool->id());
    m_activeToolId = tool->id();
    switchTool(t);
}

void KoToolManager::switchTool(const QString &id, bool temporary) {
    Q_ASSERT(m_activeCanvas);
    if(m_activeTool && temporary)
        m_stack.push(m_activeToolId);
    m_activeToolId = id;
    QMap<QString, KoTool*> toolsMap = m_allTools.value(m_activeCanvas);
    KoTool *tool = toolsMap.value(id);
    if(! tool) {
        kWarning(30004) << "Tool requested " << (temporary?"temporary":"") << "switch to unknown tool: '" << id << "'\n";
        return;
    }
    switchTool(tool);
}

void KoToolManager::switchTool(KoTool *tool) {
    Q_ASSERT(tool);
    if(m_activeCanvas == 0) {
        return;
    }
    if(m_activeTool) {
        m_activeTool->deactivate();
        disconnect(m_activeTool, SIGNAL(sigCursorChanged(QCursor)),
                this, SLOT(updateCursor(QCursor)));
        disconnect(m_activeTool, SIGNAL(sigActivateTool(const QString &)),
                this, SLOT(switchToolRequested(const QString &)));
        disconnect(m_activeTool, SIGNAL(sigActivateTemporary(const QString &)),
                this, SLOT(switchToolTemporaryRequested(const QString &)));
        disconnect(m_activeTool, SIGNAL(sigDone()), this, SLOT(switchBackRequested()));
    }
    m_activeTool = tool;
    connect(m_activeTool, SIGNAL(sigCursorChanged(QCursor)),
            this, SLOT(updateCursor(QCursor)));
    connect(m_activeTool, SIGNAL(sigActivateTool(const QString &)),
            this, SLOT(switchToolRequested(const QString &)));
    connect(m_activeTool, SIGNAL(sigActivateTemporary(const QString &)),
            this, SLOT(switchToolTemporaryRequested(const QString &)));
    connect(m_activeTool, SIGNAL(sigDone()), this, SLOT(switchBackRequested()));

    // and set it.
    foreach(KoCanvasController *controller, m_canvases) {
        if(!controller->canvas())
            continue;
        controller->canvas()->setTool(controller==m_activeCanvas ? m_activeTool : m_dummyTool);
        // we expect the tool to emit a cursur on activation.  This is for quick-fail :)
        controller->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }
    m_activeTool->activate();
    emit changedTool(m_uniqueToolIds.value(m_activeTool));
}

void KoToolManager::attachCanvas(KoCanvasController *controller) {
    QMap<QString, KoTool*> toolsMap;
    foreach(ToolHelper *tool, m_tools) {
        KoTool *tl = tool->createTool(controller->canvas());
        m_uniqueToolIds.insert(tl, tool->uniqueId());
        toolsMap.insert(tool->id(), tl);
    }
    KoCreateShapesTool *createTool = dynamic_cast<KoCreateShapesTool*>(toolsMap.value(KoCreateShapesTool_ID));
    Q_ASSERT(createTool);
    createTool->setShapeController(m_shapeControllers[controller]);
    foreach(QString id, KoShapeRegistry::instance()->keys()) {
        createTool->setShapeId(id);
        break;
    }

    m_allTools.remove(controller);
    m_allTools.insert(controller, toolsMap);

    if(m_activeTool == 0)
        toolActivated(m_defaultTool);
    else {
        controller->canvas()->setTool(m_dummyTool);
        controller->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }

    Connector *connector = new Connector(controller->canvas()->shapeManager());
    connect(connector, SIGNAL(selectionChanged(QList<KoShape*>)), this,
        SLOT(selectionChanged(QList<KoShape*>)));
}

void KoToolManager::movedFocus(QWidget *from, QWidget *to) {
    Q_UNUSED(from);
    if(to == 0 || to == m_activeCanvas)
        return;
    QWidget *newMainWindow = to;
    while(newMainWindow->parentWidget())
        newMainWindow = newMainWindow->parentWidget();

    KoCanvasController *newCanvas = 0;
    // if the 'to' is one of our canvasses, or one of its children, then switch.
    foreach(KoCanvasController* canvas, m_canvases) {
        if(canvas == m_activeCanvas)
            continue;
        // make sure we are not just talking about overlapping windows.
        QWidget *root = canvas;
        while(root->parentWidget())
            root = root->parentWidget();
        if(newMainWindow == root) {
            // this canvas is the new 'To'
            newCanvas = canvas;
            break;
        }
    }
    if(newCanvas == 0)
        return;
    if(newCanvas == m_activeCanvas)
        return;
    if(m_activeCanvas) {
        m_activeCanvas->canvas()->setTool(m_dummyTool);
        m_activeCanvas->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }
    m_activeCanvas = newCanvas;

    switchTool(m_activeToolId, false);
}

void KoToolManager::detachCanvas(KoCanvasController *controller) {
    // TODO detach
    if(m_activeCanvas == controller)
        m_activeCanvas = 0;
    m_activeTool = 0;
    QMap<QString, KoTool*> toolsMap = m_allTools.value(controller);
    foreach(KoTool *tool, toolsMap.values())
        delete tool;
    toolsMap.clear();
    controller->canvas()->setTool(m_dummyTool);
}

void KoToolManager::updateCursor(QCursor cursor) {
    Q_ASSERT(m_activeCanvas);
    Q_ASSERT(m_activeCanvas->canvas());
    m_activeCanvas->canvas()->canvasWidget()->setCursor(cursor);
}

void KoToolManager::switchToolRequested(const QString & id) {
    while (!m_stack.isEmpty()) // switching means to flush the stack
        m_stack.pop();
    switchTool(id, false);
}

void KoToolManager::switchToolTemporaryRequested(const QString &id) {
    switchTool(id, true);
}

void KoToolManager::switchBackRequested() {
    if(m_stack.isEmpty()) {
        // default to changing to the interactionTool
        switchTool(KoInteractionTool_ID, false);
        return;
    }
    switchTool(m_stack.pop(), false);
}

KoCreateShapesTool *KoToolManager::shapeCreatorTool(KoCanvasBase *canvas) const {
    foreach(KoCanvasController *controller, m_canvases) {
        if(controller->canvas() == canvas) {
            QMap<QString, KoTool*> tools = m_allTools.value(controller);
            KoCreateShapesTool *tool =
                dynamic_cast<KoCreateShapesTool*>(tools.value(KoCreateShapesTool_ID));
            Q_ASSERT(tool /* ID changed? */);
            return tool;
        }
    }
    kWarning(30004) << "KoToolManager: can't find the canvas, did you register it?" << endl;
    return 0;
}

void KoToolManager::selectionChanged(QList<KoShape*> shapes) {
    kDebug() << "selection changed, now " << shapes.count() << " shapes selected\n";
    QList<QString> types;
    foreach(KoShape *shape, shapes) {
       if(! types.contains(shape->shapeId()))
            types.append(shape->shapeId());
    }
    emit toolCodesSelected(types);
}

//static
KoToolManager* KoToolManager::s_instance = 0;
static KStaticDeleter<KoToolManager> staticToolManagerDeleter;

KoToolManager* KoToolManager::instance() {
    if(s_instance == 0)
        staticToolManagerDeleter.setObject(s_instance, new KoToolManager());
    return s_instance;
}

#include "KoToolManager.moc"
