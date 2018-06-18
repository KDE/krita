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
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoColorTransformationFactoryRegistry.h>

#include "kis_hsv_adjustment.h"
#include "kis_dodgemidtones_adjustment.h"
#include "kis_dodgehighlights_adjustment.h"
#include "kis_dodgeshadows_adjustment.h"
#include "kis_burnmidtones_adjustment.h"
#include "kis_burnhighlights_adjustment.h"
#include "kis_burnshadows_adjustment.h"
#include "kis_color_balance_adjustment.h"
#include "kis_desaturate_adjustment.h"

K_PLUGIN_FACTORY_WITH_JSON(ExtensionsPluginFactory, "krita_colorspaces_extensions_plugin.json", registerPlugin<ExtensionsPlugin>();)

ExtensionsPlugin::ExtensionsPlugin(QObject *parent, const QVariantList &)
{
    Q_UNUSED(parent);
    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisHSVAdjustmentFactory);
    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisHSVCurveAdjustmentFactory);
    
    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisDodgeMidtonesAdjustmentFactory);
    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisDodgeHighlightsAdjustmentFactory);
    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisDodgeShadowsAdjustmentFactory);
    
    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisBurnMidtonesAdjustmentFactory);
    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisBurnHighlightsAdjustmentFactory);
    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisBurnShadowsAdjustmentFactory);

    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisColorBalanceAdjustmentFactory);

    KoColorTransformationFactoryRegistry::addColorTransformationFactory(new KisDesaturateAdjustmentFactory);
}

ExtensionsPlugin::~ExtensionsPlugin()
{
}

#include "extensions_plugin.moc"
