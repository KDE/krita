/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "quick_paintop_plugin.h"

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_paintop_registry.h>
#include <kis_brush_based_paintop_settings.h>

#include "kis_quickop.h"
#include "kis_quickop_settings_widget.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(QuickPaintOpPluginFactory, "kritaquickpaintop.json", registerPlugin<QuickPaintOpPlugin>();)


QuickPaintOpPlugin::QuickPaintOpPlugin(QObject* parent, const QVariantList&):
    QObject(parent)
{
    KisPaintOpRegistry::instance()->add(new KisSimplePaintOpFactory<KisQuickOp, KisBrushBasedPaintOpSettings, KisQuickOpSettingsWidget>(
                                            "quickop", i18n("Quick Brush"), KisPaintOpFactory::categoryStable(), "krita-quickop.png",
                                            QString(), QStringList(), 0)
                                       );
}

QuickPaintOpPlugin::~QuickPaintOpPlugin() { }

#include "quick_paintop_plugin.moc"
