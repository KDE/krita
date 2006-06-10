/*
 *  Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
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
#include "KoToolManager.h"
//#include "KoToolBox.h"
#include "KoToolRegistry.h"

#include <KoToolFactory.h>
#include <KoCreateShapesToolFactory.h>
#include <KoInteractionToolFactory.h>
#include <KoCanvasView.h>

#include "kactioncollection.h"
#include "kdebug.h"

#include <QToolButton>
#include <QButtonGroup>
#include <QVBoxLayout>

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
, m_toolBox(0)
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
    m_tools.append( new ToolHelper(new KoCreateShapesToolFactory()) );
    m_defaultTool = new ToolHelper(new KoInteractionToolFactory());
    m_tools.append(m_defaultTool);

    KoToolRegistry *registry = KoToolRegistry::instance();
    foreach(KoID id, registry->listKeys()) {
        ToolHelper *t = new ToolHelper(registry->get(id));
kDebug(30004) << "   th" << t->id().name() << endl;
        connect(t, SIGNAL(toolActivated(ToolHelper*)), this, SLOT(toolActivated(ToolHelper*)));
        m_tools.append(t);
    }

    // connect to all tools so we can hear their button-clicks
    foreach(ToolHelper *tool, m_tools)
        connect(tool, SIGNAL(toolActivated(ToolHelper*)), this, SLOT(toolActivated(ToolHelper*)));

    // do some magic for the sorting here :)  TODO
    m_mutex.unlock();
}

QWidget *KoToolManager::toolBox() {
    m_mutex.lock();
    if(! m_toolBox) {
        setup();
        QWidget *widget = new QWidget();
        widget->setMinimumSize(20, 100);
        QButtonGroup *group = new QButtonGroup(widget);
        QVBoxLayout *lay = new QVBoxLayout(widget);
        foreach(ToolHelper *tool, m_tools) {
            QAbstractButton *but = tool->createButton(widget);
            group->addButton(but);
            lay->addWidget(but);
        }
        m_toolBox = widget;
    }
    m_mutex.unlock();
    return m_toolBox;
}

void KoToolManager::registerTools(KActionCollection *ac) {
    m_mutex.lock();
    setup();
    // TODO
    m_mutex.unlock();
}

void KoToolManager::addCanvasView(KoCanvasView *view) {
    if(m_canvases.contains(view))
        return;
    setup();
    m_mutex.lock();
    kDebug(30004) << "KoToolManager::addCanvasView called, setting up..." << endl;
    m_canvases.append(view);
    //QMap<KoID, KoTool*> toolsMap;
    //m_allTools.insert(view, toolsMap);
    if(m_activeCanvas == 0)
        m_activeCanvas = view;
    if(view->canvas())
        attachCanvas(view);
    connect(view, SIGNAL(canvasRemoved(KoCanvasView*)), this, SLOT(detachCanvas(KoCanvasView*)));
    connect(view, SIGNAL(canvasSet(KoCanvasView*)), this, SLOT(attachCanvas(KoCanvasView*)));
    m_mutex.unlock();
}

void KoToolManager::removeCanvasView(KoCanvasView *view) {
    m_mutex.lock();
    m_canvases.removeAll(view);
    QMap<QString, KoTool*> toolsMap = m_allTools.value(view);
    foreach(KoTool *tool, toolsMap.values())
        delete tool;
    m_allTools.remove(view);
    if(view->canvas())
        detachCanvas(view);
    disconnect(view, SIGNAL(canvasRemoved(KoCanvasView*)), this, SLOT(detachCanvas(KoCanvasView*)));
    disconnect(view, SIGNAL(canvasSet(KoCanvasView*)), this, SLOT(attachCanvas(KoCanvasView*)));
    m_mutex.unlock();
}

void KoToolManager::toolActivated(ToolHelper *tool) {
    kDebug(30004) << "ToolActivated: '" << tool->id().name() << "'\n";
    QMap<QString, KoTool*> toolsMap = m_allTools.value(m_activeCanvas);
    KoTool *t = toolsMap.value(tool->id().id());
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
        disconnect(m_activeTool, SIGNAL(sigActivateTool(const QString &id)),
                this, SLOT(switchToolRequested(const QString &id)));
        disconnect(m_activeTool, SIGNAL(sigActivateTemporary(const QString &id)),
                this, SLOT(switchToolTemporaryRequested(const QString &id)));
        disconnect(m_activeTool, SIGNAL(sigDone()), this, SLOT(switchBackRequested()));
    }
    if(m_activeTool && temporary)
        m_stack.push(m_activeTool);
    m_activeTool = tool;
    connect(m_activeTool, SIGNAL(sigCursorChanged(QCursor)),
            this, SLOT(updateCursor(QCursor)));
    connect(m_activeTool, SIGNAL(sigActivateTool(const QString &id)),
            this, SLOT(switchToolRequested(const QString &id)));
    connect(m_activeTool, SIGNAL(sigActivateTemporary(const QString &id)),
            this, SLOT(switchToolTemporaryRequested(const QString &id)));
    connect(m_activeTool, SIGNAL(sigDone()), this, SLOT(switchBackRequested()));

    // and set it.
    foreach(KoCanvasView *view, m_canvases) {
        if(!view->canvas())
            continue;
        view->canvas()->setTool(view==m_activeCanvas ? m_activeTool : m_dummyTool);
        // we expect the tool to emit a cursur on activation.  This is for quick-fail :)
        view->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }
    m_activeTool->activate();
    m_mutex.unlock();
}

void KoToolManager::attachCanvas(KoCanvasView *view) {
kDebug(30004) << "KoToolManager::attachCanvas\n";
    // TODO listen to focus changes
    // TODO listen to selection changes
    QMap<QString, KoTool*> toolsMap;
    foreach(ToolHelper *tool, m_tools)
        toolsMap.insert(tool->id().id(), tool->m_toolFactory->createTool(view->canvas()));
    m_mutex.lock();
    m_allTools.remove(view);
    m_allTools.insert(view, toolsMap);
    m_mutex.unlock();

    if(m_activeTool == 0)
        toolActivated(m_defaultTool);
    else {
        view->canvas()->setTool(m_dummyTool);
        view->canvas()->canvasWidget()->setCursor(Qt::ForbiddenCursor);
    }
}

void KoToolManager::detachCanvas(KoCanvasView *view) {
    // TODO detach
    if(m_activeCanvas == view)
        m_activeCanvas = 0;
    QMap<QString, KoTool*> toolsMap = m_allTools.value(view);
    foreach(KoTool *tool, toolsMap.values())
        delete tool;
    toolsMap.clear();
    view->canvas()->setTool(m_dummyTool);
}

void KoToolManager::updateCursor(QCursor cursor) {
    Q_ASSERT(m_activeCanvas);
    Q_ASSERT(m_activeCanvas->canvas());
    m_activeCanvas->canvas()->canvasWidget()->setCursor(cursor);
}

void KoToolManager::switchToolRequested(const QString &id) {
    while (!m_stack.isEmpty()) // switching means to flush the stack
        m_stack.pop();
    switchTool(id, false);
}

void KoToolManager::switchToolTemporaryRequested(const QString &id) {
    switchTool(id, true);
}

void KoToolManager::switchBackRequested() {
    if(m_stack.isEmpty()) {
        kWarning(30004) << "Tool emits a done while it is not temporary, ignoring";
        return;
    }
    switchTool(m_stack.pop(), false);
}

//   ************ ToolHelper **********
QAbstractButton* ToolHelper::createButton(QWidget *parent) {
    QToolButton *but = new QToolButton(parent);
    but->setText(m_toolFactory->id().name());
    connect(but, SIGNAL(clicked()), this, SLOT(buttonPressed()));
    return but;
}

void ToolHelper::buttonPressed() {
    emit toolActivated(this);
}

//static
KoToolManager* KoToolManager::s_instance = 0;
KoToolManager* KoToolManager::instance() {
    if(s_instance == 0)
        s_instance = new KoToolManager();
    return s_instance;
}

KoID ToolHelper::id() const {
    return m_toolFactory->id();
}

#include "KoToolManager.moc"
