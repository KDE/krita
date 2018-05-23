/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "HeifExport.h"

#include <QCheckBox>
#include <QSlider>
#include <QApplication>

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceConstants.h>
#include <KisImportExportManager.h>
#include <KisExportCheckRegistry.h>

#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "HeifConverter.h"


class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_heif_export.json", registerPlugin<EXRExport>();)

EXRExport::EXRExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

EXRExport::~EXRExport()
{
}

KisPropertiesConfigurationSP EXRExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("quality", 100);
    cfg->setProperty("lossless", true);
    return cfg;
}

KisConfigWidget *EXRExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return new KisWdgOptionsHeif(parent);
}

KisImportExportFilter::ConversionStatus EXRExport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisImageSP image = document->savingImage();

    HeifConverter heifConverter(document, !batchMode());

    KisImageBuilder_Result res;

    res = heifConverter.buildFile(filename(), image);

    dbgFile  << " Result =" << res;
    switch (res) {
    case KisImageBuilder_RESULT_INVALID_ARG:
        document->setErrorMessage(i18n("This layer cannot be saved to HEIF."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_EMPTY:
        document->setErrorMessage(i18n("The layer does not have an image associated with it."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_NO_URI:
        document->setErrorMessage(i18n("The filename is empty."));
        return KisImportExportFilter::CreationError;

    case KisImageBuilder_RESULT_NOT_LOCAL:
        document->setErrorMessage(i18n("HEIF images cannot be saved remotely."));
        return KisImportExportFilter::InternalError;

    case KisImageBuilder_RESULT_OK:
        if (!heifConverter.errorMessage().isNull()) {
            document->setErrorMessage(heifConverter.errorMessage());
        }
        return KisImportExportFilter::OK;
    default:
        break;
    }

    document->setErrorMessage(i18n("Internal Error"));
    return KisImportExportFilter::InternalError;

}

void EXRExport::initializeCapabilities()
{
    // This checks before saving for what the file format supports: anything that is supported needs to be mentioned here

    addCapability(KisExportCheckRegistry::instance()->get("NodeTypeCheck/KisGroupLayer")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));

    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Float16BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Float32BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Float16BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Float32BitsColorDepthID)
            << QPair<KoID, KoID>(GrayColorModelID, Float16BitsColorDepthID)
            << QPair<KoID, KoID>(GrayColorModelID, Float32BitsColorDepthID)
            << QPair<KoID, KoID>(XYZAColorModelID, Float16BitsColorDepthID)
            << QPair<KoID, KoID>(XYZAColorModelID, Float32BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "HEIF");
}


void KisWdgOptionsHeif::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    chkLossless->setChecked(cfg->getBool("lossless", true));
    sliderQuality->setValue(cfg->getInt("quality", 100));
}

KisPropertiesConfigurationSP KisWdgOptionsHeif::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("flatten", chkLossless->isChecked());
    cfg->setProperty("quality", sliderQuality->value());
    return cfg;
}

#include <HeifExport.moc>

