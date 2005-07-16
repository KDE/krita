/*
 * selection_tools.cc -- Part of Krita
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
#include <kis_view.h>

#include "selection_tools.h"

#include "kis_tool_select_outline.h"
#include "kis_tool_select_polygonal.h"
#include "kis_tool_select_rectangular.h"
#include "kis_tool_select_contiguous.h"
#include "kis_tool_select_elliptical.h"
#include "kis_tool_select_eraser.h"
#include "kis_tool_select_brush.h"

typedef KGenericFactory<SelectionTools> SelectionToolsFactory;
K_EXPORT_COMPONENT_FACTORY( kritaselectiontools, SelectionToolsFactory( "krita" ) )


SelectionTools::SelectionTools(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
       	setInstance(SelectionToolsFactory::instance());

 	kdDebug(DBG_AREA_PLUGINS) << "Selection tools plugin. Class: "
 		  << className()
 		  << ", Parent: "
 		  << parent -> className()
 		  << "\n";

 	if ( parent->inherits("KisView") )
 	{
		KisView * view = dynamic_cast<KisView*>( parent );
		KisToolRegistry * r = view -> toolRegistry();
		r -> add(new KisToolSelectOutlineFactory( actionCollection() ));
		r -> add(new KisToolSelectPolygonalFactory( actionCollection() ));
		r -> add(new KisToolSelectRectangularFactory( actionCollection() ));
		r -> add(new KisToolSelectBrushFactory( actionCollection() ));
		r -> add(new KisToolSelectContiguousFactory( actionCollection() ));
		r -> add(new KisToolSelectEllipticalFactory( actionCollection() ));
		r -> add(new KisToolSelectEraserFactory( actionCollection() ));
        }
}

SelectionTools::~SelectionTools()
{
}

#include "selection_tools.moc"
