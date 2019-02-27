/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "LayerDocker.h"


#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>
#include "kis_debug.h"

#include "LayerBox.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaLayerDockerPluginFactory, "kritalayerdocker.json", registerPlugin<KritaLayerDockerPlugin>();)

KritaLayerDockerPlugin::KritaLayerDockerPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoDockRegistry::instance()->add(new LayerBoxFactory());
}

KritaLayerDockerPlugin::~KritaLayerDockerPlugin()
{
}

#include "LayerDocker.moc"
