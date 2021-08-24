/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QFileInfo>
#include <QApplication>

#include <kpluginfactory.h>

#include <KisExportCheckRegistry.h>
#include <KisImportExportManager.h>
#include <ImageSizeCheck.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceConstants.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>

#include "psd_saver.h"

class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_psd_export.json", registerPlugin<psdExport>();)

psdExport::psdExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

psdExport::~psdExport()
{
}

KisImportExportErrorCode psdExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    PSDSaver psdSaver(document);
    return psdSaver.buildFile(*io);
}

void psdExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("PSDLayerStyleCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("NodeTypeCheck/KisGroupLayer")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("NodeTypeCheck/KisTransparencyMask")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("ColorModelHomogenousCheck")->create(KisExportCheckBase::UNSUPPORTED, i18nc("image conversion warning", "Your image contains one or more layers with a color model that is different from the image.")));

    ImageSizeCheckFactory *factory = dynamic_cast<ImageSizeCheckFactory*>(KisExportCheckRegistry::instance()->get("ImageSizeCheck"));
    if (factory) {
        addCapability(factory->create(30000, 30000, KisExportCheckBase::SUPPORTED));
    }

    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Integer16BitsColorDepthID)
//            << QPair<KoID, KoID>(RGBAColorModelID, Float16BitsColorDepthID)
//            << QPair<KoID, KoID>(RGBAColorModelID, Float32BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(CMYKAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(CMYKAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(LABAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(LABAColorModelID, Integer16BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "PSD");

}

#include <psd_export.moc>

