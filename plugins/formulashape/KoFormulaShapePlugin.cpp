/* This file is part of the KDE project
 * Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net> 
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
#include "KoFormulaShapePlugin.h"
#include "KoFormulaToolFactory.h"
#include "KoFormulaShapeFactory.h"

#include <KoShapeRegistry.h>
#include <KoToolRegistry.h>

#include <kpluginfactory.h>

#include "KoM2MMLForumulaTool.h"

K_PLUGIN_FACTORY(KoFormulaShapePluginFactory, registerPlugin<KoFormulaShapePlugin>();)
K_EXPORT_PLUGIN(KoFormulaShapePluginFactory("FormulaShape"))

KoFormulaShapePlugin::KoFormulaShapePlugin( QObject* parent, const QVariantList& )
                    : QObject( parent )
{
    KoToolRegistry::instance()->add( new KoFormulaToolFactory() );
    KoToolRegistry::instance()->add( new KoM2MMLFormulaToolFactory());
    KoShapeRegistry::instance()->add( new KoFormulaShapeFactory() );
}

KoFormulaShapePlugin::~KoFormulaShapePlugin()
{}

#include "KoFormulaShapePlugin.moc"
