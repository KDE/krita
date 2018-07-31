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

#include "exr_export.h"

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

#include "exr_converter.h"


class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_exr_export.json", registerPlugin<EXRExport>();)

EXRExport::EXRExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

EXRExport::~EXRExport()
{
}

KisPropertiesConfigurationSP EXRExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("flatten", false);
    return cfg;
}

KisConfigWidget *EXRExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return new KisWdgOptionsExr(parent);
}

KisImportExportFilter::ConversionStatus EXRExport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP configuration)
{
    Q_ASSERT(document);
    Q_ASSERT(configuration);

    KisImageSP image = document->savingImage();
    Q_ASSERT(image);

    EXRConverter exrConverter(document, !batchMode());

    KisImageBuilder_Result res;

    if (configuration && configuration->getBool("flatten")) {
        res = exrConverter.buildFile(filename(), image->rootLayer(), true);
    }
    else {
        res = exrConverter.buildFile(filename(), image->rootLayer());
    }

    dbgFile  << " Result =" << res;
    switch (res) {
    case KisImageBuilder_RESULT_INVALID_ARG:
        document->setErrorMessage(i18n("This layer cannot be saved to EXR."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_EMPTY:
        document->setErrorMessage(i18n("The layer does not have an image associated with it."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_NO_URI:
        document->setErrorMessage(i18n("The filename is empty."));
        return KisImportExportFilter::CreationError;

    case KisImageBuilder_RESULT_NOT_LOCAL:
        document->setErrorMessage(i18n("EXR images cannot be saved remotely."));
        return KisImportExportFilter::InternalError;

    case KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE:
        document->setErrorMessage(i18n("Colorspace not supported: EXR images must be 16 or 32 bits floating point RGB."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_OK:
        if (!exrConverter.errorMessage().isNull()) {
            document->setErrorMessage(exrConverter.errorMessage());
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
    addSupportedColorModels(supportedColorModels, "EXR");
}




void KisWdgOptionsExr::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    chkFlatten->setChecked(cfg->getBool("flatten", false));
}

KisPropertiesConfigurationSP KisWdgOptionsExr::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("flatten", chkFlatten->isChecked());
    return cfg;
}

#include <exr_export.moc>

