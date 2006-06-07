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

KoToolManager::KoToolManager()
: QObject()
, m_toolBox(0)
, m_activeCanvas(0)
, m_activeTool(0)
{
    m_dummyTool = new DummyTool();
#if 0
    m_oldTool = 0;
    m_paletteManager = 0;
    m_tools_disabled = false;
#endif
}

KoToolManager::~KoToolManager()
{
    delete m_dummyTool;
}

void KoToolManager::setup() {
    if(m_tools.size() > 0)
        return;
    // add defaults
    m_tools.append( new ToolHelper(new KoCreateShapesToolFactory()) );
    m_tools.append( new ToolHelper(new KoInteractionToolFactory()) );

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
}

QWidget *KoToolManager::toolBox() {
// TODO reentrant
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
    return m_toolBox;
}

void KoToolManager::registerTools(KActionCollection *ac) {
    // TODO
}

void KoToolManager::addCanvasView(KoCanvasView *view) {
    if(m_canvases.contains(view))
        return;
    m_canvases.append(view);
    if(view->canvas())
        attachCanvas(view->canvas());
    connect(view, SIGNAL(canvasRemoved(KoCanvasBase*)), this, SLOT(detachCanvas(KoCanvasBase*)));
    connect(view, SIGNAL(canvasSet(KoCanvasBase*)), this, SLOT(attachCanvas(KoCanvasBase*)));
    if(m_activeCanvas == 0)
        m_activeCanvas = view;
}

void KoToolManager::removeCanvasView(KoCanvasView *view) {
    m_canvases.removeAll(view);
    if(view->canvas())
        detachCanvas(view->canvas());
    disconnect(view, SIGNAL(canvasRemoved(KoCanvasBase*)), this, SLOT(detachCanvas(KoCanvasBase*)));
    disconnect(view, SIGNAL(canvasSet(KoCanvasBase*)), this, SLOT(attachCanvas(KoCanvasBase*)));
}


void KoToolManager::toolActivated(ToolHelper *tool) {
    kDebug(30004) << "ToolActivated: '" << tool->id().name() << "'\n";
    if(m_activeCanvas == 0)
        return;
    if(m_activeTool)
        delete m_activeTool;
    m_activeTool = tool->m_toolFactory->createTool(m_activeCanvas->canvas());
    foreach(KoCanvasView *view, m_canvases) {
        if(!view->canvas())
continue;
        if(view == m_activeCanvas)
            view->canvas()->setTool(m_activeTool);
        else
            view->canvas()->setTool(m_dummyTool);
    }
}

void KoToolManager::attachCanvas(KoCanvasBase *cb) {
    // TODO listen to focus changes
    // TODO listen to selection changes
    cb->setTool(m_dummyTool);
}

void KoToolManager::detachCanvas(KoCanvasBase *cb) {
    // TODO detach
    if(m_activeCanvas && m_activeCanvas->canvas() == cb)
        m_activeCanvas = 0;
    // TODO delete tool that canvas is holding?
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
