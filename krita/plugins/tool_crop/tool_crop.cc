/* 
 * tool_crop.cc -- Part of Krita
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
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <kis_plugin_registry.h>
#include <kis_tool_registry.h>
#include <kis_view.h>

#include "tool_crop.h"
#include "kis_tool_crop.h"


typedef KGenericFactory<ToolCrop> ToolCropFactory;
K_EXPORT_COMPONENT_FACTORY( kritatoolcrop, ToolCropFactory( "krita" ) )


ToolCrop::ToolCrop(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
       	setInstance(ToolCropFactory::instance());

 	kdDebug() << "Crop tool plugin. Class: " 
 		  << className() 
 		  << ", Parent: " 
 		  << parent -> className()
 		  << "\n";

 	if ( parent->inherits("KisView") )
 	{
		m_view = (KisView*) parent;

		KisToolRegistry * r = m_view -> toolRegistry();

		r -> add(new KisToolCropFactory(actionCollection()));
 	}
	
}

ToolCrop::~ToolCrop()
{
}

#include "tool_crop.moc"
