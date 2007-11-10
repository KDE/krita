/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "extensions_plugin.h"

#include <kdebug.h>
#include <kgenericfactory.h>

#include <KoColorTransformationFactoryRegistry.h>

#include "kis_hsv_adjustement.h"

typedef KGenericFactory<ExtensionsPlugin> ExtensionsPluginFactory;
K_EXPORT_COMPONENT_FACTORY( krita_colorspaces_extensions, ExtensionsPluginFactory( "krita" ) )

ExtensionsPlugin::ExtensionsPlugin(QObject *parent, const QStringList &)
{
    kDebug() << "Loading ExtensionsPlugin";
    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisHSVAdjustementFactory);
}

ExtensionsPlugin::~ExtensionsPlugin()
{
}

#include "extensions_plugin.moc"
