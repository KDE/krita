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
#include "kis_tool.h"
#include "kis_tool_factory.h"

#if 0
// Working tools -- note that the 'paste' tool is added in the KisView, not here.
#include "kis_tool_select_rectangular.h"
#include "kis_tool_move.h"
#include "kis_tool_zoom.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_test.h"
#include "kis_tool_qpen.h"
#include "kis_tool_brush.h"
#endif

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

void KisToolFactory::create(KActionCollection *actionCollection, KisCanvasSubject *subject)
{
#if 0
    vKisTool tools;

    Q_ASSERT(view);
    Q_ASSERT(doc);
    tools.reserve(25);

#if 0
    // painting tools
    tools.push_back(new AirBrushTool(doc, brush));
    tools.push_back(new PenTool(doc, canvas, brush));
    tools.push_back(new EraserTool(doc, brush));
#endif

    tools.push_back( new KisToolTest( view, doc ));
    tools.push_back( new KisToolQPen( view, doc ));
    tools.push_back( new KisToolBrush( view, doc ));
    tools.push_back( new KisToolColorPicker( view, doc ));

#if 0
    tools.push_back(new ColorChangerTool(doc));
    tools.push_back(new FillTool(doc));
    tools.push_back(new StampTool(doc, canvas, pattern));

    // Positioning tools
#endif
    tools.push_back(new KisZoomTool(view, doc));
    tools.push_back(new KisToolMove(view, doc));

    // selection tools
#if 0
    tools.push_back(new FreehandSelectTool(doc, canvas));
#endif
    tools.push_back(new KisToolRectangularSelect(view, doc));
#if 0
    tools.push_back(new PolygonalSelectTool(doc, canvas));
    tools.push_back(new EllipticalSelectTool(doc, canvas));
    tools.push_back(new ContiguousSelectTool(doc, canvas));

    // drawing tools
    tools.push_back(new LineTool(doc, canvas));
    tools.push_back(new PolyLineTool(doc, canvas));
    tools.push_back(new PolyGonTool(doc, canvas));
    tools.push_back(new RectangleTool(doc, canvas));
    tools.push_back(new EllipseTool(doc, canvas));
#endif

    for (vKisTool_it it = tools.begin(); it != tools.end(); it++)
        (*it) -> setup();

    return tools;

#endif
}

KisToolFactory *KisToolFactory::singleton()
{
	return KisToolFactory::m_singleton;
}

