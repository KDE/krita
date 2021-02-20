/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_qimageio_export.h"
#include "ui_kis_wdg_options_qimageio.h"

#include <QCheckBox>
#include <QSlider>

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QApplication>

#include <KisMimeDatabase.h>
#include <KisExportCheckRegistry.h>
#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_config_widget.h>

K_PLUGIN_FACTORY_WITH_JSON(KisQImageIOExportFactory, "krita_qimageio_export.json", registerPlugin<KisQImageIOExport>();)

class KisWdgOptionsQImageIO : public KisConfigWidget, public Ui::KisWdgOptionsQImageIO
{
    Q_OBJECT

public:
    KisWdgOptionsQImageIO(QWidget *parent);

    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;
};

KisWdgOptionsQImageIO::KisWdgOptionsQImageIO(QWidget *parent)
    : KisConfigWidget(parent)
{
    setupUi(this);
    imageQuality->setRange(1, 100, 0);
}

KisPropertiesConfigurationSP KisWdgOptionsQImageIO::configuration() const
{
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());

    cfg->setProperty("quality", (int)imageQuality->value());
    return cfg;
}

void KisWdgOptionsQImageIO::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    imageQuality->setValue(cfg->getInt("quality", 75));
}

KisQImageIOExport::KisQImageIOExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisQImageIOExport::~KisQImageIOExport()
{
}

KisImportExportErrorCode KisQImageIOExport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP configuration)
{
    QRect rc = document->savingImage()->bounds();
    QImage image = document->savingImage()->projection()->convertToQImage(0, 0, 0, rc.width(), rc.height(), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    int quality = -1;
    if (configuration) {
        quality = configuration->getInt("quality", -1);
    }

    bool result = image.save(io, QFileInfo(filename()).suffix().toLatin1(), quality);
    if (result) {
        return ImportExportCodes::OK;
    }
    else {
        return ImportExportCodes::FileFormatIncorrect;
    }
}

void KisQImageIOExport::initializeCapabilities()
{
    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, KisMimeDatabase::descriptionForMimeType(mimeType()));
    addCapability(KisExportCheckRegistry::instance()->get("ColorModelPerLayerCheck/" + RGBAColorModelID.id() + "/" + Integer8BitsColorDepthID.id())->create(KisExportCheckBase::SUPPORTED));
}

KisConfigWidget *KisQImageIOExport::createConfigurationWidget(QWidget *parent, const QByteArray& /*from*/, const QByteArray& /*to*/) const
{
    if (mimeType() == "image/webp") {
        return new KisWdgOptionsQImageIO(parent);
    }

    return 0;
}

KisPropertiesConfigurationSP KisQImageIOExport::defaultConfiguration(const QByteArray &, const QByteArray &) const
{
    if (mimeType() == "image/webp") {
        KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
        // the Qt webp plugin chooses 75 when passing -1 (default argument) to QImage::save()
        cfg->setProperty("quality", 75);
        return cfg;
    }
    return 0;
}

#include "kis_qimageio_export.moc"

