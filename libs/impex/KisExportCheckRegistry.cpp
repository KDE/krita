/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "KisExportCheckRegistry.h"
#include <KoID.h>
#include <klocalizedstring.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>

#include <AnimationCheck.h>
//#include <AssistantsCheck.h>
#include <ColorModelCheck.h>
//#include <ColorModelPerLayerCheck.h>
//#include <ColorProofingCheck.h>
//#include <CompositionsCheck.h>
//#include <ExifCheck.h>
//#include <GridCheck.h>
//#include <GuidesCheck.h>
#include <MultiLayerCheck.h>
//#include <PSDLayerStylesCheck.h>
//#include <sRGBProfileCheck.h>
#include <NodeTypeCheck.h>
#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisExportCheckRegistry, s_instance)

KisExportCheckRegistry::KisExportCheckRegistry ()
{
    KisExportCheckFactory *chkFactory = 0;

    // Multilayer check
    chkFactory = new MultiLayerCheckFactory();
    add(chkFactory->id(), chkFactory);

    // Animation check
    chkFactory = new AnimationCheckFactory();
    add(chkFactory->id(), chkFactory);

    // Color model/channel depth checks
    chkFactory = new ColorModelCheckFactory(AlphaColorModelID, Integer8BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(AlphaColorModelID, Integer16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(AlphaColorModelID, Float16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(AlphaColorModelID, Float32BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(AlphaColorModelID, Float64BitsColorDepthID);
    add(chkFactory->id(), chkFactory);

    chkFactory = new ColorModelCheckFactory(RGBAColorModelID, Integer8BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(RGBAColorModelID, Integer16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(RGBAColorModelID, Float16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(RGBAColorModelID, Float32BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(RGBAColorModelID, Float64BitsColorDepthID);
    add(chkFactory->id(), chkFactory);

    chkFactory = new ColorModelCheckFactory(XYZAColorModelID, Integer8BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(XYZAColorModelID, Integer16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(XYZAColorModelID, Float16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(XYZAColorModelID, Float32BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(XYZAColorModelID, Float64BitsColorDepthID);
    add(chkFactory->id(), chkFactory);

    chkFactory = new ColorModelCheckFactory(LABAColorModelID, Integer8BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(LABAColorModelID, Integer16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(LABAColorModelID, Float16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(LABAColorModelID, Float32BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(LABAColorModelID, Float64BitsColorDepthID);
    add(chkFactory->id(), chkFactory);

    chkFactory = new ColorModelCheckFactory(CMYKAColorModelID, Integer8BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(CMYKAColorModelID, Integer16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(CMYKAColorModelID, Float16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(CMYKAColorModelID, Float32BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(CMYKAColorModelID, Float64BitsColorDepthID);
    add(chkFactory->id(), chkFactory);

    chkFactory = new ColorModelCheckFactory(GrayAColorModelID, Integer8BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(GrayAColorModelID, Integer16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(GrayAColorModelID, Float16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(GrayAColorModelID, Float32BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(GrayAColorModelID, Float64BitsColorDepthID);
    add(chkFactory->id(), chkFactory);

    chkFactory = new ColorModelCheckFactory(GrayColorModelID, Integer8BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(GrayColorModelID, Integer16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(GrayColorModelID, Float16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(GrayColorModelID, Float32BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(GrayColorModelID, Float64BitsColorDepthID);
    add(chkFactory->id(), chkFactory);

    chkFactory = new ColorModelCheckFactory(YCbCrAColorModelID, Integer8BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(YCbCrAColorModelID, Integer16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(YCbCrAColorModelID, Float16BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(YCbCrAColorModelID, Float32BitsColorDepthID);
    add(chkFactory->id(), chkFactory);
    chkFactory = new ColorModelCheckFactory(YCbCrAColorModelID, Float64BitsColorDepthID);
    add(chkFactory->id(), chkFactory);

    // Node type checks
    chkFactory = new NodeTypeCheckFactory("KisCloneLayer", i18n("Clone Layer"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisFileLayer", i18n("File Layer"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisShapeLayer", i18n("Group Layer"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisAdjustmentLayer", i18n("Filter Layer"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisGeneratorLayer", i18n("Generator Layer"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisColorizeMask", i18n("Colorize Mask"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisFilterMask", i18n("Filter Mask"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisTransformMask", i18n("Transform Mask"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisTransparencyMask", i18n("Transparency Mask"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisSelectionMask", i18n("Selection Mask"));
    add(chkFactory->id(), chkFactory);


}

KisExportCheckRegistry::~KisExportCheckRegistry ()
{
    qDeleteAll(values());
}

KisExportCheckRegistry *KisExportCheckRegistry ::instance()
{
    return s_instance;
}

