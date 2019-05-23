/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_tiff_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include <KoColorModelStandardIds.h>
#include <KisImportExportManager.h>
#include <KisExportCheckRegistry.h>
#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_config.h>
#include "kis_layer_utils.h"

#include "kis_tiff_converter.h"
#include "kis_dlg_options_tiff.h"
#include "ui_kis_wdg_options_tiff.h"

K_PLUGIN_FACTORY_WITH_JSON(KisTIFFExportFactory, "krita_tiff_export.json", registerPlugin<KisTIFFExport>();)

KisTIFFExport::KisTIFFExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisTIFFExport::~KisTIFFExport()
{
}

KisImportExportErrorCode KisTIFFExport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP configuration)
{
    // If a configuration object was passed to the convert method, we use that, otherwise we load from the settings
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    if (configuration) {
        cfg->fromXML(configuration->toXML());
    }
    else {
        cfg = lastSavedConfiguration(KisDocument::nativeFormatMimeType(), "image/tiff");
    }

    const KoColorSpace* cs = document->savingImage()->colorSpace();
    cfg->setProperty("type", (int)cs->channels()[0]->channelValueType());
    cfg->setProperty("isCMYK", (cs->colorModelId() == CMYKAColorModelID));

    KisTIFFOptions options;
    options.fromProperties(configuration);

    if (!options.flatten) {
        const bool hasGroupLayers =
            KisLayerUtils::recursiveFindNode(document->savingImage()->root(),
                [] (KisNodeSP node) {
                    return node->parent() && node->inherits("KisGroupLayer");
                });
        options.flatten = hasGroupLayers;
    }

    if ((cs->channels()[0]->channelValueType() == KoChannelInfo::FLOAT16
         || cs->channels()[0]->channelValueType() == KoChannelInfo::FLOAT32) && options.predictor == 2) {
        // FIXME THIS IS AN HACK FIX THAT IN 2.0 !! (62456a7b47636548c6507593df3e2bdf440f7544, BUG:135649)
        options.predictor = 3;
    }

    KisImageSP image;

    if (options.flatten) {
        image = new KisImage(0, document->savingImage()->width(), document->savingImage()->height(), document->savingImage()->colorSpace(), "");
        image->setResolution(document->savingImage()->xRes(), document->savingImage()->yRes());
        KisPaintDeviceSP pd = KisPaintDeviceSP(new KisPaintDevice(*document->savingImage()->projection()));
        KisPaintLayerSP l = KisPaintLayerSP(new KisPaintLayer(image.data(), "projection", OPACITY_OPAQUE_U8, pd));
        image->addNode(KisNodeSP(l.data()), image->rootLayer().data());
    } else {
        image = document->savingImage();
    }

    KisTIFFConverter tiffConverter(document);
    KisImportExportErrorCode res = tiffConverter.buildFile(filename(), image, options);
    return res;
}

KisPropertiesConfigurationSP KisTIFFExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisTIFFOptions options;
    return options.toProperties();
}

KisConfigWidget *KisTIFFExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return new KisTIFFOptionsWidget(parent);
}

void KisTIFFExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));

    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Float16BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Float32BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(CMYKAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(CMYKAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(LABAColorModelID, Integer16BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "TIFF");

}

#include <kis_tiff_export.moc>

