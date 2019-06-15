/*
 * This file is part of Krita
 *
 * Copyright (c) 2019 Miguel Lopez <reptillia39@live.com>
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

#include "guassianhighpass.h"
#include <kpluginfactory.h>

#include "guassianhighpass_filter.h"

#include <filter/kis_filter_registry.h>

K_PLUGIN_FACTORY_WITH_JSON(GuassianHighPassPluginFactory, "kritaguassianhighpassfilter.json", registerPlugin<GuassianHighPassPlugin>();)

GuassianHighPassPlugin::GuassianHighPassPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisGuassianHighPassFilter());

}

GuassianHighPassPlugin::~GuassianHighPassPlugin()
{
}

#include "guassianhighpass.moc"
