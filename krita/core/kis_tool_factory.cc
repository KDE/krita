/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
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
 *  along with view program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <kaction.h>
#include "kis_canvas_subject.h"
#include "kis_tool.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_factory.h"
#include "kis_tool_move.h"
#include "kis_tool_select_rectangular.h"
#include "kis_tool_zoom.h"
#include "kis_tool_brush.h"

namespace {
	KisToolFactory moveMe; // XXX Where to create singletons in Krita?!?
}

KisToolFactory *KisToolFactory::m_singleton = 0;

KisToolFactory::KisToolFactory()
{
	KisToolFactory::m_singleton = this;
}

KisToolFactory::~KisToolFactory()
{
	for (vKisTool_it it = m_tools.begin(); it != m_tools.end(); it++)
		delete *it;
}

void KisToolFactory::create(KActionCollection *actionCollection, KisCanvasSubject *subject)
{
	Q_ASSERT(actionCollection);
	Q_ASSERT(subject);

	if (m_tools.empty()) {
		m_tools.push_back(new KisToolColorPicker);
		m_tools.push_back(new KisToolMove);
		m_tools.push_back(new KisToolZoom);
		m_tools.push_back(new KisToolRectangularSelect);
		m_tools.push_back(new KisToolBrush);
	}

	for (vKisTool_it it = m_tools.begin(); it != m_tools.end(); it++) {
		KisTool *tool = *it;

		if (tool) {
			tool -> setup(actionCollection);
			subject -> attach(tool);
		}
	}

	subject -> notify();
}

KisToolFactory *KisToolFactory::singleton()
{
	return KisToolFactory::m_singleton;
}

