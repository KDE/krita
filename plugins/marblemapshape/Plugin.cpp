/* Part of Calligra Suite - Marble Map Shape
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Simon Schmeißer <mail_to_wrt@gmx.de>
 * Copyright (C) 2011  Radosław Wicik <radoslaw@wicik.pl>
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
#include "Plugin.h"
#include "MarbleMapShapeFactory.h"
#include "MarbleMapToolFactory.h"

#include <KoToolRegistry.h>
#include <KoShapeRegistry.h>

#include <kgenericfactory.h>
#include <KDebug>


//K_EXPORT_COMPONENT_FACTORY(marblemapshape, KGenericFactory<MarbleMapPlugin>("MarbleMapShape"))
K_PLUGIN_FACTORY(PluginFactory, registerPlugin<Plugin>();)
K_EXPORT_PLUGIN(PluginFactory("MarbleMapShape"))

Plugin::Plugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    //kDebug() << "Regisering MarbleMapShapeFactory";
    KoShapeRegistry::instance()->add(new MarbleMapShapeFactory());
    //kDebug() << "Regisering MarbleMapToolFactory";
    KoToolRegistry::instance()->add(new MarbleMapToolFactory());
}

#include "Plugin.moc"

