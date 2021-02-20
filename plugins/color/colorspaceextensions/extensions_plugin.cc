/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
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
