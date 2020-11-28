/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include <KoShapeRegistry.h>
#include <KoShapeFactoryBase.h>

#include "PathShapesPlugin.h"
#include "star/StarShapeFactory.h"
#include "rectangle/RectangleShapeFactory.h"
#include "ellipse/EllipseShapeFactory.h"
#include "spiral/SpiralShapeFactory.h"
#include "enhancedpath/EnhancedPathShapeFactory.h"
#include <kpluginfactory.h>

K_PLUGIN_FACTORY_WITH_JSON(PathShapesPluginFactory, "calligra_shape_paths.json", registerPlugin<PathShapesPlugin>();)

PathShapesPlugin::PathShapesPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoShapeRegistry::instance()->add(new StarShapeFactory());
    KoShapeRegistry::instance()->add(new RectangleShapeFactory());
    KoShapeRegistry::instance()->add(new SpiralShapeFactory());
    KoShapeRegistry::instance()->add(new EnhancedPathShapeFactory());
    KoShapeRegistry::instance()->add(new EllipseShapeFactory());

}

#include <PathShapesPlugin.moc>
