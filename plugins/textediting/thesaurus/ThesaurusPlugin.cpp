/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#include "ThesaurusPlugin.h"
#include "ThesaurusFactory.h"

#include <KPluginFactory>
#include <KPluginLoader>

#include <KoTextEditingRegistry.h>

K_PLUGIN_FACTORY( ThesaurusPluginFactory, registerPlugin<ThesaurusPlugin>(); )
K_EXPORT_PLUGIN( ThesaurusPluginFactory("ThesaurusPlugin") )

ThesaurusPlugin::ThesaurusPlugin(QObject *parent, const QVariantList&)
    : QObject(parent)
{
    KoTextEditingRegistry::instance()->add(new ThesaurusFactory(parent));
}

#include <ThesaurusPlugin.moc>
