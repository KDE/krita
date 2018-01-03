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
#include <KoColorSpaceRegistry.h>

#include <AnimationCheck.h>
#include <ColorModelCheck.h>
#include <ColorModelPerLayerCheck.h>
#include <CompositionsCheck.h>
#include <ExifCheck.h>
#include <MultiLayerCheck.h>
#include <PSDLayerStylesCheck.h>
#include <sRGBProfileCheck.h>
#include <NodeTypeCheck.h>
#include <ImageSizeCheck.h>
#include <ColorModelHomogenousCheck.h>

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

    // Compositions
    chkFactory = new CompositionsCheckFactory();
    add(chkFactory->id(), chkFactory);

    // Layer styles
    chkFactory = new PSDLayerStyleCheckFactory();
    add(chkFactory->id(), chkFactory);

    // Check the layers for the presence of exiv info: note this is also
    // done for multilayer images even though jpeg, which supports exiv,
    // only can handle one layer.
    chkFactory = new ExifCheckFactory();
    add(chkFactory->id(), chkFactory);

    // Check whether the image is sRGB
    chkFactory = new sRGBProfileCheckFactory();
    add(chkFactory->id(), chkFactory);

    // Image size
    chkFactory = new ImageSizeCheckFactory();
    add(chkFactory->id(), chkFactory);

    // Do all layer have the image colorspace
    chkFactory = new ColorModelHomogenousCheckFactory();
    add(chkFactory->id(), chkFactory);

    QList<KoID> allColorModels = KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::AllColorSpaces);
    Q_FOREACH(const KoID &colorModelID, allColorModels) {
        QList<KoID> allColorDepths = KoColorSpaceRegistry::instance()->colorDepthList(colorModelID.id(), KoColorSpaceRegistry::AllColorSpaces);
        Q_FOREACH(const KoID &colorDepthID, allColorDepths) {

            Q_ASSERT(!colorModelID.name().isEmpty());
            Q_ASSERT(!colorDepthID.name().isEmpty());

            // Per layer color model/channel depth checks
            chkFactory = new ColorModelPerLayerCheckFactory(colorModelID, colorDepthID);
            add(chkFactory->id(), chkFactory);

            // Image color model/channel depth checks
            chkFactory = new ColorModelCheckFactory(colorModelID, colorDepthID);
            add(chkFactory->id(), chkFactory);
        }
    }

    // Node type checks
    chkFactory = new NodeTypeCheckFactory("KisCloneLayer", i18n("Clone Layer"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisGroupLayer", i18n("Group Layer"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisFileLayer", i18n("File Layer"));
    add(chkFactory->id(), chkFactory);
    chkFactory = new NodeTypeCheckFactory("KisShapeLayer", i18n("Vector Layer"));
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

