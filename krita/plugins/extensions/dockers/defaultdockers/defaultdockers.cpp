/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "defaultdockers.h"

#include <kcomponentdata.h>
#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>
#include "kis_debug.h"

#include "histogramdock_factory.h"
#include "kis_birdeye_box.h"
#include "kis_layer_box.h"
#include "kis_palette_docker.h"

K_PLUGIN_FACTORY(KritaDefaultDockersPluginFactory, registerPlugin<KritaDefaultDockersPlugin>();)
K_EXPORT_PLUGIN(KritaDefaultDockersPluginFactory("krita"))

KritaDefaultDockersPlugin::KritaDefaultDockersPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    //KoDockRegistry::instance()->add(new KisHistogramDockFactory());
    KoDockRegistry::instance()->add(new KisLayerBoxFactory());
    KoDockRegistry::instance()->add(new KisBirdEyeBoxFactory());
    KoDockRegistry::instance()->add(new KisPaletteDockerFactory());
}

KritaDefaultDockersPlugin::~KritaDefaultDockersPlugin()
{
}

#include "defaultdockers.moc"
