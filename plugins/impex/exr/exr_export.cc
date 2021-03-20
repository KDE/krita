/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

KisImportExportErrorCode EXRExport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP configuration)
{
    Q_ASSERT(document);
    Q_ASSERT(configuration);

    KisImageSP image = document->savingImage();
    Q_ASSERT(image);

    EXRConverter exrConverter(document, !batchMode());

    KisImportExportErrorCode res;

    if (configuration && configuration->getBool("flatten")) {
        res = exrConverter.buildFile(filename(), image->rootLayer(), true);
    }
    else {
        res = exrConverter.buildFile(filename(), image->rootLayer());
    }

    if (!exrConverter.errorMessage().isNull()) {
        document->setErrorMessage(exrConverter.errorMessage());
    }


    dbgFile  << " Result =" << res;
    return res;
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

