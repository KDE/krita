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
//#include "KoToolBox.h"
#include "KoToolRegistry.h"

#include <KoToolFactory.h>
#include <KoCreateShapesToolFactory.h>
#include <KoCreateShapesTool.h>
#include <KoInteractionToolFactory.h>
#include <KoShapeControllerBase.h>
#include <KoCanvasController.h>
#include <KoShapeRegistry.h>

#include <kactioncollection.h>
#include <kdebug.h>
#include <kstaticdeleter.h>
#include <kicon.h>

#include <QToolButton>
#include <QButtonGroup>
#include <QVBoxLayout>

#include <stdlib.h> // for random()

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
, m_mutex(QMutex::Recursive)
{
    m_dummyTool = new DummyTool();
}

KoToolManager::~KoToolManager() {
    delete m_dummyTool;
}

void KoToolManager::setup() {
    m_mutex.lock();
    if(m_tools.size() > 0) {
        m_mutex.unlock();
        return;
    }
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

    // do some magic for the sorting here :)  TODO
    m_mutex.unlock();
}

KoToolBox *KoToolManager::toolBox() {
    setup();
    KoToolBox *toolBox = new KoToolBox();
    foreach(ToolHelper *tool, m_tools) {
        QAbstractButton *but = tool->createButton(toolBox);
        toolBox->addButton(but, tool->toolType(), tool->priority(), tool->uniqueId());
    }
    toolBox->setup();
    connect(this, SIGNAL(changedTool(int)), toolBox, SLOT(setActiveTool(int)));
    return toolBox;
}

void KoToolManager::registerTools(KActionCollection *ac) {
    m_mutex.lock();
    setup();
    // TODO
    m_mutex.unlock();
}

void KoToolManager::addControllers(KoCanvasController *controller, KoShapeControllerBase *sc) {
    if(m_canvases.contains(controller))
        return;
    setup();
    m_mutex.lock();
    kDebug(30004) << "KoToolManager::addControllers called, setting up..." << endl;
    m_canvases.append(controller);
    m_shapeControllers.insert(controller, sc);
    if(m_activeCanvas == 0)
        m_activeCanvas = controller;
    if(controller->canvas())
        attachCanvas(controller);
    connect(controller, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    connect(controller, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
    m_mutex.unlock();
}

void KoToolManager::removeCanvasController(KoCanvasController *controller) {
    m_mutex.lock();
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
    m_mutex.unlock();
}

void KoToolManager::toolActivated(ToolHelper *tool) {
    kDebug(30004) << "ToolActivated: '" << tool->name() << "'\n";
    QMap<QString, KoTool*> toolsMap = m_allTools.value(m_activeCanvas);
    KoTool *t = toolsMap.value(tool->id());
    switchTool(t, false);
}

void KoToolManager::switchTool(const QString &id, bool temporary) {
    Q_ASSERT(m_activeCanvas);
    QMap<QString, KoTool*> toolsMap = m_allTools.value(m_activeCanvas);
    KoTool *tool = toolsMap.value(id);
    if(! tool) {
        kWarning(30004) << "Tool requested " << (temporary?"temporary":"") << "switch to unknown tool: '" << id << "'\n";
        return;
    }
    switchTool(tool, temporary);
}

void KoToolManager::switchTool(KoTool *tool, bool temporary) {
    m_mutex.lock();
    Q_ASSERT(tool);
    if(m_activeCanvas == 0) {
        m_mutex.unlock();
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
    if(m_activeTool && temporary)
        m_stack.push(m_activeTool);
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
    m_mutex.unlock();
}

void KoToolManager::attachCanvas(KoCanvasController *controller) {
    // TODO listen to focus changes
    // TODO listen to selection changes
    QMap<QString, KoTool*> toolsMap;
    foreach(ToolHelper *tool, m_tools) {
        KoTool *tl = tool->createTool(controller->canvas());
        m_uniqueToolIds.insert(tl, tool->uniqueId());
        toolsMap.insert(tool->id(), tl);
    }
    KoCreateShapesTool *createTool = dynamic_cast<KoCreateShapesTool*>(toolsMap.value(KoCreateShapesTool_ID));
    Q_ASSERT(createTool);
    createTool->setShapeController(m_shapeControllers[controller]);

    m_mutex.lock();
    m_allTools.remove(controller);
    m_allTools.insert(controller, toolsMap);
    m_mutex.unlock();

    if(m_activeTool == 0)
        toolActivated(m_defaultTool);
    else {
        controller->canvas()->setTool(m_dummyTool);
        controller->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }
}

void KoToolManager::detachCanvas(KoCanvasController *controller) {
    // TODO detach
    if(m_activeCanvas == controller)
        m_activeCanvas = 0;
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

//   ************ ToolHelper **********
ToolHelper::ToolHelper(KoToolFactory *tool) {
    m_toolFactory = tool;
    m_uniqueId = (int) random();
}

QAbstractButton* ToolHelper::createButton(QWidget *parent) {
    QToolButton *but = new QToolButton(parent);
    but->setIcon(KIcon( m_toolFactory->icon() ).pixmap(22));
    but->setToolTip(m_toolFactory->toolTip());
    connect(but, SIGNAL(clicked()), this, SLOT(buttonPressed()));
    return but;
}

void ToolHelper::buttonPressed() {
    emit toolActivated(this);
}

const QString &ToolHelper::id() const {
    return m_toolFactory->toolId();
}

const QString& ToolHelper::name() const {
    return m_toolFactory->name();
}

KoTool *ToolHelper::createTool(KoCanvasBase *canvas) const {
    return m_toolFactory->createTool(canvas);
}

const QString &ToolHelper::toolType() const {
    return m_toolFactory->toolType();
}

int ToolHelper::priority() const {
    return m_toolFactory->priority();
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
