/*
 * default_tools.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#include <kis_tool.h>
#include <kis_factory.h>
#include <kis_tool_registry.h>
#include "default_tools.h"

#include "kis_tool_fill.h"
#include "kis_tool_brush.h"
#include "kis_tool_freehand.h"
#include "kis_tool_colorchanger.h"
#include "kis_tool_gradient.h"
#include "kis_tool_rectangle.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_line.h"
#include "kis_tool_text.h"
#include "kis_tool_duplicate.h"
#include "kis_tool_move.h"
#include "kis_tool_zoom.h"
#include "kis_tool_ellipse.h"
#include "kis_tool_pan.h"
#include "kis_view.h"

typedef KGenericFactory<DefaultTools> DefaultToolsFactory;
K_EXPORT_COMPONENT_FACTORY( kritadefaulttools, DefaultToolsFactory( "krita" ) )


DefaultTools::DefaultTools(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
       	setInstance(DefaultToolsFactory::instance());

 	kdDebug(DBG_AREA_PLUGINS) << "Default tools plugin. Class: "
 		  << className()
 		  << ", Parent: "
 		  << parent -> className()
 		  << "\n";

 	if ( parent->inherits("KisView") )
 	{
		KisView * view = dynamic_cast<KisView*>( parent );
		KisToolRegistry * r = view -> toolRegistry();

		r -> add(new KisToolFillFactory( actionCollection() ));
		r -> add(new KisToolGradientFactory( actionCollection() ));
		r -> add(new KisToolBrushFactory( actionCollection() ));
		r -> add(new KisToolColorPickerFactory( actionCollection() ));
		r -> add(new KisToolLineFactory( actionCollection() ));
		r -> add(new KisToolTextFactory( actionCollection() ));
		r -> add(new KisToolDuplicateFactory( actionCollection() ));
		r -> add(new KisToolMoveFactory( actionCollection() ));
		r -> add(new KisToolZoomFactory( actionCollection() ));
		r -> add(new KisToolEllipseFactory( actionCollection() ));
		r -> add(new KisToolRectangleFactory( actionCollection() ));
		r -> add(new KisToolPanFactory( actionCollection() ));

        }
}

DefaultTools::~DefaultTools()
{
}

#include "default_tools.moc"
