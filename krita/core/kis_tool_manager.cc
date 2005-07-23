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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "koPaletteManager.h"

#include "kis_tool_manager.h"
#include "kis_tool_registry.h"
#include "kis_tool_dummy.h"
#include "kis_canvas_subject.h"
#include "kis_tool_controller.h"
#include "kis_view.h"
#include "kis_canvas.h"
#include "kis_cursor.h"
#include "kis_toolbox.h"

KisToolManager::KisToolManager(KisCanvasSubject * parent, KisCanvasControllerInterface * controller)
	: m_subject(parent),
	  m_controller(controller)
{
}

KisToolManager::~KisToolManager()
{
}

void KisToolManager::setUp(KisToolBox * toolbox, KoPaletteManager * paletteManager, KActionCollection * actionCollection)
{
	m_paletteManager = paletteManager;
	m_actionCollection = actionCollection;
	
	// Dummy tool for when the layer is locked or invisible
	KisToolRegistry::instance()->add(KisID("dummy", i18n("Dummy")), new KisToolDummyFactory(actionCollection));

	m_inputDeviceToolSetMap[INPUT_DEVICE_MOUSE] = KisToolRegistry::instance() -> createTools(m_subject);
	m_inputDeviceToolSetMap[INPUT_DEVICE_STYLUS] = KisToolRegistry::instance() -> createTools(m_subject);
	m_inputDeviceToolSetMap[INPUT_DEVICE_ERASER] = KisToolRegistry::instance() -> createTools(m_subject);
	m_inputDeviceToolSetMap[INPUT_DEVICE_PUCK] = KisToolRegistry::instance() -> createTools(m_subject);
	
	KisIDList keys = KisToolRegistry::instance()->listKeys();
	for ( KisIDList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
		KisTool * t = KisToolRegistry::instance()->get(*it)->createTool();

		if (!t) continue;

		toolbox->registerTool( t->action(), t->toolType(), t->priority() );

		delete(t); // These tools share their action, and the action for this tool has already been created.
	}
	
	toolbox->setupTools();

	setCurrentTool(findTool("tool_brush"));
}

void KisToolManager::updateGUI()
{
}

void KisToolManager::setCurrentTool(KisTool *tool)
{
	KisTool *oldTool = currentTool();
	KisCanvas * canvas = (KisCanvas*)m_controller->canvas();
	
	if (!tool->optionWidget()) {
		tool->createOptionWidget(0);
	}
	
	m_paletteManager->addWidget(tool->optionWidget(), krita::TOOL_OPTION_WIDGET, krita::CONTROL_PALETTE );

	if (oldTool)
	{
		oldTool -> clear();
		oldTool -> action() -> setChecked( false );
	}

	if (tool) {
		m_inputDeviceToolMap[m_controller->currentInputDevice()] = tool;
		m_controller->setCanvasCursor(tool->cursor());
		
		canvas->enableMoveEventCompressionHint(dynamic_cast<KisToolNonPaint *>(tool) != NULL);

		m_subject->notify();
		
		tool->action()->setChecked( true );

	} else {
		m_inputDeviceToolMap[m_controller->currentInputDevice()] = 0;
		m_controller->setCanvasCursor(KisCursor::arrowCursor());
	}

}

void KisToolManager::setCurrentTool( const QString & toolName)
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


void KisToolManager::setToolForInputDevice(enumInputDevice oldDevice, enumInputDevice newDevice)
{
	InputDeviceToolSetMap::iterator vit = m_inputDeviceToolSetMap.find(oldDevice);

	if (vit != m_inputDeviceToolSetMap.end()) {
		vKisTool& oldTools = (*vit).second;
		for (vKisTool::iterator it = oldTools.begin(); it != oldTools.end(); it++) {
			KisTool *tool = *it;
			KAction *toolAction = tool -> action();
			toolAction -> disconnect(SIGNAL(activated()), tool, SLOT(activate()));
		}
	}
	KisTool *oldTool = currentTool();
	if (oldTool)
	{
		m_paletteManager -> removeWidget(krita::TOOL_OPTION_WIDGET);
		oldTool -> clear();
	}

	
	vit = m_inputDeviceToolSetMap.find(newDevice);

	Q_ASSERT(vit != m_inputDeviceToolSetMap.end());

	vKisTool& tools = (*vit).second;

	for (vKisTool::iterator it = tools.begin(); it != tools.end(); it++) {
		KisTool *tool = *it;
		KAction *toolAction = tool -> action();

		connect(toolAction, SIGNAL(activated()), tool, SLOT(activate()));
	}
}

void KisToolManager::activateCurrentTool()
{
	currentTool() -> action() -> activate();
}

KisTool * KisToolManager::findTool(const QString &toolName, enumInputDevice inputDevice) const
{
	if (inputDevice == INPUT_DEVICE_UNKNOWN) {
		inputDevice = m_controller->currentInputDevice();
	}

	KisTool *tool = 0;

	InputDeviceToolSetMap::const_iterator vit = m_inputDeviceToolSetMap.find(inputDevice);

	Q_ASSERT(vit != m_inputDeviceToolSetMap.end());

	const vKisTool& tools = (*vit).second;

	for (vKisTool::const_iterator it = tools.begin(); it != tools.end(); it++) {
		KisTool *t = *it;
		if (t -> name() == toolName) {
			tool = t;
			break;
		}
	}

	return tool;
}


#include "kis_tool_manager.moc"
