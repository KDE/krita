/* 
 * default_tools.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <kis_plugin_registry.h>
#include <kis_tool_registry.h>

#include "default_tools.h"
#include "kis_tool_fill.h"
#include "kis_tool_pen.h"
#include "kis_tool_select_freehand.h"
#include "kis_tool_airbrush.h"
#include "kis_tool_filter.h"
#include "kis_tool_polygon.h"
#include "kis_tool_select_polygonal.h"
#include "kis_tool_brush.h"
#include "kis_tool_freehand.h"
#include "kis_tool_polyline.h"
#include "kis_tool_select_rectangular.h"
#include "kis_tool_colorchanger.h"
#include "kis_tool_gradient.h"
#include "kis_tool_rectangle.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_line.h"
#include "kis_tool_select_brush.h"
#include "kis_tool_text.h"
#include "kis_tool_duplicate.h"
#include "kis_tool_move.h"
#include "kis_tool_select_contiguous.h"
#include "kis_tool_zoom.h"
#include "kis_tool_ellipse.h"
#include "kis_tool_pan.h"
#include "kis_tool_select_elliptical.h"
#include "kis_tool_eraser.h"
#include "kis_tool_paste.h"
#include "kis_tool_select_eraser.h"


typedef KGenericFactory<DefaultTools> DefaultToolsFactory;
K_EXPORT_COMPONENT_FACTORY( kritadefaulttools, DefaultToolsFactory( "krita" ) )


DefaultTools::DefaultTools(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
       	setInstance(DefaultToolsFactory::instance());

 	kdDebug() << "Default tools plugin. Class: " 
 		  << className() 
 		  << ", Parent: " 
 		  << parent -> className()
 		  << "\n";

 	if ( parent->inherits("KisPluginRegistry") )
 	{
		KisToolRegistry * r = KisToolRegistry::singleton();

		r -> add(new KisToolFillFactory());
		r -> add(new KisToolGradientFactory());
		r -> add(new KisToolPenFactory());
		r -> add(new KisToolSelectFreehandFactory());
		r -> add(new KisToolAirbrushFactory());
		r -> add(new KisToolFilterFactory());
		r -> add(new KisToolPolygonFactory());
		r -> add(new KisToolSelectPolygonalFactory());
		r -> add(new KisToolBrushFactory());
		r -> add(new KisToolPolyLineFactory());
		r -> add(new KisToolSelectRectangularFactory());
		r -> add(new KisToolColorPickerFactory());
		r -> add(new KisToolLineFactory());
		r -> add(new KisToolSelectBrushFactory());
		r -> add(new KisToolTextFactory());
		r -> add(new KisToolDuplicateFactory());
		r -> add(new KisToolMoveFactory());
		r -> add(new KisToolSelectContiguousFactory());
		r -> add(new KisToolZoomFactory());
		r -> add(new KisToolEllipseFactory());
		r -> add(new KisToolRectangleFactory());
		r -> add(new KisToolPanFactory());
		r -> add(new KisToolSelectEllipticalFactory());
		r -> add(new KisToolEraserFactory());
		r -> add(new KisToolPasteFactory());
		r -> add(new KisToolSelectEraserFactory());
 	}
	
}

DefaultTools::~DefaultTools()
{
}

#include "default_tools.moc"
