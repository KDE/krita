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

#include "kis_png_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QApplication>

#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KisImportExportManager.h>
#include <KoColorProfile.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>

#include <KisExportCheckRegistry.h>

#include <kis_properties_configuration.h>
#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_config.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_filter_registry_model.h>
#include <kis_exif_info_visitor.h>
#include "kis_png_converter.h"
#include <kis_iterator_ng.h>

K_PLUGIN_FACTORY_WITH_JSON(KisPNGExportFactory, "krita_png_export.json", registerPlugin<KisPNGExport>();)

KisPNGExport::KisPNGExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisPNGExport::~KisPNGExport()
{
}

KisImportExportFilter::ConversionStatus KisPNGExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    KisImageSP image = document->savingImage();

    KisPNGOptions options;

    options.alpha = configuration->getBool("alpha", true);
    options.interlace = configuration->getBool("interlaced", false);
    options.compression = configuration->getInt("compression", 3);
    options.tryToSaveAsIndexed = configuration->getBool("indexed", false);
    KoColor c(KoColorSpaceRegistry::instance()->rgb8());
    c.fromQColor(Qt::white);
    options.transparencyFillColor = configuration->getColor("transparencyFillcolor", c).toQColor();
    options.saveSRGBProfile = configuration->getBool("saveSRGBProfile", false);
    options.forceSRGB = configuration->getBool("forceSRGB", true);
    options.storeAuthor = configuration->getBool("storeAuthor", false);
    options.storeMetaData = configuration->getBool("storeMetaData", false);
    options.saveAsHDR = configuration->getBool("saveAsHDR", false);

    vKisAnnotationSP_it beginIt = image->beginAnnotations();
    vKisAnnotationSP_it endIt = image->endAnnotations();

    KisExifInfoVisitor eIV;
    eIV.visit(image->rootLayer().data());
    KisMetaData::Store *eI = 0;
    if (eIV.metaDataCount() == 1) {
        eI = eIV.exifInfo();
    }
    if (eI) {
        KisMetaData::Store* copy = new KisMetaData::Store(*eI);
        eI = copy;
    }

    KisPNGConverter pngConverter(document);

    KisImageBuilder_Result res = pngConverter.buildFile(io, image->bounds(), image->xRes(), image->yRes(), image->projection(), beginIt, endIt, options, eI);

    if (res == KisImageBuilder_RESULT_OK) {
        delete eI;
        return KisImportExportFilter::OK;
    }

    delete eI;
    dbgFile << " Result =" << res;
    return KisImportExportFilter::InternalError;
}

KisPropertiesConfigurationSP KisPNGExport::defaultConfiguration(const QByteArray &, const QByteArray &) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("alpha", true);
    cfg->setProperty("indexed", false);
    cfg->setProperty("compression", 3);
    cfg->setProperty("interlaced", false);

    KoColor fill_color(KoColorSpaceRegistry::instance()->rgb8());
    fill_color = KoColor();
    fill_color.fromQColor(Qt::white);
    QVariant v;
    v.setValue(fill_color);

    cfg->setProperty("transparencyFillcolor", v);
    cfg->setProperty("saveSRGBProfile", false);
    cfg->setProperty("forceSRGB", true);
    cfg->setProperty("saveAsHDR", false);
    cfg->setProperty("storeMetaData", false);
    cfg->setProperty("storeAuthor", false);

    return cfg;
}

KisConfigWidget *KisPNGExport::createConfigurationWidget(QWidget *parent, const QByteArray &, const QByteArray &) const
{
    return new KisWdgOptionsPNG(parent);
}

void KisPNGExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "PNG");
}

KisWdgOptionsPNG::KisWdgOptionsPNG(QWidget *parent)
    : KisConfigWidget(parent)
{
    setupUi(this);

    connect(chkSaveAsHDR, SIGNAL(toggled(bool)), this, SLOT(slotUseHDRChanged(bool)));
}

void KisWdgOptionsPNG::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    // the export manager should have prepared some info for us!
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::ImageContainsTransparencyTag));
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::ColorModelIDTag));
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::sRGBTag));

    const bool isThereAlpha = cfg->getBool(KisImportExportFilter::ImageContainsTransparencyTag);

    alpha->setChecked(cfg->getBool("alpha", isThereAlpha));

    bnTransparencyFillColor->setEnabled(!alpha->isChecked());

    if (cfg->getString(KisImportExportFilter::ColorModelIDTag) == RGBAColorModelID.id()) {
        tryToSaveAsIndexed->setVisible(true);
        if (alpha->isChecked()) {
            tryToSaveAsIndexed->setChecked(false);
        }
        else {
            tryToSaveAsIndexed->setChecked(cfg->getBool("indexed", false));
        }
    }
    else {
        tryToSaveAsIndexed->setVisible(false);
    }
    interlacing->setChecked(cfg->getBool("interlaced", false));
    compressionLevel->setValue(cfg->getInt("compression", 3));
    compressionLevel->setRange(1, 9, 0);

    tryToSaveAsIndexed->setVisible(!isThereAlpha);

    //const bool sRGB = cfg->getBool(KisImportExportFilter::sRGBTag, false);

    //chkSRGB->setEnabled(sRGB);
    chkSRGB->setChecked(cfg->getBool("saveSRGBProfile", true));

    //chkForceSRGB->setEnabled(!sRGB);
    chkForceSRGB->setChecked(cfg->getBool("forceSRGB", false));

    chkSaveAsHDR->setChecked(cfg->getBool("saveAsHDR", false));
    slotUseHDRChanged(chkSaveAsHDR->isChecked());

    chkAuthor->setChecked(cfg->getBool("storeAuthor", false));
    chkMetaData->setChecked(cfg->getBool("storeMetaData", false));

    KoColor background(KoColorSpaceRegistry::instance()->rgb8());
    background.fromQColor(Qt::white);
    bnTransparencyFillColor->setDefaultColor(background);
    bnTransparencyFillColor->setColor(cfg->getColor("transparencyFillcolor", background));
}

KisPropertiesConfigurationSP KisWdgOptionsPNG::configuration() const
{

    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());

    bool alpha = this->alpha->isChecked();
    bool interlace = interlacing->isChecked();
    int compression = (int)compressionLevel->value();
    bool saveAsHDR = chkSaveAsHDR->isChecked();
    bool tryToSaveAsIndexed = !saveAsHDR && this->tryToSaveAsIndexed->isChecked();
    bool saveSRGB = !saveAsHDR && chkSRGB->isChecked();
    bool forceSRGB = !saveAsHDR && chkForceSRGB->isChecked();
    bool storeAuthor = chkAuthor->isChecked();
    bool storeMetaData = chkMetaData->isChecked();


    QVariant transparencyFillcolor;
    transparencyFillcolor.setValue(bnTransparencyFillColor->color());

    cfg->setProperty("alpha", alpha);
    cfg->setProperty("indexed", tryToSaveAsIndexed);
    cfg->setProperty("compression", compression);
    cfg->setProperty("interlaced", interlace);
    cfg->setProperty("transparencyFillcolor", transparencyFillcolor);
    cfg->setProperty("saveAsHDR", saveAsHDR);
    cfg->setProperty("saveSRGBProfile", saveSRGB);
    cfg->setProperty("forceSRGB", forceSRGB);
    cfg->setProperty("storeAuthor", storeAuthor);
    cfg->setProperty("storeMetaData", storeMetaData);

    return cfg;
}

void KisWdgOptionsPNG::on_alpha_toggled(bool checked)
{
    bnTransparencyFillColor->setEnabled(!checked);
}

void KisWdgOptionsPNG::slotUseHDRChanged(bool value)
{
    tryToSaveAsIndexed->setDisabled(value);
    chkForceSRGB->setDisabled(value);
    chkSRGB->setDisabled(value);
}

#include "kis_png_export.moc"

