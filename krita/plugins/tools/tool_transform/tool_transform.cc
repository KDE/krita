/*
 * tool_transform.cc -- Part of Krita
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "tool_transform.h"
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


#include "kis_tool_transform.h"

K_PLUGIN_FACTORY(ToolTransformFactory, registerPlugin<ToolTransform>();)
K_EXPORT_PLUGIN(ToolTransformFactory("krita"))


ToolTransform::ToolTransform(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    //setComponentData(ToolTransformFactory::componentData());

    KoToolRegistry * r = KoToolRegistry::instance();
    r->add(new KisToolTransformFactory(r, QStringList()));

}

ToolTransform::~ToolTransform()
{
}

#include "tool_transform.moc"
