/*
 * tool_polygon.cc -- Part of Krita
 *
 * Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include <QPoint>

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
#include "tool_polygon.h"
#include "tool_polygon.moc"
#include "kis_tool_polygon.h"


typedef KGenericFactory<ToolPolygon> ToolPolygonFactory;
K_EXPORT_COMPONENT_FACTORY( kritatoolpolygon, ToolPolygonFactory( "krita" ) )


ToolPolygon::ToolPolygon(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    setInstance(ToolPolygonFactory::instance());

    if ( parent->inherits("KisToolRegistry") )
    {
        KisToolRegistry * r = dynamic_cast<KisToolRegistry*>( parent );
        r->add(KisToolFactorySP(new KisToolPolygonFactory()));
    }

}

ToolPolygon::~ToolPolygon()
{
}

//#include "tool_polygon.moc"
