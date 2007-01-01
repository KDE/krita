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
// local lib
#include "KoToolManager.h"
#include "KoToolManager_p.h"
#include "KoToolRegistry.h"
#include "KoToolDocker.h"
#include "ToolProxy_p.h"

// koffice
#include <KoToolBox.h>
#include <KoCreatePathToolFactory.h>
#include <KoCreateShapesToolFactory.h>
#include <KoCreateShapesTool.h>
#include <KoInteractionToolFactory.h>
#include <KoCanvasController.h>
#include <KoShapeRegistry.h>
#include <KoShapeManager.h>

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

/*
 TODO
    * We now have one m_activeTool, we should allow a different active tool per canvas.
    * We now have one set of all tools per canvas, but this should be per canvas, per input device.
*/

// ******** KoToolManager **********
KoToolManager::KoToolManager()
    : QObject()
    , m_activeCanvas(0)
    , m_activeTool(0)
{
    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(movedFocus(QWidget*,QWidget*)));
}

KoToolManager::~KoToolManager() {
}

void KoToolManager::setup() {
    if (m_tools.size() > 0)
        return;

    // add defaults. XXX: Make these settable? We have not yet solved
    // the problem of application-specific versus generic flake tools,
    // have we?
    m_tools.append( new ToolHelper(new KoCreatePathToolFactory(this, QStringList())) );
    m_tools.append( new ToolHelper(new KoCreateShapesToolFactory(this, QStringList())) );
    m_defaultTool = new ToolHelper(new KoInteractionToolFactory(this, QStringList()));
    m_tools.append(m_defaultTool);

    KoShapeRegistry::instance();
    KoToolRegistry *registry = KoToolRegistry::instance();
    foreach(QString id, registry->keys()) {
        ToolHelper *t = new ToolHelper(registry->get(id));
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
        if(tool->id() == KoCreateShapesTool_ID)
            continue; // don't show this one.
        QAbstractButton *but = tool->createButton();
        toolBox->addButton(but, tool->toolType(), tool->priority(), tool->uniqueId());
        if (tool->toolType() == KoToolFactory::dynamicToolType())
            toolBox->setVisibilityCode(but, tool->activationShapeId());
    }

    toolBox->setup();
    toolBox->setWindowTitle(applicationName);
    toolBox->setObjectName("ToolBox_"+ applicationName);
    connect(this, SIGNAL(changedTool(int)), toolBox, SLOT(setActiveTool(int)));
    connect(this, SIGNAL(toolCodesSelected(QList<QString>)),
            toolBox, SLOT(setButtonsVisible(QList<QString>)));
    QList<QString> empty;
    toolBox->setButtonsVisible(empty);
    toolBox->setActiveTool(m_defaultTool->uniqueId());
    return toolBox;
}

void KoToolManager::registerTools(KActionCollection *ac) {
    Q_UNUSED(ac);
    setup();

    class ToolSwitchAction : public KAction {
      public:
        ToolSwitchAction(KActionCollection *parent, const QString &name, const QString &toolId) : KAction(parent, name) {
            m_toolId = toolId;
        }
      private:
        void slotTriggered() {
            KoToolManager::instance()->switchToolRequested(m_toolId);
        }
        QString m_toolId;
    };

    foreach(ToolHelper *th, m_tools) {
        ToolSwitchAction *tsa = new ToolSwitchAction(ac, "tools"+ th->name(), th->id());
        //tsa->setShortcut(th->shortcut()); // this crashes currently, lets try tomorrow with the kdelibs updates
        tsa->setText(i18n("Activate %1", th->name()));
    }
}

void KoToolManager::addControllers(KoCanvasController *controller ) {
    if (m_canvases.contains(controller))
        return;
    setup();
    m_canvases.append(controller);
    if (m_activeCanvas == 0)
        m_activeCanvas = controller;
    if (controller->canvas())
        attachCanvas(controller);
    connect(controller, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    connect(controller, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::removeCanvasController(KoCanvasController *controller) {
    m_canvases.removeAll(controller);
    QHash<QString, KoTool*> toolsHash = m_allTools.value(controller);
    foreach(KoTool *tool, toolsHash.values()) {
        m_uniqueToolIds.remove(tool);
        delete tool;
    }
    m_allTools.remove(controller);
    if (controller->canvas()) {
        detachCanvas(controller);
        m_proxies.remove(controller->canvas());
    }
    disconnect(controller, SIGNAL(canvasRemoved(KoCanvasController*)), this, SLOT(detachCanvas(KoCanvasController*)));
    disconnect(controller, SIGNAL(canvasSet(KoCanvasController*)), this, SLOT(attachCanvas(KoCanvasController*)));
}

void KoToolManager::toolActivated(ToolHelper *tool) {

    QHash<QString, KoTool*> toolsHash = m_allTools.value(m_activeCanvas);
    KoTool *t = toolsHash.value(tool->id());

    m_activeToolId = tool->id();

    // cache all the selected shapes relevant to the activated tool
    if ( m_activeToolId != KoInteractionTool_ID ) {
        m_lastSelectedShapes.clear();
        KoSelection *selection = m_activeCanvas->canvas()->shapeManager()->selection();
        foreach( KoShape *shape, selection->selectedShapes() ) {
            if ( tool->activationShapeId().isNull() || tool->activationShapeId() == shape->shapeId() )
                m_lastSelectedShapes.append( shape );
        }
    }

    switchTool(t);
}

void KoToolManager::switchTool(const QString &id, bool temporary) {

    if (!m_activeCanvas) kDebug(30004) << kBacktrace();
    Q_ASSERT(m_activeCanvas);
    if (m_activeTool && temporary)
        m_stack.push(m_activeToolId);
    m_activeToolId = id;
    QHash<QString, KoTool*> toolsHash = m_allTools.value(m_activeCanvas);
    KoTool *tool = toolsHash.value(id);
    if (! tool) {
        kWarning(30004) << "Tool requested " << (temporary?"temporary":"") << "switch to unknown tool: '" << id << "'\n";
        return;
    }
    switchTool(tool);
}

void KoToolManager::switchTool(KoTool *tool) {
    Q_ASSERT(tool);
    if (m_activeCanvas == 0) {
        return;
    }
    if (m_activeTool) {
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
        if (!controller->canvas())
            continue;
        // we expect the tool to emit a cursor on activation.  This is for quick-fail :)
        controller->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }
    m_activeTool->activate();

    if(m_activeCanvas->canvas()) {
        ToolProxy *tp = m_proxies.value(m_activeCanvas->canvas());
        if(tp)
            tp->setActiveTool(m_activeTool);
    }

    if (m_activeCanvas->toolOptionDocker()) {
// XXX: Commented out until the tool knows what for tool it is.
//        if (m_activeTool->optionWidget()) {
            m_activeCanvas->toolOptionDocker()->setOptionWidget( m_activeTool->optionWidget() );
//         }
//         else {
//             QString name;
//             foreach( ToolHelper * tool, m_tools ) {
//                 if ( tool->toolId() == m_activeTool->toolId() ) {
//                     name = tool->name();
//                     break;
//                 }
//             }
//             m_activeCanvas->toolOptionDocker()->setDummyText(i18n( "Active tool: %1" ).arg( name ) );
//         }
    }
    emit changedTool(m_uniqueToolIds.value(m_activeTool));
}

void KoToolManager::attachCanvas(KoCanvasController *controller) {
    QHash<QString, KoTool*> toolsHash;
    foreach(ToolHelper *tool, m_tools) {
        kDebug() << "Creating tool " << tool->id() << ", " << tool->activationShapeId() << endl;
        KoTool *tl = tool->createTool(controller->canvas());
        m_uniqueToolIds.insert(tl, tool->uniqueId());
        toolsHash.insert(tool->id(), tl);
        tl->setObjectName(tool->id());
    }
    KoCreateShapesTool *createTool = dynamic_cast<KoCreateShapesTool*>(toolsHash.value(KoCreateShapesTool_ID));
    Q_ASSERT(createTool);
    QString id = KoShapeRegistry::instance()->keys()[0];
    createTool->setShapeId(id);

    m_allTools.remove(controller);
    m_allTools.insert(controller, toolsHash);

    if (m_activeTool == 0) {
        toolActivated(m_defaultTool);
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
    if (to == 0 || to == m_activeCanvas)
        return;

    KoCanvasController *newCanvas = 0;
    // if the 'to' is one of our canvasses, or one of its children, then switch.
    foreach(KoCanvasController* canvas, m_canvases) {
        if (canvas == to || canvas->canvas()->canvasWidget() == to) {
            newCanvas = canvas;
            break;
        }
    }

    if (newCanvas == 0)
        return;
    if (newCanvas == m_activeCanvas)
        return;
    if (m_activeCanvas) {
        m_activeCanvas->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }
    m_activeCanvas = newCanvas;

    switchTool(m_activeToolId, false);
    selectionChanged(m_activeCanvas->canvas()->shapeManager()->selection()->selectedShapes());
}

void KoToolManager::detachCanvas(KoCanvasController *controller) {
    if (m_activeCanvas == controller)
        m_activeCanvas = 0;
    m_activeTool = 0;
    QHash<QString, KoTool*> toolsHash = m_allTools.value(controller);
    foreach(KoTool *tool, toolsHash.values())
        delete tool;
    toolsHash.clear();
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
    if (m_stack.isEmpty()) {
        // default to changing to the interactionTool
        switchTool(KoInteractionTool_ID, false);
        return;
    }
    switchTool(m_stack.pop(), false);
}

KoCreateShapesTool * KoToolManager::shapeCreatorTool(KoCanvasBase *canvas) const
{
    foreach(KoCanvasController *controller, m_canvases) {
        if (controller->canvas() == canvas) {
            QHash<QString, KoTool*> tools = m_allTools.value(controller);

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

    // check if all previous selected shapes are still selected
    // if not change the current tool to the default tool
    if ( m_activeToolId != KoInteractionTool_ID ) {
        foreach( KoShape* shape, m_lastSelectedShapes ) {
            if ( ! shapes.contains( shape ) ) {
                switchTool(KoInteractionTool_ID, false);
                break;
            }
        }
    }
    m_lastSelectedShapes = shapes;

    emit toolCodesSelected(types);
}

KoCanvasController *KoToolManager::activeCanvasController() const {
    return m_activeCanvas;
}

KoToolProxy *KoToolManager::createToolProxy(KoCanvasBase *parentCanvas) {
    ToolProxy *tp = new ToolProxy(parentCanvas);
    m_proxies.insert(parentCanvas, tp);
    return tp;
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
