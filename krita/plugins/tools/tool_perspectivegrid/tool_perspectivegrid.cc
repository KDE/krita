/*
 * tool_perspectivegrid.cc -- Part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#include <kis_tool_registry.h>

#include "tool_perspectivegrid.h"
#include "kis_tool_perspectivegrid.h"


typedef KGenericFactory<ToolPerspectiveGrid> ToolPerspectiveGridFactory;
K_EXPORT_COMPONENT_FACTORY( kritatoolperspectivegrid, ToolPerspectiveGridFactory( "krita" ) )


ToolPerspectiveGrid::ToolPerspectiveGrid(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    setInstance(ToolPerspectiveGridFactory::instance());

    if ( parent->inherits("KisToolRegistry") )
    {
        KisToolRegistry * r = dynamic_cast<KisToolRegistry*>(parent);
        r->add(KisToolFactorySP(new KisToolPerspectiveGridFactory()));
    }

}

ToolPerspectiveGrid::~ToolPerspectiveGrid()
{
}

#include "tool_perspectivegrid.moc"
