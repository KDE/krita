/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_png_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QApplication>

#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KisImportExportManager.h>
#include <KisImportExportErrorCode.h>
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

KisImportExportErrorCode KisPNGExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
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
    options.forceSRGB = configuration->getBool("forceSRGB", true);
    options.storeAuthor = configuration->getBool("storeAuthor", false);
    options.storeMetaData = configuration->getBool("storeMetaData", false);
    options.downsample = configuration->getBool("downsample", false);
    options.storeColorSpaceInfo = configuration->getBool("storeColorSpaceInfo", true);
    options.writeCicpIfPossible = configuration->getBool("writeCicpIfPossible", false);
    options.storeExtraColorChunks = configuration->getBool("storeExtraColorChunks", false);

    const QString conversionOption = configuration->getString("floatingPointConversionOption", "KeepSame");
    options.convertFloatToRec2020 = false;
    if (conversionOption == "Rec2100PQ") {
        options.convertFloatToRec2020 = true;
        options.floatingPointConversion = ConversionPolicy::ApplyPQ;
    } else if (conversionOption == "Rec2100HLG") {
        options.convertFloatToRec2020 = true;
        options.floatingPointConversion = ConversionPolicy::ApplyHLG;
    } else if (conversionOption == "ApplyPQ") {
        options.floatingPointConversion = ConversionPolicy::ApplyPQ;
    } else if (conversionOption == "ApplyHLG") {
        options.floatingPointConversion = ConversionPolicy::ApplyHLG;
    }  else if (conversionOption == "ApplySMPTE428") {
        options.floatingPointConversion = ConversionPolicy::ApplySMPTE428;
    }

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

    KisImportExportErrorCode res = pngConverter.buildFile(io, image->bounds(), image->xRes(), image->yRes(), image->projection(), beginIt, endIt, options, eI);
    delete eI;
    dbgFile << " Result =" << res;
    return res;
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
    cfg->setProperty("forceSRGB", true);
    cfg->setProperty("storeMetaData", false);
    cfg->setProperty("storeAuthor", false);
    cfg->setProperty("downsample", false);
    cfg->setProperty("storeColorSpaceInfo", true);
    cfg->setProperty("writeCicpIfPossible", false);
    cfg->setProperty("storeExtraColorChunks", false);
    cfg->setProperty("floatingPointConversionOption", "KeepSame");
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
}

void KisWdgOptionsPNG::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    // the export manager should have prepared some info for us!
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::ImageContainsTransparencyTag));
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::ColorModelIDTag));
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::sRGBTag));

    // Setup the floating point conversion options.
    QStringList conversionOptionsList = { i18nc("Color space name", "Rec 2100 PQ"), i18nc("Color space name", "Rec 2100 HLG")};
    QStringList toolTipList = {i18nc("@tooltip", "The image will be converted to Rec 2020 linear first, and then encoded with a perceptual quantizer curve"
                                     " (also known as SMPTE 2048 curve). Recommended for HDR images where the absolute brightness is important."),
                               i18nc("@tooltip", "The image will be converted to Rec 2020 linear first, and then encoded with a Hybrid Log Gamma curve."
                                     " Recommended for HDR images where the display may not understand HDR.")};
    QStringList conversionOptionName = {"Rec2100PQ", "Rec2100HLG"};
    int cicpPrimaries = cfg->getInt(KisImportExportFilter::CICPPrimariesTag,
                                    static_cast<int>(PRIMARIES_UNSPECIFIED));
    if (cfg->getString(KisImportExportFilter::ColorModelIDTag) == "RGBA") {
        if (cicpPrimaries != PRIMARIES_UNSPECIFIED) {
            conversionOptionsList << i18nc("Color space option plus transfer function name", "Keep colorants, encode PQ");
            toolTipList << i18nc("@tooltip", "The image will be linearized first, and then encoded with a perceptual quantizer curve"
                                 " (also known as the SMPTE 2048 curve). Recommended for images where the absolute brightness is important.");
            conversionOptionName << "ApplyPQ";

            conversionOptionsList << i18nc("Color space option plus transfer function name", "Keep colorants, encode HLG");
            toolTipList << i18nc("@tooltip", "The image will be linearized first, and then encoded with a Hybrid Log Gamma curve."
                                 " Recommended for images intended for screens which cannot understand PQ");
            conversionOptionName << "ApplyHLG";

            conversionOptionsList << i18nc("Color space option plus transfer function name", "Keep colorants, encode SMPTE ST 428");
            toolTipList << i18nc("@tooltip", "The image will be linearized first, and then encoded with SMPTE ST 428."
                                 " Krita always opens images like these as linear floating point, this option is there to reverse that");
            conversionOptionName << "ApplySMPTE428";
        }

        conversionOptionsList << i18nc("Color space option", "No changes, clip");
        toolTipList << i18nc("@tooltip", "The image will be converted plainly to 12bit integer, and values that are out of bounds are clipped, the icc profile will be embedded.");
        conversionOptionName << "KeepSame";
    }

    cmbFloatingConversion->addItems(conversionOptionsList);
    for (int i=0; i< toolTipList.size(); i++) {
        cmbFloatingConversion->setItemData(i, toolTipList.at(i), Qt::ToolTipRole);
        cmbFloatingConversion->setItemData(i, conversionOptionName.at(i), Qt::UserRole+1);
    }
    QString optionName =
        cfg->getString("floatingPointConversionOption", "KeepSame");
    if (conversionOptionName.contains(optionName)) {
        cmbFloatingConversion->setCurrentIndex(
            conversionOptionName.indexOf(optionName));
    }
    const QString colorDepthId =
        cfg->getString(KisImportExportFilter::ColorDepthIDTag);
    if (colorDepthId == Float16BitsColorDepthID.id()
        || colorDepthId == Float32BitsColorDepthID.id()
        || colorDepthId == Float64BitsColorDepthID.id()) {
        cmbFloatingConversion->setEnabled(true);
    } else {
        cmbFloatingConversion->setEnabled(false);
    }

    gbSaveColorInfo->setChecked(cfg->getBool("storeColorSpaceInfo", true));
    chkCICP->setChecked(cfg->getBool("writeCicpIfPossible", true));
    chkExtraColorChunks->setChecked(cfg->getBool("storeExtraColorChunks", true));


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

    chkForceSRGB->setChecked(cfg->getBool("forceSRGB", false));

    chkAuthor->setChecked(cfg->getBool("storeAuthor", false));
    chkMetaData->setChecked(cfg->getBool("storeMetaData", false));

    KoColor background(KoColorSpaceRegistry::instance()->rgb8());
    background.fromQColor(Qt::white);
    bnTransparencyFillColor->setDefaultColor(background);
    bnTransparencyFillColor->setColor(cfg->getColor("transparencyFillcolor", background));

    chkDownsample->setChecked(cfg->getBool("downsample", false));
}

KisPropertiesConfigurationSP KisWdgOptionsPNG::configuration() const
{

    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());

    bool alpha = this->alpha->isChecked();
    bool interlace = interlacing->isChecked();
    int compression = (int)compressionLevel->value();
    bool tryToSaveAsIndexed = this->tryToSaveAsIndexed->isChecked();
    bool forceSRGB = chkForceSRGB->isChecked();
    bool storeAuthor = chkAuthor->isChecked();
    bool storeMetaData = chkMetaData->isChecked();
    bool downsample = chkDownsample->isChecked();

    bool saveColorInfo = gbSaveColorInfo->isChecked();
    bool saveCICP = chkCICP->isChecked();
    bool extraColor = chkExtraColorChunks->isChecked();

    QString conversionMode = cmbFloatingConversion->currentData(Qt::UserRole+1).toString();

    QVariant transparencyFillcolor;
    transparencyFillcolor.setValue(bnTransparencyFillColor->color());

    cfg->setProperty("alpha", alpha);
    cfg->setProperty("indexed", tryToSaveAsIndexed);
    cfg->setProperty("compression", compression);
    cfg->setProperty("interlaced", interlace);
    cfg->setProperty("transparencyFillcolor", transparencyFillcolor);
    cfg->setProperty("forceSRGB", forceSRGB);
    cfg->setProperty("storeAuthor", storeAuthor);
    cfg->setProperty("storeMetaData", storeMetaData);
    cfg->setProperty("downsample", downsample);
    cfg->setProperty("storeColorSpaceInfo", saveColorInfo);
    cfg->setProperty("writeCicpIfPossible", saveCICP);
    cfg->setProperty("storeExtraColorChunks", extraColor);
    cfg->setProperty("floatingPointConversionOption", conversionMode);
    return cfg;
}

void KisWdgOptionsPNG::on_alpha_toggled(bool checked)
{
    bnTransparencyFillColor->setEnabled(!checked);
}

#include "kis_png_export.moc"

