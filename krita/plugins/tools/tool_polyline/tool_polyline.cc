/*
 * tool_polyline.cc -- Part of Krita
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

#include "tool_polyline.h"
#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <KoToolRegistry.h>


#include "kis_tool_polyline.h"

K_PLUGIN_FACTORY(ToolPolylineFactory, registerPlugin<ToolPolyline>();)
K_EXPORT_PLUGIN(ToolPolylineFactory("krita"))


ToolPolyline::ToolPolyline(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    //setComponentData(ToolPolylineFactory::componentData());

    KoToolRegistry * r = KoToolRegistry::instance();
    r->add(new KisToolPolylineFactory(r, QStringList()));

}

ToolPolyline::~ToolPolyline()
{
}

#include "tool_polyline.moc"
