/* This file is part of the KDE project
 *
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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
#include "TemplateShapePlugin.h"

// KDE
#include <kpluginfactory.h>

// Calligra libs
#include <KoToolRegistry.h>
#include <KoShapeRegistry.h>

// This shape
#include "TemplateShapeFactory.h"
#include "TemplateToolFactory.h"


K_PLUGIN_FACTORY(TemplateShapePluginFactory, registerPlugin<TemplateShapePlugin>();)
K_EXPORT_PLUGIN(TemplateShapePluginFactory("TemplateShape"))

TemplateShapePlugin::TemplateShapePlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoShapeRegistry::instance()->add( new TemplateShapeFactory() );

    // Template: Disable this if you don't need editing.
    KoToolRegistry::instance()->add( new TemplateToolFactory() );
}


#include <TemplateShapePlugin.moc>
