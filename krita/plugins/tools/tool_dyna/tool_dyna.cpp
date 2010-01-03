/*
 * tool_dyna.cpp -- Part of Krita
 *
 * Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "tool_dyna.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kis_paint_device.h>
#include <kgenericfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <KoToolRegistry.h>


#include "kis_tool_dyna.h"


typedef KGenericFactory<ToolDyna> ToolDynaFactory;
K_EXPORT_COMPONENT_FACTORY(kritatooldyna, ToolDynaFactory("krita"))


ToolDyna::ToolDyna(QObject *parent, const QStringList &)
        : QObject(parent)
{
    //setComponentData(ToolDynaFactory::componentData());

    KoToolRegistry * r = KoToolRegistry::instance();
    r->add(new KisToolDynaFactory(r, QStringList()));
}

ToolDyna::~ToolDyna()
{
}

#include "tool_dyna.moc"
