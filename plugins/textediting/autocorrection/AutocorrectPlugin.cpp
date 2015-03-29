/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "AutocorrectPlugin.h"
#include "AutocorrectFactory.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <KoTextEditingRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(AutocorrectPluginFactory, "calligra_textediting_autocorrect.json",
                           registerPlugin<AutocorrectPlugin>(); )

AutocorrectPlugin::AutocorrectPlugin( QObject *parent, const QVariantList& )
    : QObject(parent)
{
    KoTextEditingRegistry::instance()->add( new AutocorrectFactory());
}

#include <AutocorrectPlugin.moc>

