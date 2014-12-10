/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "layerstyles.h"
#include <kpluginfactory.h>

#include "kis_ls_drop_shadow_filter.h"

#include <filter/kis_filter_registry.h>

K_PLUGIN_FACTORY(LayerStylesPluginFactory, registerPlugin<LayerStylesPlugin>();)
K_EXPORT_PLUGIN(LayerStylesPluginFactory("krita"))

LayerStylesPlugin::LayerStylesPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisLsDropShadowFilter());

}

LayerStylesPlugin::~LayerStylesPlugin()
{
}
