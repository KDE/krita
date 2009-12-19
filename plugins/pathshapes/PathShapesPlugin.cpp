/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>

#include "PathShapesPlugin.h"
#include "star/KoStarShapeFactory.h"
#include "rectangle/KoRectangleShapeFactory.h"
#include "ellipse/KoEllipseShapeFactory.h"
#include "spiral/KoSpiralShapeFactory.h"
#include "enhancedpath/KoEnhancedPathShapeFactory.h"
#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY(pathshapes,
                           KGenericFactory<PathShapesPlugin>("PathShapes"))

PathShapesPlugin::PathShapesPlugin(QObject *parent, const QStringList&)
    : QObject(parent)
{
    KoShapeRegistry::instance()->add(new KoStarShapeFactory(parent));
    KoShapeRegistry::instance()->add(new KoRectangleShapeFactory(parent));
    KoShapeRegistry::instance()->add(new KoEllipseShapeFactory(parent));
    KoShapeRegistry::instance()->add(new KoSpiralShapeFactory(parent));
    KoShapeRegistry::instance()->add(new KoEnhancedPathShapeFactory(parent));
}

#include "PathShapesPlugin.moc"
