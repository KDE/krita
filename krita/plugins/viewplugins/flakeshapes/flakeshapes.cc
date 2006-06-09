/*
 * flakeshapes.cc -- Part of Krita
 *
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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


#include <math.h>

#include <stdlib.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_view.h>

#include "flakeshapes.h"
#include "kis_image_shape.h"
#include "kis_paint_device_shape.h"

typedef KGenericFactory<FlakeShapes> FlakeShapesFactory;
K_EXPORT_COMPONENT_FACTORY( kritaflakeshapes, FlakeShapesFactory( "krita" ) )

FlakeShapes::FlakeShapes(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{

    if ( parent->inherits("KoShapeRegistry") )
    {
        setInstance(FlakeShapesFactory::instance());
        KoShapeRegistry::instance()->add( new KoPaintDeviceShapeFactory() );
        KoShapeRegistry::instance()->add( new KoImageShapeFactory() );
    }
}

FlakeShapes::~FlakeShapes()
{
}


#include "flakeshapes.moc"

