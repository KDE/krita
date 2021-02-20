/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2009 Inge Wallin <inge@lysator.liu.se>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

// Own
#include "ImageShapePlugin.h"

// KDE
#include <kpluginfactory.h>

// Calligra libs
#include <KoShapeRegistry.h>
//#include <KoToolRegistry.h>

// ImageShape
//#include "ImageToolFactory.h"
#include "ImageShapeFactory.h"

K_PLUGIN_FACTORY_WITH_JSON(ImageShapePluginFactory, "calligra_shape_image.json", registerPlugin<ImageShapePlugin>();)

ImageShapePlugin::ImageShapePlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    //KoToolRegistry::instance()->add(new ImageToolFactory());
    KoShapeRegistry::instance()->add(new ImageShapeFactory());
}

#include <ImageShapePlugin.moc>
