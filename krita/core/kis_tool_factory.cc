/*
 * kis_tool_factory.cc -- part of Krita
 *
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_tool_pan.h"
#include "kis_tool_brush.h"
#include "kis_tool_duplicate.h"
#include "kis_tool_eraser.h"
#include "kis_tool_line.h"
#include "kis_tool_stamp.h"
#include "kis_tool_airbrush.h"
#include "kis_tool_paste.h"
#include "kis_tool_select_freehand.h"
#include "kis_tool_select_rectangular.h"
#include "kis_tool_select_polygonal.h"
#include "kis_tool_select_elliptical.h"
#include "kis_tool_select_contiguous.h"
#include "kis_tool_select_brush.h"
#include "kis_tool_pen.h"
#include "kis_tool_fill.h"
#include "kis_tool_rectangle.h"
#include "kis_tool_ellipse.h"
#include "kis_tool_polyline.h"
#include "kis_tool_polygon.h"
#include "kis_tool_colorchanger.h"
#include "kis_tool_gradient.h"
#include "kis_tool_text.h"

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
}

vKisTool KisToolFactory::create(KActionCollection *actionCollection, KisCanvasSubject *subject)
{
	Q_ASSERT(actionCollection);
	Q_ASSERT(subject);

	vKisTool tools;

	tools.push_back(new KisToolColorPicker);
	tools.push_back(new KisToolMove);
	tools.push_back(new KisToolZoom);
	tools.push_back(new KisToolPan);
	tools.push_back(new KisToolBrush);
	tools.push_back(new KisToolDuplicate);
	tools.push_back(new KisToolAirBrush);
	tools.push_back(new KisToolEraser);
	tools.push_back(new KisToolLine);
	//tools.push_back(new KisToolStamp);
	tools.push_back(new KisToolPaste);
	tools.push_back(new KisToolSelectRectangular);
	tools.push_back(new KisToolSelectFreehand);
	tools.push_back(new KisToolSelectPolygonal);
	tools.push_back(new KisToolSelectElliptical);
	tools.push_back(new KisToolSelectContiguous);
	tools.push_back(new KisToolSelectBrush);
	tools.push_back(new KisToolPen);
	tools.push_back(new KisToolRectangle);
	tools.push_back(new KisToolEllipse);
	tools.push_back(new KisToolPolyLine);
	tools.push_back(new KisToolPolygon);
	tools.push_back(new KisToolColorChanger);
	tools.push_back(new KisToolFill);
	tools.push_back(new KisToolGradient);
	tools.push_back(new KisToolText);

	for (vKisTool_it it = tools.begin(); it != tools.end(); it++) {
		KisTool *tool = *it;

		if (tool) {
			tool -> setup(actionCollection);
			subject -> attach(tool);
		}
	}

	subject -> notify();

	return tools;
}

KisToolFactory *KisToolFactory::singleton()
{
	return KisToolFactory::m_singleton;
}

