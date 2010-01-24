/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


// Own
#include "VectorShapePlugin.h"

// KDE
#include <kgenericfactory.h>

// KOffice libs
#include <KoShapeRegistry.h>
#include <KoToolRegistry.h>

// VectorShape
#include "VectorToolFactory.h"
#include "VectorShapeFactory.h"


K_EXPORT_COMPONENT_FACTORY(vectorshape, KGenericFactory<VectorShapePlugin>( "VectorShape" ) )

VectorShapePlugin::VectorShapePlugin(QObject * parent, const QStringList &)
    : QObject(parent)
{
    //KoToolRegistry::instance()->add(new VectorToolFactory(parent));
    KoShapeRegistry::instance()->add(new VectorShapeFactory(parent));
}

#include <VectorShapePlugin.moc>
