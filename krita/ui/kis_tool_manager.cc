/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include "kopalettemanager.h"

#include "kis_part_layer.h"
#include "kis_tool_manager.h"
#include "kis_tool_registry.h"
#include "kis_tool_dummy.h"
#include "kis_canvas_subject.h"
#include "kis_tool_controller.h"
#include "kis_view.h"
#include "kis_canvas.h"
#include "kis_cursor.h"
#include "KoToolBox.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_input_device.h"


KisToolManager::KisToolManager(KisCanvasSubject * parent, KisCanvasController * controller)
    : m_subject(parent),
      m_controller(controller)
{
    m_toolBox = 0;
    m_oldTool = 0;
    m_dummyTool = 0;
    m_paletteManager = 0;
    m_actionCollection = 0;
    m_tools_disabled = false;
    setup = false;
}

KisToolManager::~KisToolManager()
{
    delete m_dummyTool;
}

void KisToolManager::setUp(KoToolBox * toolbox, KoPaletteManager * paletteManager, KActionCollection * actionCollection)
{
    if (setup) {
        resetToolBox( toolbox );
        return;
    }

    m_toolBox = toolbox;
    m_paletteManager = paletteManager;
    m_actionCollection = actionCollection;

    // Dummy tool for when the layer is locked or invisible
    if (!m_dummyTool)
        m_dummyTool = KisToolDummyFactory().createTool(actionCollection);

    QValueVector<KisInputDevice> inputDevices = KisInputDevice::inputDevices();

    for (Q_UINT32 inputDevice = 0; inputDevice < inputDevices.count(); inputDevice++) {
        m_inputDeviceToolSetMap[inputDevices[inputDevice]] = KisToolRegistry::instance()->createTools(actionCollection, m_subject);
    }

    m_tools = m_inputDeviceToolSetMap[KisInputDevice::mouse()];
    for (vKisTool_it it = m_tools.begin(); it != m_tools.end(); ++it) {
        KisTool * t = *it;
        if (!t) continue;
        toolbox->registerTool( t->action(), t->toolType(), t->priority() );
    }

    toolbox->setupTools();

    KisTool * t = findTool("tool_brush");
    if (t) {
        t->activate();
        setCurrentTool(t);
    }
    setup = true;
        
}



void KisToolManager::youAintGotNoToolBox()
{
    m_toolBox = 0;
    m_oldTool = currentTool();
}

void KisToolManager::resetToolBox(KoToolBox * toolbox)
{
    m_toolBox = toolbox;

    m_tools = m_inputDeviceToolSetMap[KisInputDevice::mouse()];
    for (vKisTool_it it = m_tools.begin(); it != m_tools.end(); ++it) {
        KisTool * t = *it;
        if (!t) continue;
        m_toolBox->registerTool( t->action(), t->toolType(), t->priority() );
    }

    toolbox->setupTools();

#if 0 //  Because I cannot find out how to reset the toolbox so the button is depressed, we reset the tool to brush
    setCurrentTool(findTool("tool_brush"));
#else
    if (m_oldTool) {
         // restore the old current tool
         setCurrentTool(m_oldTool);
         m_oldTool = 0;
    }
#endif

}

void KisToolManager::updateGUI()
{
    Q_ASSERT(m_subject);
    if (m_subject == 0) {
        // "Eek, no parent!
        return;
    }

    if (!m_toolBox) return;

    KisImageSP img = m_subject->currentImg();
    KisLayerSP l = 0;

    bool enable = false;


    KisPartLayer * partLayer = dynamic_cast<KisPartLayer*>(l.data());
    
    if (img) {
        l = img->activeLayer();
        enable = l && !l->locked() && l->visible() && (partLayer == 0);
    }

    m_toolBox->enableTools( enable );

    KisTool * current = currentTool();

    // XXX: Fix this properly: changing the visibility of a layer causes this cause to be executed twice!
    if (!enable && current != m_dummyTool) {
        // Store the current tool
        m_oldTool = currentTool();
        // Set the dummy tool
        if (!m_dummyTool) {
            m_dummyTool = KisToolDummyFactory().createTool(m_actionCollection);
        }
        setCurrentTool(m_dummyTool);
        m_tools_disabled = true;
    }
    else if (enable && m_tools_disabled) {
        m_tools_disabled = false;
        if (m_oldTool) {
            // restore the old current tool
            setCurrentTool(m_oldTool);
            m_oldTool = 0;
        }
        else {
            m_oldTool = 0;
            KisTool * t = findTool("tool_brush");
            setCurrentTool(t);
        }
    }
}

void KisToolManager::setCurrentTool(KisTool *tool)
{
    KisTool *oldTool = currentTool();
    KisCanvas * canvas = (KisCanvas*)m_controller->kiscanvas();


    if (oldTool)
    {
        oldTool->deactivate();
        oldTool->action()->setChecked( false );

        m_paletteManager->removeWidget(krita::TOOL_OPTION_WIDGET);
    }

    if (tool) {

        if (!tool->optionWidget()) {
            tool->createOptionWidget(0);
        }
        QWidget * w = tool->optionWidget();
        
        if (w)
            m_paletteManager->addWidget(w, krita::TOOL_OPTION_WIDGET, krita::CONTROL_PALETTE );

        m_inputDeviceToolMap[m_controller->currentInputDevice()] = tool;
        m_controller->setCanvasCursor(tool->cursor());

        canvas->enableMoveEventCompressionHint(dynamic_cast<KisToolNonPaint *>(tool) != NULL);

        m_subject->notifyObservers();

        tool->action()->setChecked( true );
        tool->action()->activate();
        m_toolBox->slotSetTool(tool->name());
    } else {
        m_inputDeviceToolMap[m_controller->currentInputDevice()] = 0;
        m_controller->setCanvasCursor(KisCursor::arrowCursor());
    }

}

void KisToolManager::setCurrentTool( const QString & toolName )
{
    setCurrentTool(findTool(toolName));
}

KisTool * KisToolManager::currentTool() const
{
    InputDeviceToolMap::const_iterator it = m_inputDeviceToolMap.find(m_controller->currentInputDevice());

    if (it != m_inputDeviceToolMap.end()) {
        return (*it).second;
    } else {
        return 0;
    }
}


void KisToolManager::setToolForInputDevice(KisInputDevice oldDevice, KisInputDevice newDevice)
{
    InputDeviceToolSetMap::iterator vit = m_inputDeviceToolSetMap.find(oldDevice);

    if (vit != m_inputDeviceToolSetMap.end()) {
        vKisTool& oldTools = (*vit).second;
        for (vKisTool::iterator it = oldTools.begin(); it != oldTools.end(); ++it) {
            KisTool *tool = *it;
            KAction *toolAction = tool->action();
            toolAction->disconnect(SIGNAL(activated()), tool, SLOT(activate()));
        }
    }
    KisTool *oldTool = currentTool();
    if (oldTool)
    {
        m_paletteManager->removeWidget(krita::TOOL_OPTION_WIDGET);
        oldTool->deactivate();
    }


    vit = m_inputDeviceToolSetMap.find(newDevice);

    Q_ASSERT(vit != m_inputDeviceToolSetMap.end());

    vKisTool& tools = (*vit).second;

    for (vKisTool::iterator it = tools.begin(); it != tools.end(); ++it) {
        KisTool *tool = *it;
        KAction *toolAction = tool->action();
        connect(toolAction, SIGNAL(activated()), tool, SLOT(activate()));
    }
}

void KisToolManager::activateCurrentTool()
{
    KisTool * t = currentTool();
    if (t && t->action()) {
        t->action()->activate();
    }
}

KisTool * KisToolManager::findTool(const QString &toolName, KisInputDevice inputDevice) const
{
    if (inputDevice == KisInputDevice::unknown()) {
        inputDevice = m_controller->currentInputDevice();
    }

    KisTool *tool = 0;

    InputDeviceToolSetMap::const_iterator vit = m_inputDeviceToolSetMap.find(inputDevice);

    Q_ASSERT(vit != m_inputDeviceToolSetMap.end());

    const vKisTool& tools = (*vit).second;

    for (vKisTool::const_iterator it = tools.begin(); it != tools.end(); ++it) {
        KisTool *t = *it;
        if (t->name() == toolName) {
            tool = t;
            break;
        }
    }

    return tool;
}


#include "kis_tool_manager.moc"
