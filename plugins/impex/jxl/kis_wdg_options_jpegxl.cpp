/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_options_jpegxl.h"

#include <tuple>

#include <KoColorModelStandardIds.h>

KisWdgOptionsJPEGXL::KisWdgOptionsJPEGXL(QWidget *parent)
    : KisConfigWidget(parent)
{
    setupUi(this);

    {
        resampling->addItem(i18nc("JPEG-XL encoder options", "Default (only for low quality)"), -1);
        resampling->addItem(i18nc("JPEG-XL encoder options", "No downsampling"), 1);
        resampling->addItem(i18nc("JPEG-XL encoder options", "2x2 downsampling"), 2);
        resampling->addItem(i18nc("JPEG-XL encoder options", "4x4 downsampling"), 4);
        resampling->addItem(i18nc("JPEG-XL encoder options", "8x8 downsampling"), 8);

        extraChannelResampling->addItem(i18nc("JPEG-XL encoder options", "Default (only for low quality)"), -1);
        extraChannelResampling->addItem(i18nc("JPEG-XL encoder options", "No downsampling"), 1);
        extraChannelResampling->addItem(i18nc("JPEG-XL encoder options", "2x2 downsampling"), 2);
        extraChannelResampling->addItem(i18nc("JPEG-XL encoder options", "4x4 downsampling"), 4);
        extraChannelResampling->addItem(i18nc("JPEG-XL encoder options", "8x8 downsampling"), 8);
    }

    {
        dots->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        dots->addItem(i18nc("JPEG-XL encoder options", "Disabled"), 0);
        dots->addItem(i18nc("JPEG-XL encoder options", "Enabled"), 1);
    }

    {
        patches->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        patches->addItem(i18nc("JPEG-XL encoder options", "Disabled"), 0);
        patches->addItem(i18nc("JPEG-XL encoder options", "Enabled"), 1);
    }

    {
        gaborish->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        gaborish->addItem(i18nc("JPEG-XL encoder options", "Disabled"), 0);
        gaborish->addItem(i18nc("JPEG-XL encoder options", "Enabled"), 1);
    }

    {
        modular->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        modular->addItem(i18nc("JPEG-XL encoder options", "VarDCT mode (e.g. for photographic images)"), 0);
        modular->addItem(i18nc("JPEG-XL encoder options", "Modular mode (e.g. for lossless images)"), 1);
    }

    {
        keepInvisible->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        keepInvisible->addItem(i18nc("JPEG-XL encoder options", "Disabled"), 0);
        keepInvisible->addItem(i18nc("JPEG-XL encoder options", "Enabled"), 1);
    }

    {
        groupOrder->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        groupOrder->addItem(i18nc("JPEG-XL encoder options", "Scanline order"), 0);
        groupOrder->addItem(i18nc("JPEG-XL encoder options", "Center-first order"), 1);
    }

    {
        responsive->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        responsive->addItem(i18nc("JPEG-XL encoder options", "Disabled"), 0);
        responsive->addItem(i18nc("JPEG-XL encoder options", "Enabled"), 1);
    }

    {
        progressiveAC->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        progressiveAC->addItem(i18nc("JPEG-XL encoder options", "Disabled"), 0);
        progressiveAC->addItem(i18nc("JPEG-XL encoder options", "Enabled"), 1);
    }

    {
        qProgressiveAC->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        qProgressiveAC->addItem(i18nc("JPEG-XL encoder options", "Disabled"), 0);
        qProgressiveAC->addItem(i18nc("JPEG-XL encoder options", "Enabled"), 1);
    }

    {
        progressiveDC->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        progressiveDC->addItem(i18nc("JPEG-XL encoder options", "Disabled"), 0);
        progressiveDC->addItem(i18nc("JPEG-XL encoder options", "64x64 lower resolution pass"), 1);
        progressiveDC->addItem(i18nc("JPEG-XL encoder options", "512x512 + 64x64 lower resolution passes"), 2);
    }

    {
        lossyPalette->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        lossyPalette->addItem(i18nc("JPEG-XL encoder options", "Disabled"), 0);
        lossyPalette->addItem(i18nc("JPEG-XL encoder options", "Enabled"), 1);
    }

    {
        modularGroupSize->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        modularGroupSize->addItem(i18nc("JPEG-XL encoder options", "128"), 0);
        modularGroupSize->addItem(i18nc("JPEG-XL encoder options", "256"), 1);
        modularGroupSize->addItem(i18nc("JPEG-XL encoder options", "512"), 2);
        modularGroupSize->addItem(i18nc("JPEG-XL encoder options", "1024"), 3);
    }

    {
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Zero"), 0);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Left"), 1);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Top"), 2);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Avg0"), 3);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Select"), 4);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Gradient"), 5);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Weighted"), 6);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Top right"), 7);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Top left"), 8);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Left left"), 9);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Avg1"), 10);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Avg2"), 11);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Avg3"), 12);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Toptop predictive average"), 13);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Gradient + Weighted"), 14);
        modularPredictor->addItem(i18nc("JPEG-XL encoder options", "Use all predictors"), 15);
    }

    {
        jpegReconCFL->addItem(i18nc("JPEG-XL encoder options", "Default (encoder chooses)"), -1);
        jpegReconCFL->addItem(i18nc("JPEG-XL encoder options", "Disabled"), 0);
        jpegReconCFL->addItem(i18nc("JPEG-XL encoder options", "Enabled"), 1);
    }

    metaDataFilters->setModel(&m_filterRegistryModel);

    connect(cmbConversionPolicy,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &KisWdgOptionsJPEGXL::toggleExtraHDROptions);

    connect(modular, qOverload<int>(&QComboBox::currentIndexChanged), this, &KisWdgOptionsJPEGXL::toggleModularTabs);
}

void KisWdgOptionsJPEGXL::toggleExtraHDROptions(int index)
{
    const QString option = cmbConversionPolicy->itemData(index).value<QString>();
    chkHLGOOTF->setEnabled(option.contains("HLG"));
}

void KisWdgOptionsJPEGXL::toggleModularTabs(int index)
{
    const int id = modular->itemData(index, Qt::UserRole).value<int>();
    advancedParameters->setTabEnabled(1, id == -1 || id == 0); // vardct
    advancedParameters->setTabEnabled(2, id == -1 || id == 1); // modular
}

void KisWdgOptionsJPEGXL::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    using SpaceList = QList<std::tuple<QString, QString, QString>>;

    haveAnimation->setChecked(cfg->getBool("haveAnimation", true));
    flattenLayers->setChecked(cfg->getBool("flattenLayers", true));
    lossyEncoding->setChecked(!cfg->getBool("lossless", true));
    effort->setValue(cfg->getInt("effort", 7));
    decodingSpeed->setValue(cfg->getInt("decodingSpeed", 0));
    lossyQuality->setValue(cfg->getInt("lossyQuality", 100));
    forceModular->setChecked(cfg->getBool("forceModular", false));

    const int CicpPrimaries = cfg->getInt(KisImportExportFilter::CICPPrimariesTag, PRIMARIES_UNSPECIFIED);
    tabWidget->setTabEnabled(1, cfg->getBool(KisImportExportFilter::HDRTag, false)
                                   && cfg->getString(KisImportExportFilter::ColorModelIDTag) != GrayAColorModelID.id());
    SpaceList conversionOptionsList = {
        {i18nc("Color space option", "Save as is"),
         i18nc("@tooltip",
               "The image will be stored without conversion, the ICC profile will be converted to CICP or embedded if "
               "it was not possible."),
         "KeepSame"},
        {i18nc("Color space name", "Rec 2100 PQ"),
         i18nc("@tooltip",
               "The image will be converted to Rec 2020 linear first, and then encoded with a perceptual quantizer "
               "curve"
               " (also known as SMPTE 2048 curve). Recommended for HDR images where the absolute brightness is "
               "important."),
         "Rec2100PQ"},
        {i18nc("Color space name", "Rec 2100 HLG"),
         i18nc("@tooltip",
               "The image will be converted to Rec 2020 linear first, and then encoded with a Hybrid Log Gamma curve."
               " Recommended for HDR images where the display may not understand HDR."),
         "Rec2100HLG"}};

    if (cfg->getString(KisImportExportFilter::ColorModelIDTag) == RGBAColorModelID.id()) {
        if (CicpPrimaries != PRIMARIES_UNSPECIFIED) {
            conversionOptionsList << std::make_tuple<QString, QString, QString>(
                i18nc("Color space option plus transfer function name", "Keep colorants, encode PQ"),
                i18nc("@tooltip",
                      "The image will be linearized first, and then encoded with a perceptual quantizer curve"
                      " (also known as the SMPTE 2048 curve). Recommended for images where the absolute brightness is "
                      "important."),
                "ApplyPQ");

            conversionOptionsList << std::make_tuple<QString, QString, QString>(
                i18nc("Color space option plus transfer function name", "Keep colorants, encode HLG"),
                i18nc("@tooltip",
                      "The image will be linearized first, and then encoded with a Hybrid Log Gamma curve."
                      " Recommended for images intended for screens which cannot understand PQ"),
                "ApplyHLG");

            conversionOptionsList << std::make_tuple<QString, QString, QString>(
                i18nc("Color space option plus transfer function name", "Keep colorants, encode SMPTE ST 428"),
                i18nc("@tooltip",
                      "The image will be linearized first, and then encoded with SMPTE ST 428."
                      " Krita always opens images like these as linear floating point, this option is there to reverse "
                      "that"),
                "ApplySMPTE428");
        }
    }
    for (int i = 0; i < conversionOptionsList.size(); i++) {
        const auto &option = conversionOptionsList.at(i);
        cmbConversionPolicy->addItem(std::get<0>(option), std::get<2>(option));
        cmbConversionPolicy->setItemData(i, std::get<1>(option), Qt::ToolTipRole);
    }
    const QString optionName = cfg->getString("floatingPointConversionOption", "Rec2100PQ");
    cmbConversionPolicy->setCurrentIndex(cmbConversionPolicy->findData(optionName, Qt::UserRole));
    spnNits->setValue(cfg->getDouble("HLGnominalPeak", 1000.0));
    spnGamma->setValue(cfg->getDouble("HLGgamma", 1.2));
    chkHLGOOTF->setChecked(cfg->getBool("removeHGLOOTF", true));

    resampling->setCurrentIndex(resampling->findData(cfg->getInt("resampling", -1)));
    extraChannelResampling->setCurrentIndex(
        extraChannelResampling->findData(cfg->getInt("extraChannelResampling", -1)));
    photonNoise->setValue(cfg->getInt("photonNoise", 0));
    dots->setCurrentIndex(dots->findData(cfg->getInt("dots", -1)));
    patches->setCurrentIndex(patches->findData(cfg->getInt("patches", -1)));
    epf->setValue(cfg->getInt("epf", -1));
    gaborish->setCurrentIndex(gaborish->findData(cfg->getInt("gaborish", -1)));
    modular->setCurrentIndex(modular->findData(cfg->getInt("modularSetVal", -1)));
    keepInvisible->setCurrentIndex(keepInvisible->findData(cfg->getInt("keepInvisible", -1)));
    groupOrder->setCurrentIndex(groupOrder->findData(cfg->getInt("groupOrder", -1)));
    responsive->setCurrentIndex(responsive->findData(cfg->getInt("responsive", -1)));
    progressiveAC->setCurrentIndex(progressiveAC->findData(cfg->getInt("progressiveAC", -1)));
    qProgressiveAC->setCurrentIndex(qProgressiveAC->findData(cfg->getInt("qProgressiveAC", -1)));
    progressiveDC->setCurrentIndex(progressiveDC->findData(cfg->getInt("progressiveDC", -1)));
    channelColorsGlobalPercent->setValue(cfg->getInt("channelColorsGlobalPercent", -1));
    channelColorsGroupPercent->setValue(cfg->getInt("channelColorsGroupPercent", -1));
    paletteColors->setValue(cfg->getInt("paletteColors", -1));
    lossyPalette->setCurrentIndex(lossyPalette->findData(cfg->getInt("lossyPalette", -1)));
    modularGroupSize->setCurrentIndex(modularGroupSize->findData(cfg->getInt("modularGroupSize", -1)));
    modularPredictor->setCurrentIndex(modularPredictor->findData(cfg->getInt("modularPredictor", -1)));
    modularMATreeLearningPercent->setValue(cfg->getInt("modularMATreeLearningPercent", -1));
    jpegReconCFL->setCurrentIndex(jpegReconCFL->findData(cfg->getInt("jpegReconCFL", -1)));

    chkAuthor->setChecked(cfg->getBool("storeAuthor", false));
    exif->setChecked(cfg->getBool("exif", true));
    xmp->setChecked(cfg->getBool("xmp", true));
    iptc->setChecked(cfg->getBool("iptc", true));
    chkMetadata->setChecked(cfg->getBool("storeMetaData", true));
    m_filterRegistryModel.setEnabledFilters(cfg->getString("filters").split(','));
}

KisPropertiesConfigurationSP KisWdgOptionsJPEGXL::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();

    cfg->setProperty("haveAnimation", haveAnimation->isChecked());
    cfg->setProperty("flattenLayers", flattenLayers->isChecked());
    cfg->setProperty("lossless", !lossyEncoding->isChecked());
    cfg->setProperty("effort", effort->value());
    cfg->setProperty("decodingSpeed", decodingSpeed->value());
    cfg->setProperty("lossyQuality", lossyQuality->value());
    cfg->setProperty("forceModular", forceModular->isChecked());
    cfg->setProperty("modularSetVal", modular->currentData());

    cfg->setProperty("floatingPointConversionOption", cmbConversionPolicy->currentData(Qt::UserRole).toString());
    cfg->setProperty("HLGnominalPeak", spnNits->value());
    cfg->setProperty("HLGgamma", spnGamma->value());
    cfg->setProperty("removeHGLOOTF", chkHLGOOTF->isChecked());

    cfg->setProperty("resampling", resampling->currentData());
    cfg->setProperty("extraChannelResampling", extraChannelResampling->currentData());
    cfg->setProperty("photonNoise", photonNoise->value());
    cfg->setProperty("dots", dots->currentData());
    cfg->setProperty("patches", patches->currentData());
    cfg->setProperty("epf", epf->value());
    cfg->setProperty("gaborish", gaborish->currentData());
    // force modular encoding without overriding value in advanced tab
    if (forceModular->isChecked() && lossyEncoding->isChecked()) {
        cfg->setProperty("modular", 1);
    } else {
        cfg->setProperty("modular", modular->currentData());
    }
    cfg->setProperty("keepInvisible", keepInvisible->currentData());
    cfg->setProperty("groupOrder", groupOrder->currentData());
    cfg->setProperty("responsive", responsive->currentData());
    cfg->setProperty("progressiveAC", progressiveAC->currentData());
    cfg->setProperty("qProgressiveAC", qProgressiveAC->currentData());
    cfg->setProperty("progressiveDC", progressiveDC->currentData());
    cfg->setProperty("channelColorsGlobalPercent", channelColorsGlobalPercent->value());
    cfg->setProperty("channelColorsGroupPercent", channelColorsGroupPercent->value());
    cfg->setProperty("paletteColors", paletteColors->value());
    cfg->setProperty("lossyPalette", lossyPalette->currentData());
    cfg->setProperty("modularGroupSize", modularGroupSize->currentData());
    cfg->setProperty("modularPredictor", modularPredictor->currentData());
    cfg->setProperty("modularMATreeLearningPercent", modularMATreeLearningPercent->value());
    cfg->setProperty("jpegReconCFL", jpegReconCFL->currentData());

    cfg->setProperty("storeAuthor", chkAuthor->isChecked());
    cfg->setProperty("exif", exif->isChecked());
    cfg->setProperty("xmp", xmp->isChecked());
    cfg->setProperty("iptc", iptc->isChecked());
    cfg->setProperty("storeMetaData", chkMetadata->isChecked());

    QString enabledFilters;
    for (const auto *filter: m_filterRegistryModel.enabledFilters()) {
        enabledFilters += filter->id() + ',';
    }
    cfg->setProperty("filters", enabledFilters);

    return cfg;
}
