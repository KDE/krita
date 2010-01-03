/*
 * tool_perspectivetransform.cc -- Part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "tool_perspectivetransform.h"


#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <KoToolRegistry.h>


#include "kis_tool_perspectivetransform.h"


typedef KGenericFactory<ToolPerspectiveTransform> ToolPerspectiveTransformFactory;
K_EXPORT_COMPONENT_FACTORY(kritatoolperspectivetransform, ToolPerspectiveTransformFactory("krita"))


ToolPerspectiveTransform::ToolPerspectiveTransform(QObject *parent, const QStringList &)
        : QObject(parent)
{
    //setComponentData(ToolPerspectiveTransformFactory::componentData());

    KoToolRegistry * r = KoToolRegistry::instance();
    r->add(new KisToolPerspectiveTransformFactory(r, QStringList()));
}

ToolPerspectiveTransform::~ToolPerspectiveTransform()
{
}

#include "tool_perspectivetransform.moc"
